/******************************************************************************
* File Name: main.c
*
* Description:
*   This is the main application file which initializes the BSP and starts
*   the RTOS scheduler. It starts a task that connects to the Wi-Fi Access
*   Point, starts the mDNS (multicast DNS) and then starts the HTTP client
*   in secure mode. All the HTTP security keys are configured in the file
*   secure_http_client.h using openSSL utility. See README.md to understand
*   how the security keys are generated.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2023, Cypress Semiconductor Corporation (an Infineon company) or
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

/* Header file includes */

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "secure_http_client.h"
#include "FreeRTOS.h"
#include <task.h>
#include "display_task.h"
#include "motion_task.h"
#include "capsense_task.h"
#include "emfile_task.h"
/* Include serial flash library and QSPI memory configurations only for the
 * kits that require the Wi-Fi firmware to be loaded in external QSPI NOR flash.
 */
#if defined(CY_ENABLE_XIP_PROGRAM)
#include "cy_serial_flash_qspi.h"
#include "cycfg_qspi_memslot.h"
#endif

/******************************************************************************
* Macros
******************************************************************************/
/* RTOS related macros. */

#define HTTPS_CLIENT_TASK_PRIORITY          	(configMAX_PRIORITIES - 4)
#define DISPLAY_TASK_PRIORITY          			(configMAX_PRIORITIES - 1)
#define TASK_EMFILE_PRIORITY              	    (configMAX_PRIORITIES - 2)
#define TASK_CAPSENSE_PRIORITY 				    (configMAX_PRIORITIES - 3)
#define MOTION_TASK_PRIORITY        			(configMAX_PRIORITIES - 5)

#define HTTPS_CLIENT_TASK_STACK_SIZE        (5 * 1024)
#define DISPLAY_TASK_STACK_SIZE        		(1024 * 10)
#define TASK_EMFILE_STACK_SIZE              (512u)
#define TASK_CAPSENSE_STACK_SIZE 			(256u)
#define MOTION_TASK_STACK_SIZE        		(256)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE (1u)

/*******************************************************************************
* Global Variables
********************************************************************************/
/* HTTPS client task handle. */
TaskHandle_t https_client_task_handle;
TaskHandle_t display_task_handle;
TaskHandle_t motion_task_handle;
TaskHandle_t emfile_task_handle;
TaskHandle_t capsense_task_handle;

/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 * Summary:
 *  Entry function for the application.
 *  This function initializes the BSP, UART port for debugging, and starts the RTOS scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int: Should never return.
 *
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* This enables RTOS aware debugging in OpenOCD */
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ;

    /* Initialize the Board Support Package (BSP) */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    /* Init QSPI and enable XIP to get the Wi-Fi firmware from the QSPI NOR flash */
    #if defined(CY_ENABLE_XIP_PROGRAM)
        const uint32_t bus_frequency = 50000000lu;

        cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0, CYBSP_QSPI_D1,
                                      CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
                                      CYBSP_QSPI_SCK, CYBSP_QSPI_SS, bus_frequency);

        cy_serial_flash_qspi_enable_xip(true);
    #endif

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen */
    APP_INFO(("\x1b[2J\x1b[;H"));
    APP_INFO(("===================================\n"));
    APP_INFO(("HTTPS Client\n"));
    APP_INFO(("===================================\n\n"));

    /* Create the queues. See the respective data-types for details of queue
     * contents
     */
    capsense_command_q = xQueueCreate(SINGLE_ELEMENT_QUEUE,
                                      sizeof(capsense_command_t));
    capsense_touch_q   = xQueueCreate(SINGLE_ELEMENT_QUEUE,
                                      sizeof(capsense_touch_t));
    emfile_command_q = xQueueCreate(SINGLE_ELEMENT_QUEUE,
                                      sizeof(emfile_command_t));

    /* Starts the HTTPS client in secure mode. */
    /* xTaskCreate(https_client_task, "HTTPS Client", HTTPS_CLIENT_TASK_STACK_SIZE, NULL,
                HTTPS_CLIENT_TASK_PRIORITY, &https_client_task_handle);
	*/
    xTaskCreate(display_task, "Display Task", DISPLAY_TASK_STACK_SIZE, NULL,  DISPLAY_TASK_PRIORITY,  &display_task_handle);
    xTaskCreate(motion_task, "Motion Task", MOTION_TASK_STACK_SIZE, NULL,  MOTION_TASK_PRIORITY,  &motion_task_handle);
    xTaskCreate(emfile_task, "emFile Task", TASK_EMFILE_STACK_SIZE,
                NULL, TASK_EMFILE_PRIORITY, &emfile_task_handle);
    xTaskCreate(task_capsense, "CapSense Task", TASK_CAPSENSE_STACK_SIZE,
                NULL, TASK_CAPSENSE_PRIORITY, &capsense_task_handle);
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Should never get here */
    CY_ASSERT(0);
}
/* [] END OF FILE */
