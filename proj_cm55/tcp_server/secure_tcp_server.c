/*******************************************************************************
* File Name:   secure_tcp_server.c
*
* Description: This file contains declaration of task and functions related to
* TCP server operation.
*
********************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
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
*******************************************************************************/
/* TCP server task header file. */
#include "secure_tcp_server.h"

/* Secure socket header file */
#include "cy_secure_sockets.h"
#include "cy_tls.h"

/* Wi-Fi connection manager header files */
#include "cy_wcm.h"
#include "cy_wcm_error.h"

/* Standard C header file */
#include <string.h>

#include "network_credentials.h"

/* IP address related header files (part of the lwIP TCP/IP stack). */
#include "ip_addr.h"

/* to use the portable formatting macros */
#include <inttypes.h>

#include "retarget_io_init.h"

#include "ring_buffer.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* Length of the LED ON/OFF command issued from the TCP server. */
#define TCP_LED_CMD_LEN                                (1U)

/* LED ON and LED OFF commands. */
#define LED_ON_CMD                                     '1'
#define LED_OFF_CMD                                    '0'

#define RESET_VAL                                      (0U)
#define DEBOUNCE_DELAY                                 (250U)
#define TASKNOTIFYBITS_TO_CLEARONENTRY                 (0U)
#define TASKNOTIFYBITS_TO_CLEARONEXIT                  (0U)
#define NULL_CHARACTER                                 '\0'
#define GPIO_INTERRUPT_PRIORITY                        (7U)
#define APP_SDIO_INTERRUPT_PRIORITY                    (7U)
#define APP_HOST_WAKE_INTERRUPT_PRIORITY               (2U)
#define APP_SDIO_FREQUENCY_HZ                          (25000000U)
#define SDHC_SDIO_64BYTES_BLOCK                        (64U)
#define DEBOUNCE_TIME_MS                               (1U)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
cy_rslt_t create_secure_tcp_server_socket(void);
cy_rslt_t tcp_connection_handler(cy_socket_t socket_handle, void *arg);
cy_rslt_t tcp_receive_msg_handler(cy_socket_t socket_handle, void *arg);
cy_rslt_t tcp_disconnection_handler(cy_socket_t socket_handle, void *arg);
static void user_button_interrupt_handler(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Secure socket variables. */
cy_socket_sockaddr_t tcp_server_addr, peer_addr;
cy_socket_t server_handle, client_handle;
cy_wcm_ip_address_t ip_address;

/* TLS credentials of the TCP server. */
static const char tcp_server_cert[] = keySERVER_CERTIFICATE_PEM;
static const char server_private_key[] = keySERVER_PRIVATE_KEY_PEM;

/* Root CA certificate for TCP client identity verification. */
static const char tcp_client_ca_cert[] = keyCLIENT_ROOTCA_PEM;

/* Variable to store the TLS identity (certificate and private key). */
void *tls_identity;

/* Size of the peer socket address. */
uint32_t peer_addr_len;

/* Flags to tack the LED state and command. */
bool led_state = CYBSP_LED_STATE_OFF;

/* Flag variable to check if TCP client is connected. */
bool client_connected;

/* Interrupt config structure */
cy_stc_sysint_t sysint_cfg =
{
    .intrSrc = CYBSP_USER_BTN_IRQ,
    .intrPriority = GPIO_INTERRUPT_PRIORITY
};

/* Flag to check button pressed event */
bool button_pressed = false;

/* Secure TCP server task handle. */
//extern TaskHandle_t server_task_handle;

static mtb_hal_sdio_t sdio_instance;
static cy_stc_sd_host_context_t sdhc_host_context;
static cy_wcm_config_t wcm_config;
volatile bool button_debouncing = false;
volatile uint32_t button_debounce_timestamp = 0;

#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)

/* SysPm callback parameter structure for SDHC */
static cy_stc_syspm_callback_params_t sdcardDSParams =
{
    .context   = &sdhc_host_context,
    .base      = CYBSP_WIFI_SDIO_HW
};

/* SysPm callback structure for SDHC*/
static cy_stc_syspm_callback_t sdhcDeepSleepCallbackHandler =
{
    .callback           = Cy_SD_Host_DeepSleepCallback,
    .skipMode           = SYSPM_SKIP_MODE,
    .type               = CY_SYSPM_DEEPSLEEP,
    .callbackParams     = &sdcardDSParams,
    .prevItm            = NULL,
    .nextItm            = NULL,
    .order              = SYSPM_CALLBACK_ORDER
};
#endif

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: sdio_interrupt_handler
********************************************************************************
* Summary:
*  Interrupt handler function for SDIO instance.
*******************************************************************************/
static void sdio_interrupt_handler(void)
{
    mtb_hal_sdio_process_interrupt(&sdio_instance);
}

/*******************************************************************************
* Function Name: host_wake_interrupt_handler
********************************************************************************
* Summary:
* Interrupt handler function for Host wake up instance.
*******************************************************************************/
static void host_wake_interrupt_handler(void)
{
    mtb_hal_gpio_process_interrupt(&wcm_config.wifi_host_wake_pin);
}

/*******************************************************************************
* Function Name: app_sdio_init
********************************************************************************
* Summary:
* This function configures and initializes the SDIO instance used in
* communication between the host MCU and the wireless device.
*******************************************************************************/
static void app_sdio_init(void)
{
    cy_rslt_t result;
    mtb_hal_sdio_cfg_t sdio_hal_cfg;

    cy_stc_sysint_t sdio_intr_cfg =
    {
        .intrSrc = CYBSP_WIFI_SDIO_IRQ,
        .intrPriority = APP_SDIO_INTERRUPT_PRIORITY
    };

    cy_stc_sysint_t host_wake_intr_cfg =
    {
        .intrSrc = CYBSP_WIFI_HOST_WAKE_IRQ,
        .intrPriority = APP_HOST_WAKE_INTERRUPT_PRIORITY
    };

    /* Initialize the SDIO interrupt and specify the interrupt handler. */
    cy_en_sysint_status_t sdio_interrupt_init_status = Cy_SysInt_Init
            (&sdio_intr_cfg, sdio_interrupt_handler);

    /* SDIO interrupt initialization failed. Stop program execution. */
    if(CY_SYSINT_SUCCESS != sdio_interrupt_init_status)
    {
        handle_app_error();
    }

    /* Enable NVIC interrupt. */
    NVIC_EnableIRQ(CYBSP_WIFI_SDIO_IRQ);

    /* Setup SDIO using the HAL object and desired configuration */
    result = mtb_hal_sdio_setup(&sdio_instance,
            &CYBSP_WIFI_SDIO_sdio_hal_config, NULL, &sdhc_host_context);

    /* SDIO setup failed. Stop program execution. */
    if(CY_RSLT_SUCCESS != result)
    {
        handle_app_error();
    }

    /* Initialize and Enable SD HOST */
    Cy_SD_Host_Enable(CYBSP_WIFI_SDIO_HW);
    Cy_SD_Host_Init(CYBSP_WIFI_SDIO_HW,
            CYBSP_WIFI_SDIO_sdio_hal_config.host_config, &sdhc_host_context);
    Cy_SD_Host_SetHostBusWidth(CYBSP_WIFI_SDIO_HW, CY_SD_HOST_BUS_WIDTH_4_BIT);

    sdio_hal_cfg.frequencyhal_hz = APP_SDIO_FREQUENCY_HZ;
    sdio_hal_cfg.block_size = SDHC_SDIO_64BYTES_BLOCK;

    /* Configure SDIO */
    mtb_hal_sdio_configure(&sdio_instance, &sdio_hal_cfg);
     
#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)
    /* SDHC SysPm callback registration */
    Cy_SysPm_RegisterCallback(&sdhcDeepSleepCallbackHandler);
#endif /* (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP) */

    /* Setup GPIO using the HAL object for WIFI WL REG ON  */
    mtb_hal_gpio_setup(&wcm_config.wifi_wl_pin, CYBSP_WIFI_WL_REG_ON_PORT_NUM,
            CYBSP_WIFI_WL_REG_ON_PIN);

    /* Setup GPIO using the HAL object for WIFI HOST WAKE PIN  */
    mtb_hal_gpio_setup(&wcm_config.wifi_host_wake_pin,
            CYBSP_WIFI_HOST_WAKE_PORT_NUM, CYBSP_WIFI_HOST_WAKE_PIN);

    /* Initialize the Host wakeup interrupt and specify the interrupt handler.*/
    cy_en_sysint_status_t interrupt_init_status_host_wake =
            Cy_SysInt_Init(&host_wake_intr_cfg, host_wake_interrupt_handler);

    /* Host wake up interrupt initialization failed. Stop program execution. */
    if(CY_SYSINT_SUCCESS != interrupt_init_status_host_wake)
    {
        handle_app_error();
    }

    /* Enable NVIC interrupt. */
    NVIC_EnableIRQ(CYBSP_WIFI_HOST_WAKE_IRQ);
}

#if(USE_AP_INTERFACE)

static void ap_event_cb(cy_wcm_event_t event, cy_wcm_event_data_t *event_data)
{
    printf("######### Received event changed from wcm, event = %d #######\n", event);
    switch(event)
    {
        case CY_WCM_EVENT_DISCONNECTED:
        {
            printf("Network is down! \n");
            break;
        }
        case CY_WCM_EVENT_RECONNECTED:
        {
            printf("Network is up again! \n");
            break;
        }
        case CY_WCM_EVENT_CONNECTING:
        {
            printf("Connecting to AP ... \n");
            break;
        }
        case CY_WCM_EVENT_CONNECTED:
        {
            printf("Connected to AP and network is up !! \n");
            break;
        }
        case CY_WCM_EVENT_CONNECT_FAILED:
        {
            printf("Connection to AP Failed ! \n");
            break;
        }
        case CY_WCM_EVENT_IP_CHANGED:
        {
        	printf("CY_WCM_EVENT_IP_CHANGED ! \n");
            break;
        }
        case CY_WCM_EVENT_STA_JOINED_SOFTAP:
        {
            printf("mac address of the STA which joined = %02X : %02X : %02X : %02X : %02X : %02X \n",
                    event_data->sta_mac[0], event_data->sta_mac[1], event_data->sta_mac[2],
                    event_data->sta_mac[3], event_data->sta_mac[4], event_data->sta_mac[5]);
            // Someone connected
            Cy_GPIO_Write(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN, 0);
            break;
        }
        case CY_WCM_EVENT_STA_LEFT_SOFTAP:
        {
            printf("mac address of the STA which left = %02X : %02X : %02X : %02X : %02X : %02X \n",
                    event_data->sta_mac[0], event_data->sta_mac[1], event_data->sta_mac[2],
                    event_data->sta_mac[3], event_data->sta_mac[4], event_data->sta_mac[5]);
            // Left
            Cy_GPIO_Write(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN, 1);
            break;
        }
        default:
        {
            printf("Invalid event \n");
            break;
        }
    }
}

/*******************************************************************************
* Function Name: softap_start
********************************************************************************
* Summary:
*  This function configures device in AP mode and initializes
*  a SoftAP with the given credentials (SOFTAP_SSID, SOFTAP_PASSWORD and
*  SOFTAP_SECURITY_TYPE).
*
* Parameters:
*  void
*
* Return:
*  cy_rslt_t: Returns CY_RSLT_SUCCESS if the Soft AP is started successfully,
*  a WCM error code otherwise.
*
*******************************************************************************/
static cy_rslt_t softap_start(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize the Wi-Fi device as a Soft AP. */
    cy_wcm_ap_credentials_t softap_credentials = {SOFTAP_SSID, SOFTAP_PASSWORD,
                                                  SOFTAP_SECURITY_TYPE};

    static const cy_wcm_ip_setting_t softap_ip_info = {
        INITIALISER_IPV4_ADDRESS(.ip_address, SOFTAP_IP_ADDRESS),
        INITIALISER_IPV4_ADDRESS(.gateway,    SOFTAP_GATEWAY),
        INITIALISER_IPV4_ADDRESS(.netmask,    SOFTAP_NETMASK)
        };

    cy_wcm_wifi_band_t wifi_band ={CY_WCM_WIFI_BAND_ANY};

    cy_wcm_ap_config_t softap_config = {softap_credentials, wifi_band,
                                        SOFTAP_RADIO_CHANNEL,softap_ip_info,
                                        NULL};

    // No-one connected
    Cy_GPIO_Write(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN, 1);

    /* Start the the Wi-Fi device as a Soft AP. */
    result = cy_wcm_start_ap(&softap_config);
    if(CY_RSLT_SUCCESS == result)
    {
        printf("Wi-Fi Device configured as Soft AP\n");
        printf("Connect TCP client device to the network: SSID: "
                "%s Password:%s\n",SOFTAP_SSID, SOFTAP_PASSWORD);

        result = cy_wcm_register_event_callback(&ap_event_cb);
		if( result != CY_RSLT_SUCCESS )
		{
			printf("\ncy_wcm_register_event_callback failed....! \n");
		}

    #if(USE_IPV6_ADDRESS)
        /* Get the IPv6 address.*/
        result = cy_wcm_get_ipv6_addr(CY_WCM_INTERFACE_TYPE_AP,
                CY_WCM_IPV6_LINK_LOCAL, &ip_address);
        if(CY_RSLT_SUCCESS == result)
        {
            printf("SofAP : IPv6 address (link-local) assigned: %s\n",
                   ip6addr_ntoa((const ip6_addr_t*)&ip_address.ip.v6));
            memcpy(ip_address.ip.v6, tcp_server_addr.ip_address.ip.v6,
                   sizeof(ip_address.ip.v6));
            tcp_server_addr.ip_address.version = CY_SOCKET_IP_VER_V6;
        }
    #else
        printf("SofAP : IPv4 address assigned : %s\n\n",
                ip4addr_ntoa((const ip4_addr_t *)&softap_ip_info.ip_address.
                        ip.v4));

        /* IP address and TCP port number of the TCP server. */
        tcp_server_addr.ip_address.ip.v4 = softap_ip_info.ip_address.ip.v4;
        tcp_server_addr.ip_address.version = CY_SOCKET_IP_VER_V4;
    #endif            
        tcp_server_addr.port = TCP_SERVER_PORT;
    }
    return result;
}
#endif /* USE_AP_INTERFACE */

#if(!USE_AP_INTERFACE)
/*******************************************************************************
* Function Name: connect_to_wifi_ap()
********************************************************************************
* Summary:
*  Connects to Wi-Fi AP using the user-configured credentials, retries up to a
*  configured number of times until the connection succeeds.
* 
* Parameters:
*  void
*
* Return:
*  cy_result result: Result of the operation.
*
*******************************************************************************/
static cy_rslt_t connect_to_wifi_ap(void)
{
    cy_rslt_t result;

    /* Variables used by Wi-Fi connection manager.*/
    cy_wcm_connect_params_t wifi_conn_param;

    /* Set the Wi-Fi SSID, password and security type. */
    memset(&wifi_conn_param, RESET_VAL, sizeof(cy_wcm_connect_params_t));
    memcpy(wifi_conn_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
    memcpy(wifi_conn_param.ap_credentials.password, WIFI_PASSWORD,
            sizeof(WIFI_PASSWORD));
    wifi_conn_param.ap_credentials.security = WIFI_SECURITY_TYPE;

    /* Join the Wi-Fi AP. */
    for(uint32_t conn_retries = RESET_VAL; conn_retries < MAX_WIFI_CONN_RETRIES;
            conn_retries++ )
    {
        result = cy_wcm_connect_ap(&wifi_conn_param, &ip_address);

        if(CY_RSLT_SUCCESS == result)
        {
            printf("Successfully connected to Wi-Fi network '%s'.\n",
                                wifi_conn_param.ap_credentials.SSID);

            /* IP address and TCP port number of the TCP server */
            #if(USE_IPV6_ADDRESS)
                /* Get the IPv6 address.*/
                result = cy_wcm_get_ipv6_addr(CY_WCM_INTERFACE_TYPE_STA,
                    CY_WCM_IPV6_LINK_LOCAL, &ip_address);
                if(CY_RSLT_SUCCESS == result)
                {
                    printf("IPv6 address (link-local) assigned: %s\n",
                            ip6addr_ntoa((const ip6_addr_t*)&ip_address.ip.v6));
                    memcpy(ip_address.ip.v6, tcp_server_addr.ip_address.ip.v6,
                            sizeof(ip_address.ip.v6));
                    tcp_server_addr.ip_address.version = CY_SOCKET_IP_VER_V6;
                }
            #else
                printf("IPv4 address assigned: %s\n", ip4addr_ntoa
                        ((const ip4_addr_t*)&ip_address.ip.v4));
                tcp_server_addr.ip_address.ip.v4 = ip_address.ip.v4;
                tcp_server_addr.ip_address.version = CY_SOCKET_IP_VER_V4;
            #endif /* USE_IPV6_ADDRESS */

            tcp_server_addr.port = TCP_SERVER_PORT;
            return result;
        }
        printf("Connection to Wi-Fi network failed with error code %"PRIu32"."
               "Retrying in %d ms...\n", result, WIFI_CONN_RETRY_INTERVAL_MSEC);

        vTaskDelay(pdMS_TO_TICKS(WIFI_CONN_RETRY_INTERVAL_MSEC));
    }

    /* Stop retrying after maximum retry attempts. */
    printf("Exceeded maximum Wi-Fi connection attempts\n");

    return result;
}
#endif

SemaphoreHandle_t ring_buffer_mutex;

void tcp_secure_server_set_mutex(SemaphoreHandle_t mutex)
{
	ring_buffer_mutex = mutex;
}

/*******************************************************************************
* Function Name: tcp_secure_server_task
********************************************************************************
* Summary:
*  Task used to establish a connection with a remote TCP client to exchange
*  data between the TCP server and TCP client.
*
* Parameters:
*  void *args: Task parameter defined during task creation (unused)
*
* Return:
*  void
*
*******************************************************************************/
void tcp_secure_server_task(void *arg)
{
    cy_rslt_t result;

    /* TCP server certificate length and private key length. */
    const size_t tcp_server_cert_len = strlen( tcp_server_cert );
    const size_t pkey_len = strlen( server_private_key );

    /* Variable to store number of bytes sent over TCP socket. */
    uint32_t bytes_sent = RESET_VAL;

    // Not connected
    client_connected = false;
    //Cy_GPIO_Write(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN, 1);

    /* CYBSP_USER_BTN1 (SW2) and CYBSP_USER_BTN2 (SW4) share the same port in the
    * PSOC™ Edge E84 evaluation kit and hence they share the same NVIC IRQ line.
    * Since both are configured in the BSP via the Device Configurator, the
    * interrupt flags for both the buttons are set right after they get initialized
    * through the call to cybsp_init(). The flags must be cleared before initializing
    * the interrupt, otherwise the interrupt line will be constantly asserted.
    */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);
    #ifdef CYBSP_USER_BTN2_ENABLED
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN2_IRQ);
    #endif

    /* Initialize the interrupt and register interrupt callback */
    cy_en_sysint_status_t btn_interrupt_init_status = Cy_SysInt_Init(&sysint_cfg,
            &user_button_interrupt_handler);

    if(CY_SYSINT_SUCCESS != btn_interrupt_init_status)
    {
        handle_app_error();
    }

    /* Enable the interrupt in the NVIC */
    NVIC_EnableIRQ(sysint_cfg.intrSrc);

    app_sdio_init();
    wcm_config.interface = WIFI_INTERFACE_TYPE;
    wcm_config.wifi_interface_instance = &sdio_instance;

    /* Initialize Wi-Fi connection manager. */
    result = cy_wcm_init(&wcm_config);
    if (CY_RSLT_SUCCESS != result)
    {
        printf("Wi-Fi Connection Manager initialization failed! Error code: "
                "0x%08"PRIx32"\n", (uint32_t)result);
        handle_app_error();
    }

    printf("Wi-Fi Connection Manager initialized.\r\n");

    #if(USE_AP_INTERFACE)
        /* Start the Wi-Fi device as a Soft AP interface. */
        result = softap_start();
        if (CY_RSLT_SUCCESS != result)
        {
            printf("Failed to Start Soft AP! Error code: 0x%08"PRIx32"\n",
                    (uint32_t)result);
            handle_app_error();
        }
    #else
        /* Connect to Wi-Fi AP */
        result = connect_to_wifi_ap();
        if(CY_RSLT_SUCCESS != result )
        {
            printf("\n Failed to connect to Wi-Fi AP! Error code: 0x%08"PRIx32"\n",
                    (uint32_t)result);
            handle_app_error();
        }
    #endif /* USE_AP_INTERFACE */

    /* Initialize secure socket library. */
    result = cy_socket_init();
    if (CY_RSLT_SUCCESS != result)
    {
        printf("Secure Socket initialization failed! Error code: %"PRIu32"\n",
                result);
        handle_app_error();
    }
    printf("Secure Socket initialized\n");

    /* Create TCP server identity using the SSL certificate and private key. */
    result = cy_tls_create_identity(tcp_server_cert, tcp_server_cert_len,
                                    server_private_key, pkey_len, &tls_identity);
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Failed cy_tls_create_identity! Error code: %"PRIu32"\n", result);
        handle_app_error();
    }

    /* Initializes the global trusted RootCA certificate. This examples uses a
     * self signed certificate which implies that the RootCA certificate is
     * same as the TCP client certificate. */
    result = cy_tls_load_global_root_ca_certificates(tcp_client_ca_cert,
             strlen(tcp_client_ca_cert));

    if(CY_RSLT_SUCCESS != result)
    {
        printf("cy_tls_load_global_root_ca_certificates failed! Error code: "
               "%"PRIu32"\n", result);
        handle_app_error();
    }
    else
    {
        printf("Global trusted RootCA certificate loaded\n");
    }

    /* Create secure TCP server socket. */
    result = create_secure_tcp_server_socket();
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Failed to create socket! Error code: %"PRIu32"\n", result);
        handle_app_error();
    }

    /* Start listening on the secure TCP socket. */
    result = cy_socket_listen(server_handle, TCP_SERVER_MAX_PENDING_CONNECTIONS);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_socket_delete(server_handle);
        printf("cy_socket_listen returned error. Error: %"PRIu32"\n", result);
        handle_app_error();
    }
    else
    {
        printf("==========================================================\n");
        printf("Listening for incoming TCP client connection on Port: %d\n",
                tcp_server_addr.port);
    }

    uint8_t tmp_buffer[8192] = {0};

    for(;;)
    {
    	// Wait for something to happen
    	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    	for(;;)
    	{
    		int size_to_send = 0;
			if(xSemaphoreTake(ring_buffer_mutex, portMAX_DELAY) == pdPASS)
			{
				int used_memory = ring_buffer_get_used_memory();
				if (used_memory > 8192) size_to_send = 8192;
				else size_to_send = used_memory;
				if (size_to_send > 0) ring_buffer_dequeue(tmp_buffer, size_to_send);
				xSemaphoreGive(ring_buffer_mutex);
			}

			if (size_to_send == 0) break;

			if (client_connected)
			{
				// Cy_GPIO_Inv(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN);
				result = cy_socket_send(client_handle, tmp_buffer, size_to_send, CY_SOCKET_FLAGS_NONE, &bytes_sent);
				if(CY_RSLT_SUCCESS != result)
				{
					printf("Failed to send command to client. Error: %"PRIu32"\n", result);
					//if(CY_RSLT_MODULE_SECURE_SOCKETS_CLOSED == result)
					{
						client_connected = false;
						/* Disconnect the socket. */
						cy_socket_disconnect(client_handle,RESET_VAL);
						/* Delete the socket. */
						cy_socket_delete(client_handle);
					}
					Cy_GPIO_Write(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, 0);
					break;
				}
				Cy_GPIO_Inv(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN);
			}
			else
			{
				Cy_GPIO_Write(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, 0);
			}
    	}
    }
 }

/*******************************************************************************
* Function Name: create_secure_tcp_server_socket
********************************************************************************
* Summary:
*  Function to create a socket and set the socket options for configuring TLS
*  identity, socket connection handler, message reception handler and
*  socket disconnection handler.
* 
* Parameters:
*  void.
*
* Return:
*  cy_result result: Result of the operation.
*
*******************************************************************************/
cy_rslt_t create_secure_tcp_server_socket(void)
{
    cy_rslt_t result;
    /* TCP socket receive timeout period. */
    uint32_t tcp_recv_timeout = TCP_SERVER_RECV_TIMEOUT_MS;

    /* Variables used to set socket options. */
    cy_socket_opt_callback_t tcp_receive_option;
    cy_socket_opt_callback_t tcp_connection_option;
    cy_socket_opt_callback_t tcp_disconnect_option;

    /* TLS authentication mode.*/
    //cy_socket_tls_auth_mode_t tls_auth_mode = CY_SOCKET_TLS_VERIFY_REQUIRED;
    cy_socket_tls_auth_mode_t tls_auth_mode = CY_SOCKET_TLS_VERIFY_NONE;

    /* Create a Secure TCP socket. */
    #if(USE_IPV6_ADDRESS)
        result = cy_socket_create(CY_SOCKET_DOMAIN_AF_INET6, CY_SOCKET_TYPE_STREAM,
                                  CY_SOCKET_IPPROTO_TLS, &server_handle);
    #else
        result = cy_socket_create(CY_SOCKET_DOMAIN_AF_INET, CY_SOCKET_TYPE_STREAM,
        		CY_SOCKET_IPPROTO_TCP, &server_handle);
    #endif

    if(CY_RSLT_SUCCESS != result)
    {
        printf("Failed to create socket! Error code: %"PRIu32"\n", result);
        return result;
    }

    /* Set the TCP socket receive timeout period. */
    result = cy_socket_setsockopt(server_handle, CY_SOCKET_SOL_SOCKET,
                                 CY_SOCKET_SO_RCVTIMEO, &tcp_recv_timeout,
                                 sizeof(tcp_recv_timeout));
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Set socket option: CY_SOCKET_SO_RCVTIMEO failed! Error code: "
               "%"PRIu32"\n", result);
        return result;
    }

    /* Register the callback function to handle connection request from a
     * TCP client. */
    tcp_connection_option.callback = tcp_connection_handler;
    tcp_connection_option.arg = NULL;

    result = cy_socket_setsockopt(server_handle, CY_SOCKET_SOL_SOCKET,
                                  CY_SOCKET_SO_CONNECT_REQUEST_CALLBACK,
                                  &tcp_connection_option,
                                  sizeof(cy_socket_opt_callback_t));
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Set socket option: CY_SOCKET_SO_CONNECT_REQUEST_CALLBACK "
               "failed! Error code: %"PRIu32"\n", result);
        return result;
    }

    /* Register the callback function to handle messages received from a TCP
     * client. */
    tcp_receive_option.callback = tcp_receive_msg_handler;
    tcp_receive_option.arg = NULL;

    result = cy_socket_setsockopt(server_handle, CY_SOCKET_SOL_SOCKET,
                                  CY_SOCKET_SO_RECEIVE_CALLBACK,
                                  &tcp_receive_option,
                                  sizeof(cy_socket_opt_callback_t));
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Set socket option: CY_SOCKET_SO_RECEIVE_CALLBACK failed! "
               "Error code: %"PRIu32"\n", result);
        return result;
    }

    /* Register the callback function to handle disconnection. */
    tcp_disconnect_option.callback = tcp_disconnection_handler;
    tcp_disconnect_option.arg = NULL;

    result = cy_socket_setsockopt(server_handle, CY_SOCKET_SOL_SOCKET,
                                  CY_SOCKET_SO_DISCONNECT_CALLBACK,
                                  &tcp_disconnect_option,
                                  sizeof(cy_socket_opt_callback_t));
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Set socket option: CY_SOCKET_SO_DISCONNECT_CALLBACK failed! "
               "Error code: %"PRIu32"\n", result);
        return result;
    }

//    /* Set the TCP socket to use the TLS identity. */
//    result = cy_socket_setsockopt(server_handle, CY_SOCKET_SOL_TLS,
//                                  CY_SOCKET_SO_TLS_IDENTITY,
//                                  tls_identity, sizeof((uint32_t)tls_identity));
//    if(CY_RSLT_SUCCESS != result)
//    {
//        printf("Failed cy_socket_setsockopt! Error code: %"PRIu32"\n", result);
//        return result;
//    }
//
    /* Set the TLS authentication mode. */
    cy_socket_setsockopt(server_handle, CY_SOCKET_SOL_TLS,
                         CY_SOCKET_SO_TLS_AUTH_MODE,
                         &tls_auth_mode, sizeof(cy_socket_tls_auth_mode_t));

     /* Bind the TCP socket created to Server IP address and to TCP port. */
    result = cy_socket_bind(server_handle, &tcp_server_addr,
             sizeof(tcp_server_addr));
    if(CY_RSLT_SUCCESS != result)
    {
        printf("Failed to bind to socket! Error code: %"PRIu32"\n", result);
    }

    return result;
}

/*******************************************************************************
* Function Name: tcp_connection_handler
********************************************************************************
* Summary:
*  Callback function to handle incoming secure TCP client connection.
*
* Parameters:
*  cy_socket_t socket_handle: Connection handle for the TCP server socket
*  void *args : Parameter passed on to the function (unused)
*
* Return:
*  cy_result result: Result of the operation
*
*******************************************************************************/
cy_rslt_t tcp_connection_handler(cy_socket_t socket_handle, void *arg)
{
    cy_rslt_t result;

    /* Accept new incoming connection from a TCP client and
     * perform TLS handshake. */
    result = cy_socket_accept(socket_handle, &peer_addr, &peer_addr_len,
                              &client_handle);
    if(CY_RSLT_SUCCESS == result )
    {
        printf("Incoming TCP connection accepted\n");
        printf("TLS Handshake successful and communication secured!\n");
        printf("Press the user button to send LED ON/OFF command to the "
               "TCP client\n");

        /* Set the client connection flag as true. */
        client_connected = true;
//        Cy_GPIO_Write(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN, 0);
    }
    else
    {
        printf("Failed to accept incoming client connection. Error: "
               "%"PRIu32"\n", result);
        printf("==========================================================="
               "====\n");
        printf("Listening for incoming TCP client connection on Port: %d\n",
                tcp_server_addr.port);
    }

    return result;
}

/*******************************************************************************
* Function Name: tcp_receive_msg_handler
********************************************************************************
* Summary:
*  Callback function to handle incoming TCP client messages.
*
* Parameters:
*  cy_socket_t socket_handle: Connection handle for the TCP client socket
*  void *args : Parameter passed on to the function (unused)
*
* Return:
*  cy_result result: Result of the operation
*
*******************************************************************************/
cy_rslt_t tcp_receive_msg_handler(cy_socket_t socket_handle, void *arg)
{
    char message_buffer[MAX_TCP_RECV_BUFFER_SIZE];
    cy_rslt_t result;

    /* Variable to store number of bytes received from TCP client. */
    uint32_t bytes_received = RESET_VAL;
    result = cy_socket_recv(socket_handle, message_buffer,
                            MAX_TCP_RECV_BUFFER_SIZE,
                            CY_SOCKET_FLAGS_NONE, &bytes_received);

    if(CY_RSLT_SUCCESS == result )
    {
        /* Terminate the received string with '\0'. */
        message_buffer[bytes_received] = NULL_CHARACTER;
        printf("\r\nAcknowledgement from TCP Client: %s\n", message_buffer);

        /* Set the LED state based on the acknowledgement received from the
         * TCP client. */
        if(strcmp(message_buffer, "LED ON ACK") == RESET_VAL)
        {
            led_state = CYBSP_LED_STATE_ON;
        }
        else
        {
            led_state = CYBSP_LED_STATE_OFF;
        }
    }
    else
    {
        printf("Failed to receive acknowledgement from the secure TCP client. "
               "Error: %"PRIu32"\n", result);
        if(CY_RSLT_MODULE_SECURE_SOCKETS_CLOSED == result )
        {
            /* Disconnect the socket. */
            cy_socket_disconnect(socket_handle, RESET_VAL);
            /* Delete the socket. */
            cy_socket_delete(socket_handle);
        }
    }

    printf("===============================================================\n");
    printf("Press the user button to send LED ON/OFF command to the TCP "
           "client\n");

    return result;
}

/*******************************************************************************
* Function Name: tcp_disconnection_handler
********************************************************************************
* Summary:
*  Callback function to handle TCP client disconnection event.
*
* Parameters:
*  cy_socket_t socket_handle: Connection handle for the TCP client socket
*  void *args : Parameter passed on to the function (unused)
*
* Return:
*  cy_result result: Result of the operation
*
*******************************************************************************/
cy_rslt_t tcp_disconnection_handler(cy_socket_t socket_handle, void *arg)
{
    cy_rslt_t result;

    /* Disconnect the TCP client. */
    result = cy_socket_disconnect(socket_handle, RESET_VAL);
    /* Delete the socket. */
    cy_socket_delete(socket_handle);

    /* Set the client connection flag as false. */
    client_connected = false;
//    Cy_GPIO_Write(CYBSP_LED_RGB_GREEN_PORT, CYBSP_LED_RGB_GREEN_PIN, 1);
    printf("TCP Client disconnected! Please reconnect the TCP Client\n");
    printf("===============================================================\n");
    printf("Listening for incoming TCP client connection on Port:%d\n",
            tcp_server_addr.port);

    return result;
}

/*******************************************************************************
* Function Name: user_button_interrupt_handler
********************************************************************************
* Summary:
*  GPIO interrupt service routine. This function detects button
*  presses and sets the command to be sent to the secure TCP client.
*
* Parameters:
*  void *callback_arg : pointer to the variable passed to the ISR
*  cyhal_gpio_event_t event : GPIO event type
*
* Return:
*  None
*
*******************************************************************************/
static void user_button_interrupt_handler(void)
{
}

/* [] END OF FILE */
