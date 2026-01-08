/*******************************************************************************
* File Name: network_credentials.h
*
* Description: This file is the public interface for Wi-Fi credentials and
* TCP server certificate.
*
* Related Document: See README.md
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

/*******************************************************************************
* Include guard
*******************************************************************************/
#ifndef NETWORK_CREDENTIALS_H_
#define NETWORK_CREDENTIALS_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "cy_wcm.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define INITIALISER_IPV4_ADDRESS(addr_var, addr_val) addr_var = \
                                                     { CY_WCM_IP_VER_V4, \
                                                     { .v4 = (uint32_t) \
                                                     (addr_val) } }

#define MAKE_IPV4_ADDRESS(a, b, c, d)        ((((uint32_t) d) << 24U) | \
                                             (((uint32_t) c) << 16U) | \
                                             (((uint32_t) b) << 8U) | \
                                             ((uint32_t) a))

/* To use the Wi-Fi device in AP interface mode, set this macro as '1' */
#define USE_AP_INTERFACE                     (1U)

#if(USE_AP_INTERFACE)
    #define WIFI_INTERFACE_TYPE              CY_WCM_INTERFACE_TYPE_AP

    /* SoftAP Credentials: Modify SOFTAP_SSID and SOFTAP_PASSWORD as required */
    #define SOFTAP_SSID                      "MY_SOFT_AP"
    #define SOFTAP_PASSWORD                  "psoc1234"

    /* Security type of the SoftAP. See 'cy_wcm_security_t' structure
     * in "cy_wcm.h" for more details.
     */
    #define SOFTAP_SECURITY_TYPE             CY_WCM_SECURITY_WPA2_AES_PSK

    #define SOFTAP_IP_ADDRESS_COUNT          (2U)
    #define SOFTAP_IP_ADDRESS                MAKE_IPV4_ADDRESS(192, 168, 10, 1)
    #define SOFTAP_NETMASK                   MAKE_IPV4_ADDRESS(255, 255, 255, 0)
    #define SOFTAP_GATEWAY                   MAKE_IPV4_ADDRESS(192, 168, 10, 1)
    #define SOFTAP_RADIO_CHANNEL             (1U)
#else
    #define WIFI_INTERFACE_TYPE              CY_WCM_INTERFACE_TYPE_STA

    /* Wi-Fi Credentials: Modify WIFI_SSID, WIFI_PASSWORD, and
     * WIFI_SECURITY_TYPE to match your Wi-Fi network credentials.
     * Note: Maximum length of the Wi-Fi SSID and password is set to
     * CY_WCM_MAX_SSID_LEN and CY_WCM_MAX_PASSPHRASE_LEN as defined in cy_wcm.h
     * file.
     */
    #define WIFI_SSID                        "MY_WIFI_SSID"
    #define WIFI_PASSWORD                    "MY_WIFI_PASSWORD"

    /* Security type of the Wi-Fi access point. See 'cy_wcm_security_t'
     * structure in "cy_wcm.h" for more details.
     */
    #define WIFI_SECURITY_TYPE               CY_WCM_SECURITY_WPA2_AES_PSK

    /* Maximum number of connection retries to a Wi-Fi network. */
    #define MAX_WIFI_CONN_RETRIES            (10U)

    /* Wi-Fi re-connection time interval in milliseconds */
    #define WIFI_CONN_RETRY_INTERVAL_MSEC    (1000U)
#endif

/* TCP server certificate. Copy from the TCP server certificate
 * generated by OpenSSL (See Readme.md on how to generate a SSL certificate).
 */
#define keySERVER_CERTIFICATE_PEM \
"-----BEGIN CERTIFICATE-----\n"\
"MIIBujCCAV8CFEKiUA1YW//+eMaxl/XPV7Q0fUVgMAoGCCqGSM49BAMCMF8xCzAJ\n"\
"BgNVBAYTAklOMQswCQYDVQQIDAJLQTERMA8GA1UEBwwIQkFOR0xPUkUxETAPBgNV\n"\
"BAoMCElORklORU9OMQwwCgYDVQQLDANJQ1cxDzANBgNVBAMMBnNlcnZlcjAeFw0y\n"\
"NDEwMDgxMTEwMDlaFw0yNzA3MDUxMTEwMDlaMF8xCzAJBgNVBAYTAklOMQswCQYD\n"\
"VQQIDAJLQTERMA8GA1UEBwwIQkFOR0xPUkUxETAPBgNVBAoMCElORklORU9OMQww\n"\
"CgYDVQQLDANJQ1cxDzANBgNVBAMMBnNlcnZlcjBZMBMGByqGSM49AgEGCCqGSM49\n"\
"AwEHA0IABByz2TOLQq+M1RaBf1LkDwrtEvKxP4QT47PIf/ICnWdjmFIlswLCInxt\n"\
"ezAOwQPq141Aj3AFYEFNQCqu3zdsF3IwCgYIKoZIzj0EAwIDSQAwRgIhAOz0F0kG\n"\
"HNWNAKN6HHNmPMNJSeuQ4XC4XrCagWTDbA9WAiEAoMWiDuRpejrak0qy6SJ2G1Zf\n"\
"vGu8l1XNeW4vVlWwMZY=\n"\
"-----END CERTIFICATE-----\n"

/* Private key of the TCP Server. Copy from the TCP server key 
 * generated by OpenSSL (See Readme.md on how to create a private key).
 */
#define keySERVER_PRIVATE_KEY_PEM \
"-----BEGIN EC PRIVATE KEY-----\n"\
"MHcCAQEEIHzfC2IFu+MdNNq3NRhocaSBsEYZdBIu1b8J28U7LJ47oAoGCCqGSM49\n"\
"AwEHoUQDQgAEHLPZM4tCr4zVFoF/UuQPCu0S8rE/hBPjs8h/8gKdZ2OYUiWzAsIi\n"\
"fG17MA7BA+rXjUCPcAVgQU1AKq7fN2wXcg==\n"\
"-----END EC PRIVATE KEY-----\n"

/* TCP client certificate. In this example this is the RootCA
 * certificate so as to verify the TCP client's identity. */
#define keyCLIENT_ROOTCA_PEM \
"-----BEGIN CERTIFICATE-----\n"\
"MIICEzCCAbmgAwIBAgIUDF40ZIoUHB8KhkyycXOKhxL3oiwwCgYIKoZIzj0EAwIw\n"\
"XzELMAkGA1UEBhMCSU4xCzAJBgNVBAgMAktBMREwDwYDVQQHDAhCQU5HTE9SRTER\n"\
"MA8GA1UECgwISU5GSU5FT04xDDAKBgNVBAsMA0lDVzEPMA0GA1UEAwwGc2VydmVy\n"\
"MB4XDTI0MTAwODExMDkxMFoXDTI0MTEwNzExMDkxMFowXzELMAkGA1UEBhMCSU4x\n"\
"CzAJBgNVBAgMAktBMREwDwYDVQQHDAhCQU5HTE9SRTERMA8GA1UECgwISU5GSU5F\n"\
"T04xDDAKBgNVBAsMA0lDVzEPMA0GA1UEAwwGc2VydmVyMFkwEwYHKoZIzj0CAQYI\n"\
"KoZIzj0DAQcDQgAEsTjPdfqdeWq9Tumz1aqj7X+sdMHPGzrvQ1ZM76ffxzoFhCig\n"\
"cfl4QkhzVhOgh1xxpme2wLlnBgm7oNz6cw4VUaNTMFEwHQYDVR0OBBYEFFj0oApL\n"\
"Cn4KOrQGis5e7GmfTSU/MB8GA1UdIwQYMBaAFFj0oApLCn4KOrQGis5e7GmfTSU/\n"\
"MA8GA1UdEwEB/wQFMAMBAf8wCgYIKoZIzj0EAwIDSAAwRQIgV6BCXDfoyBR3BkTQ\n"\
"Nppjw1yb4jhApzn5EGDUa/4BVR8CIQCxAbKYT9avlTIDoixG8utrhHufHXXQGFFe\n"\
"mnu4rA5vYA==\n"\
"-----END CERTIFICATE-----\n"

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* NETWORK_CREDENTIALS_H_ */
