/*
 * emfile_task.c
 *
 *  Created on: Aug 17, 2023
 *      Author: msnidhin
 */

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "FS.h"
#include <string.h>
#include "emfile_task.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include <inttypes.h>

QueueHandle_t emfile_command_q;

extern TaskHandle_t display_task_handle;

char *filenames[NUM_OF_FILES] = {"img0.bmp","img1.bmp","img2.bmp","img3.bmp","img4.bmp"};


/*******************************************************************************
* Function Name: check_error
****************************************************************************//**
* Summary:
*  Prints the message and halts the execution by running an infinite loop, when
*  the error value is negative.
*
* Parameters:
*  message - message to print if error value is negative.
*  error - error value for evaluation.
*
*******************************************************************************/
static void check_error(char *message, int error)
{
    if (error < 0)
    {
        printf("\n================================================================================\n");
        printf("\nFAIL: %s\n", message);
        printf("Error Value: %d\n", error);
        printf("emFile-defined Error Message: %s", FS_ErrorNo2Text(error));
        printf("\n================================================================================\n");

        while(true);
    }
}

/*******************************************************************************
* Function Name: emfile_task
********************************************************************************
* Summary:
*   Formats the storage device, reads the content from a file and prints the
*   content to the UART terminal, writes a message to the same file, and waits
*   for the user button press. When the button is pressed, deletes the file and
*   returns.
*
* Parameters:
*  arg - Unused.
*
*******************************************************************************/
void emfile_task(void* arg)
{
    U32    		volume_size;
    U32    		file_size;
    U32    		num_bytes_to_read;
    int         error;
    FS_FILE    *file_ptr;
    const char *volume_name = "";

    BaseType_t rtos_api_result;

    emfile_command_t emfile_cmd;
    /* Initialize the file system. */
    FS_Init();
    /* Check if low-level format is required. Applicable only for NOR flash. */
    error = FS_FormatLLIfRequired(volume_name);
    check_error("Error in low-level formatting", error);

    /* Check if volume needs to be high-level formatted. */
    error = FS_IsHLFormatted(volume_name);
    check_error("Error in checking if volume is high-level formatted", error);

    /* Return value of 0 indicates that high-level format is required. */
    if (error == 0)
    {
        printf("Perform high-level format\n");
        error = FS_Format(volume_name, NULL);
        check_error("Error in high-level formatting", error);
    }

    volume_size = FS_GetVolumeSizeKB(volume_name);
    printf("Volume size: %"PRIu32" KB\n\n", volume_size);

    if(0U == volume_size)
    {
        printf("Error in checking the volume size\n");
        CY_ASSERT(0U);
    }


    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a command has been received over queue */
        rtos_api_result = xQueueReceive(emfile_command_q, &emfile_cmd,
                            portMAX_DELAY);

        /* Command has been received */
	   if(rtos_api_result == pdTRUE)
	   {
		   switch(emfile_cmd.operation)
		   {
			   case FILE_READ:
			   {
				    printf("Opening the file for reading...\n");

				    if (emfile_cmd.file_index < NUM_OF_FILES)
				    {
						/* Open the file for reading. */
						file_ptr = FS_FOpen(filenames[emfile_cmd.file_index], "r");
						num_bytes_to_read = emfile_cmd.size_bytes;

						if (file_ptr != NULL)
						{
							file_size = FS_GetFileSize(file_ptr);

							if(file_size < num_bytes_to_read)
							{
								num_bytes_to_read = file_size;
								printf("File size is less than %d \r\n",emfile_cmd.size_bytes);
							}

							printf("Reading %"PRIu32" bytes from the file. \r\n", num_bytes_to_read);
							file_size = FS_Read(file_ptr, emfile_cmd.buffer_ptr, num_bytes_to_read);

							if(file_size != num_bytes_to_read)
							{
								error = FS_FError(file_ptr);
								check_error("Error in reading from the file \r\n", error);
							}

							error = FS_FClose(file_ptr);
							check_error("Error in closing the file \r\n", error);

							xTaskNotify(display_task_handle,true,eSetValueWithOverwrite);
						}
						else
						{
							printf("Unable to read. File not found.\n");
							xTaskNotify(display_task_handle,false,eSetValueWithOverwrite);
						}
				    }
				    else
				    {
				    	printf("Invalid File Index.\n");
				    	xTaskNotify(display_task_handle,false,eSetValueWithOverwrite);
				    }
			        break;
			   }
			   case FILE_WRITE:
			   {
				   if (emfile_cmd.file_index < NUM_OF_FILES)
					{
						printf("Opening the file for writing...\n");

					   /* Mode 'w' truncates the file size to zero if the file exists otherwise
						* creates a new file.
						*/
						file_ptr = FS_FOpen(filenames[emfile_cmd.file_index], "w");

						if(file_ptr != NULL)
						{
						   file_size = FS_Write(file_ptr, emfile_cmd.buffer_ptr, emfile_cmd.size_bytes);

						   if(file_size != emfile_cmd.size_bytes)
						   {
							   error = FS_FError(file_ptr);
							   check_error("Error in writing to the file", error);
						   }

						   printf("File has been written \n");
						   error = FS_FClose(file_ptr);
						   check_error("Error in closing the file", error);

						   xTaskNotify(display_task_handle,true,eSetValueWithOverwrite);
						}
						else
						{
							printf("Unable to open the file for writing! Exiting...\n");

							 xTaskNotify(display_task_handle,false,eSetValueWithOverwrite);
						}
					}
				   else
				   {
				    	printf("Invalid File Index.\n");
				    	xTaskNotify(display_task_handle,false,eSetValueWithOverwrite);
				   }

					break;
				}

			   case FORCE_FORMAT:
			   {
				 error = FS_Format(volume_name, NULL);
				 check_error("Error in high-level formatting", error);
				 xTaskNotify(display_task_handle,true,eSetValueWithOverwrite);
				 break;
				}

			   /* Invalid command */
			   default:
			   {
				   break;
			   }
		   }
	   }
    }
}



