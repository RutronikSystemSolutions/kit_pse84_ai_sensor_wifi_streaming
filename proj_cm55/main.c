#include "cybsp.h"
#include "cy_pdl.h"
#include "mtb_dvp_camera_ov7675.h"

#include "retarget_io_init.h"

#include <stdlib.h>
#include <string.h>

#include "usbd.h"

#include "driver/radar.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tcp_server/secure_tcp_server.h"

#include "ring_buffer.h"

#undef USE_USB

uint8_t* image_buffer_0;
uint8_t* image_buffer_1;
uint8_t* copy_buffer;
uint16_t* radar_data;
uint16_t radar_num_samples;

cy_stc_scb_i2c_context_t i2c_master_context;
bool frame_ready = false;
bool active_frame = false;

#define COM_OVERHEAD	8

#define BLINKY_LED_TASK_STACK_SIZE          (configMINIMAL_STACK_SIZE * 8)
#define BLINKY_LED_TASK_PRIORITY            (configMAX_PRIORITIES - 1)

#define TCP_SECURE_SERVER_TASK_STACK_SIZE  (1024U * 5U)
#define TCP_SECURE_SERVER_TASK_PRIORITY    (1U)


SemaphoreHandle_t radar_data_mutex;

TaskHandle_t network_task_handler;


static void my_task(void * arg)
{
    CY_UNUSED_PARAMETER(arg);

#ifdef USE_USB
    uint8_t cmd = 0;
	int send_data = 0;
	uint8_t counterint = 0;
#endif

	uint16_t radar_frame_counter = 0;

	printf("MY TASK STARTED \r\n");

	if (ring_buffer_init(1000000) != 0)
	{
		printf("Cannot init ring buffer...\r\n");
		vTaskSuspend(NULL);
	}

#ifdef USE_USB
	usbd_t* usb_handle = usbd_create();
#endif

	// Wait until network task is ready
	vTaskDelay(6000);

	printf("MY TASK STARTED after usb\r\n");

//	uint16_t radar_block_index = 0;

	uint8_t header_socket[4];

	if (radar_init() != 0)
	{
		printf("Cannot init radar...\r\n");
		vTaskSuspend(NULL);
	}

	printf("Init done \r\n");

    for (;;)
	{
		// Something in USB read buffer?
#ifdef USE_USB
		if ( usbd_read(usb_handle, &cmd, 1) == 1)
		{
			printf("Received command: %d \r\n", cmd);
			if (cmd == 49)
			{
				send_data = 1;
				counterint = 0;
			}
			else send_data = 0;
		}
#endif

		if ( (frame_ready) )
		{
			frame_ready = false;

			// if (active_frame == 0)
			{
				if (active_frame == 0) memcpy(&copy_buffer[COM_OVERHEAD], image_buffer_0, OV7675_MEMORY_BUFFER_SIZE);
				else memcpy(&copy_buffer[COM_OVERHEAD], image_buffer_1, OV7675_MEMORY_BUFFER_SIZE);

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
						// xTaskNotifyGive(network_task_handler);
					}
//					else
//					{
//						printf("ring buffer camera\r\n");
//					}
					Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, 0);
					xSemaphoreGive(radar_data_mutex);
				}
//				else
//				{
//					printf("oops camera sem\r\n");
//				}

				xTaskNotifyGive(network_task_handler);
#ifdef USE_USB
				// Send once per USB
				if (send_data == 1)
				{
					// Add overhead
					//copy_buffer[0] = counterint;
					// counterint++;
					copy_buffer[0] = 0x55;
					copy_buffer[1] = 0x55;
					copy_buffer[2] = counterint;
					counterint++;
					*((uint32_t*)&copy_buffer[3]) = OV7675_MEMORY_BUFFER_SIZE;
					copy_buffer[7] = 0x33; // CRC to be computed of what is sent after

					if (usbd_write(usb_handle, copy_buffer, OV7675_MEMORY_BUFFER_SIZE + COM_OVERHEAD) != 0)
					{
						printf("Failed to write\r\n");
						send_data = 0;
					}
				}

#endif
			}
		}

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
//					else
//					{
//						printf("ring buffer radar\r\n");
//					}
					xSemaphoreGive(radar_data_mutex);
					Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, 0);
				}
//				else
//				{
//					printf("oops radar sem\r\n");
//				}

				xTaskNotifyGive(network_task_handler);

				radar_frame_counter++;

#ifdef USE_USB
				// Send once per USB
				if (send_data == 1)
				{
					copy_buffer[0] = 0x55;
					copy_buffer[1] = 0x55;
					copy_buffer[2] = counterint;
					counterint++;
					*((uint32_t*)&copy_buffer[3]) = 4096;
					copy_buffer[7] = 0x33; // CRC to be computed of what is sent after

					memcpy(&copy_buffer[COM_OVERHEAD], (uint8_t*)radar_data, radar_num_samples * sizeof(uint16_t));

					if (usbd_write(usb_handle, copy_buffer, 4096 + COM_OVERHEAD) != 0)
					{
						printf("Failed to write\r\n");
						send_data = 0;
					}
				}
#endif
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

	copy_buffer = malloc(OV7675_MEMORY_BUFFER_SIZE + COM_OVERHEAD);
	if (copy_buffer == NULL)
	{
		printf("Cannot allocate copy_buffer \r\n");
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

    /* Create the FreeRTOS Task */
    result = xTaskCreate(my_task, "usb task",
                        BLINKY_LED_TASK_STACK_SIZE, NULL,
                        BLINKY_LED_TASK_PRIORITY, NULL);

    if( pdPASS != result )
    {
    	printf("Cannot create usb task\r\n");
    	return 0;
    }

    /* Create the tasks */
	result = xTaskCreate(tcp_secure_server_task, "Network Task",
			TCP_SECURE_SERVER_TASK_STACK_SIZE, NULL,
			TCP_SECURE_SERVER_TASK_PRIORITY, &network_task_handler);

	if( pdPASS != result )
	{
		printf("Cannot create servder task\r\n");
		return 0;
	}

	printf("Start scheduler\r\n");

	/* Start the RTOS Scheduler */
	vTaskStartScheduler();

    printf("Something wrong\r\n");
}
