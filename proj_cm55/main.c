/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the KIT PSE84 AI: OV7675 and BGT60TR13C data streaming over Wifi
*             Application for ModusToolbox.
*
* Related Document: See README.md
*
* Created on: 2025-12-01
* Company: Rutronik Elektronische Bauelemente GmbH
* Address: Industriestraße 2, 75228 Ispringen, Germany
* Author: RJ030
*
*******************************************************************************
* Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cybsp.h"
#include "cy_pdl.h"
#include "mtb_dvp_camera_ov7675.h"

#include "retarget_io_init.h"

#include <stdlib.h>
#include <string.h>

#include "driver/radar.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tcp_server/secure_tcp_server.h"

#include "ring_buffer.h"

uint8_t* image_buffer_0;
uint8_t* image_buffer_1;

uint16_t* radar_data;
uint16_t radar_num_samples;

cy_stc_scb_i2c_context_t i2c_master_context;
bool frame_ready = false;
bool active_frame = false;

#define COM_OVERHEAD	8

#define DATA_COLLECTION_TASK_STACK_SIZE          (configMINIMAL_STACK_SIZE * 8)
#define DATA_COLLECTION_TASK_PRIORITY            (configMAX_PRIORITIES - 1)

#define TCP_SECURE_SERVER_TASK_STACK_SIZE  (1024U * 5U)
#define TCP_SECURE_SERVER_TASK_PRIORITY    (1U)


SemaphoreHandle_t radar_data_mutex;

TaskHandle_t network_task_handler;

#define RING_BUFFER_SIZE 1000000

/**
 * @brief This task is used to collect the data from the radar and from the OV7675
 * It pushes the data into a ring-buffer, read by the socket server
 */
static void data_collection_task(void * arg)
{
    CY_UNUSED_PARAMETER(arg);

    uint8_t header_socket[6];
	uint16_t radar_frame_counter = 0;

	printf("data_collection_task started.\r\n");

	// Initialize memory
	if (ring_buffer_init(RING_BUFFER_SIZE) != 0)
	{
		printf("Cannot init ring buffer...\r\n");
		vTaskSuspend(NULL);
	}

	// Wait until network task is ready
	vTaskDelay(6000);

	// Initialize the radar sensor
	if (radar_init() != 0)
	{
		printf("Cannot init radar...\r\n");
		vTaskSuspend(NULL);
	}

    for (;;)
	{
    	// Something from the OV7675ß
		if (frame_ready)
		{
			frame_ready = false;

			uint8_t* copy_buffer = image_buffer_0;
			if (active_frame != 0)
				copy_buffer = image_buffer_1;

			if(xSemaphoreTake(radar_data_mutex, 0) == pdPASS)
			{
				Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, 1);
				// Enough size?
				int size_with_header = 4 + (OV7675_MEMORY_BUFFER_SIZE);
				if ((ring_buffer_get_free_memory() / 2) > size_with_header)
				{
					header_socket[0] = 0x55;
					header_socket[1] = 0x55;
					header_socket[2] = 0x2; // type camera
					header_socket[3] = 0x33; // CRC (todo)
					ring_buffer_enqueue(header_socket, 4);
					ring_buffer_enqueue(copy_buffer, OV7675_MEMORY_BUFFER_SIZE);
				}
				Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, 0);
				xSemaphoreGive(radar_data_mutex);
			}
			xTaskNotifyGive(network_task_handler);
		}

		// Something from the radar?
		if (radar_is_data_available())
		{
			if (radar_read_data(radar_data, radar_num_samples) != 0)
			{
				printf("Error reading radar data\r\n");
			}
			else
			{
				if(xSemaphoreTake(radar_data_mutex, 10) == pdPASS)
				{
					Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, 1);
					// Enough size?
					int size_with_header = 4 + 2 + (radar_num_samples * 2);
					if (ring_buffer_get_free_memory() > size_with_header)
					{
						header_socket[0] = 0x55;
						header_socket[1] = 0x55;
						header_socket[2] = 0x1; // type radar
						header_socket[3] = 0x33; // CRC (todo)
						*(uint16_t*)(&header_socket[4]) = radar_frame_counter;
						ring_buffer_enqueue(header_socket, 6);
						ring_buffer_enqueue((uint8_t*)radar_data, radar_num_samples * 2);
					}
					xSemaphoreGive(radar_data_mutex);
					Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, 0);
				}
				xTaskNotifyGive(network_task_handler);
				radar_frame_counter++;
			}
		}

		vTaskDelay(20);
	}
}

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board initialization failed. Stop program execution. */
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

	init_retarget_io();

    /* Enable global interrupts */
    __enable_irq();

	printf("PSOC EDGE OV7675 v1.0 - WIFI Streaming\r\n");

    /* Enable I2C Controller */
    result = Cy_SCB_I2C_Init(CYBSP_I2C_CAM_CONTROLLER_HW, &CYBSP_I2C_CAM_CONTROLLER_config,
                             &i2c_master_context);
    if (CY_SCB_I2C_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_I2C_Enable(CYBSP_I2C_CAM_CONTROLLER_HW);

    image_buffer_0 = malloc(OV7675_MEMORY_BUFFER_SIZE);
	if (image_buffer_0 == NULL)
	{
		printf("Cannot allocate 0 \r\n");
		return 0;
	}

	memset(image_buffer_0, 0, OV7675_MEMORY_BUFFER_SIZE);

	image_buffer_1 = malloc(OV7675_MEMORY_BUFFER_SIZE);
	if (image_buffer_1 == NULL)
	{
		printf("Cannot allocate 1 \r\n");
		return 0;
	}

	radar_num_samples = radar_get_num_samples_per_frame();
	radar_data = malloc(radar_num_samples * sizeof(uint16_t));
	if (radar_data == NULL)
	{
		printf("Cannot allocate radar_data \r\n");
		return 0;
	}

	// Create mutex and queue for radar
	radar_data_mutex = xSemaphoreCreateMutex();

	tcp_secure_server_set_mutex(radar_data_mutex);

    /* Initialize the camera DVP OV7675 */
    result = mtb_dvp_cam_ov7675_init(image_buffer_0, image_buffer_1, &i2c_master_context,
                                     &frame_ready, &active_frame);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    // Create the FreeRTOS Task used to collect the data
    result = xTaskCreate(data_collection_task, "data_collection",
    		DATA_COLLECTION_TASK_STACK_SIZE, NULL,
			DATA_COLLECTION_TASK_PRIORITY, NULL);

    if( pdPASS != result )
    {
    	printf("Cannot create data collection task\r\n");
    	return 0;
    }

    // Create the TCP server task
	result = xTaskCreate(tcp_secure_server_task, "Network Task",
			TCP_SECURE_SERVER_TASK_STACK_SIZE, NULL,
			TCP_SECURE_SERVER_TASK_PRIORITY, &network_task_handler);

	if( pdPASS != result )
	{
		printf("Cannot create server task\r\n");
		return 0;
	}

	// Start the RTOS Scheduler
	vTaskStartScheduler();

    printf("Something went wrong\r\n");
    return -1;
}
