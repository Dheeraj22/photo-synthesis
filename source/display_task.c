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

static void __update_picture(GUI_BITMAP * pBM);

extern GUI_CONST_STORAGE GUI_BITMAP bmpingpong;
extern GUI_CONST_STORAGE GUI_BITMAP bmvw_golf;
extern GUI_CONST_STORAGE GUI_BITMAP bmcat;
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

void display_task(void *arg)
{
	(void)arg;
	cy_rslt_t result;

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
	uint8_t shift = 0;

	while(1)
	{

		__update_picture(&bmcat);
		__update_picture(&bmvw_golf);
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


