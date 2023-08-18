/*
 * display_task.c
 *
 *  Created on: Aug 17, 2023
 *      Author: chana
 */

#include "GUI.h"
#include "mtb_st7789v.h"
#include "cyhal.h"
#include "cybsp.h"
#include "display_task.h"
#include "FreeRTOS.h"
//#include "vw.h"
//#include "campeon.h"
//#include "mountain.h"
#include "cat.h"
#include "emfile_task.h"
#include "limits.h"
#include "capsense_task.h"
static void __update_picture(GUI_BITMAP * pBM);

extern GUI_CONST_STORAGE GUI_BITMAP bmpingpong;
extern GUI_CONST_STORAGE GUI_BITMAP bmvw_golf;
extern GUI_CONST_STORAGE GUI_BITMAP bmcat;
extern unsigned char acInfineonBlackLogo[32400UL + 1];

extern QueueHandle_t emfile_command_q;
extern QueueHandle_t capsense_touch_q;

/* Display Pinout Configuration */
const mtb_st7789v_pins_t tft_pins =
{
    .db08 = CYBSP_A8,
    .db09 = CYBSP_A9,
    .db10 = CYBSP_A10,
    .db11 = CYBSP_A11,
    .db12 = CYBSP_A12,
    .db13 = CYBSP_A13,
    .db14 = CYBSP_A14,
    .db15 = CYBSP_A15,
    .nrd  = CYBSP_D10,
    .nwr  = CYBSP_D11,
    .dc   = CYBSP_D12,
    .rst  = CYBSP_D13
};

char filebuff[FILE_SIZE_BYTES];

void display_task(void *arg)
{
	(void)arg;
	cy_rslt_t result;
	uint8_t index = 0;
	char txbuffer[20];

	/* Initialize the display controller */
	result = mtb_st7789v_init8(&tft_pins);
	CY_ASSERT(result == CY_RSLT_SUCCESS);

	GUI_Init();
	GUI_SetFont(&GUI_Font8x16);
	GUI_SetBkColor(GUI_BLACK);
	GUI_Clear();

	GUI_SetPenSize(1);
	GUI_SetColor(GUI_RED);
	GUI_GotoXY(10,10);
	GUI_DispString("Test");
	uint32_t notifiedValue;

	emfile_command_t test_cmd = {FILE_WRITE, 2u, cat, FILE_SIZE_BYTES};
	//xQueueSendToBack(emfile_command_q, &test_cmd, portMAX_DELAY);

	//xTaskNotifyWait(0x00,ULONG_MAX,&notifiedValue,portMAX_DELAY);

	//emfile_command_t test_cmd = {FILE_WRITE, 3u, mountain, FILE_SIZE_BYTES};
	//xQueueSendToBack(emfile_command_q, &test_cmd, portMAX_DELAY);

	//xTaskNotifyWait(0x00,ULONG_MAX,&notifiedValue,portMAX_DELAY);
	while(1)
	{
		emfile_command_t test_cmd = {FILE_READ, index, filebuff, FILE_SIZE_BYTES};

			//emfile_command_t test_cmd = {FORCE_FORMAT, 0u, NULL, 0u};

		xQueueSendToBack(emfile_command_q, &test_cmd, portMAX_DELAY);

		xTaskNotifyWait(0x00,ULONG_MAX,&notifiedValue,portMAX_DELAY);

		if(notifiedValue)
		{
			printf("Operation succeeded \r\n");
			GUI_Clear();
			GUI_BMP_Draw(filebuff, 0,0);
			sprintf(txbuffer, "index %d", index);
			GUI_DispString(txbuffer);

			capsense_touch_t touchinfo;

			BaseType_t rtos_api_result;

			rtos_api_result = xQueueReceive(capsense_touch_q, &touchinfo,10000u);

			/* Command has been received from capsense_cmd */
			if(rtos_api_result == pdTRUE)
			{
				if(touchinfo.sldr_right)
				{
					index++;
				}
				else if(touchinfo.sldr_left)
				{
					if(index == 0)
					{
						index = 4;
					}
					else
					{
						index--;
					}
				}
			}
			else
			{
				index++;
			}

		}
		else
		{
			index++;
			printf("Operation failed \r\n");

		}

		if (index>=NUM_OF_FILES)
		{
			index = 0u;
		}





		//__update_picture(&bmcat);
		//__update_picture(&bmvw_golf);





/*
		GUI_SetColor(GUI_BLUE);
		shift -= 4;
		GUI_DrawLine(80 + shift, 10, 240 + shift, 90);
		GUI_DrawLine(80, 90 + shift, 240, 10 + shift);
		GUI_SetColor(GUI_RED);
		shift += 5;
		GUI_DrawLine(80 + shift, 10, 240 + shift, 90);
		GUI_DrawLine(80, 90 + shift, 240, 10 + shift);
*/
	}
}
static void __update_picture(GUI_BITMAP * pBM)
{
	GUI_Clear();
	GUI_DrawBitmap(pBM, 0,0);
	vTaskDelay(pdMS_TO_TICKS(5000));
}


