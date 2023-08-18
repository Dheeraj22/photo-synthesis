/*
 * motion_task.c
 *
 *  Created on: Aug 18, 2023
 *      Author: chana
 */

#include "FreeRTOS.h"
#include "motion_task.h"
#include "cyhal.h"
#include <task.h>
#include "cybsp.h"
#include "mtb_st7789v.h"

extern TaskHandle_t display_task_handle;
extern TaskHandle_t motion_task_handle;

static void motion_sensor_handler(void *handler_arg, cyhal_gpio_event_t event);

static uint8_t sleep_countdown = DISPLAY_ON_DURATION_SEC;

#define ST7789V_CMD_DISPOFF   (0x28) /* Display Off */
#define ST7789V_CMD_DISPON    (0x29) /* Display On */
#define ST7789V_CMD_WRDISBV   (0x51) /* Write Display Brightness */
#define ST7789V_CMD_WRCTRLD   (0x53) /* Write CTRL Display */
#define ST7789V_CMD_RDCTRLD   (0x54) /* Read CTRL Value Display */

void motion_task(void *arg)
{
    cy_rslt_t result;
    cyhal_gpio_callback_data_t gpio_callback_data;

	/* init GPIO to read PIR status */
    result = cyhal_gpio_init(CYBSP_A1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE , 0u);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    gpio_callback_data.callback = motion_sensor_handler;
    cyhal_gpio_register_callback(CYBSP_A1, &gpio_callback_data);
    cyhal_gpio_enable_event(CYBSP_A1, CYHAL_GPIO_IRQ_RISE,
                                 GPIO_INTERRUPT_PRIORITY, true);

    /* Initialize the User LED */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG,
                     CYBSP_LED_STATE_OFF);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

	while(1)
	{
		//mtb_st7789v_write_command(ST7789V_CMD_RDCTRLD);
		//printf("sleep_countdown: %d\r\n DIS: 0x%x\r\n", sleep_countdown, mtb_st7789v_read_data());
		if(sleep_countdown == 0)
		{
			cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
			vTaskSuspend(display_task_handle);
			GUI_Clear();
			vTaskSuspend(motion_task_handle);

		}
		else
		{
			sleep_countdown --;
			cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}

static void motion_sensor_handler(void *handler_arg, cyhal_gpio_event_t event)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	sleep_countdown = DISPLAY_ON_DURATION_SEC;
	xTaskResumeFromISR(display_task_handle);
	xTaskResumeFromISR(motion_task_handle);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


