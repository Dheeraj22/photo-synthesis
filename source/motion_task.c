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
extern TaskHandle_t display_task_handle;

void motion_task(void *arg)
{
    cy_rslt_t result;
    uint8_t motion_detected = 0;
	/* init GPIO to read PIR status */
    result = cyhal_gpio_init(CYBSP_A1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE , 0u);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    /* Initialize the User LED */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG,
                     CYBSP_LED_STATE_OFF);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
	while(1)
	{
		motion_detected = cyhal_gpio_read(CYBSP_A1);
		if(motion_detected)
		{
			cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
		}
		else
		{
			cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
		}
		//vTaskDelay(pdMS_TO_TICKS(100));

	}

}


