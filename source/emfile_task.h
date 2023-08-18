/*
 * emfile_task.h
 *
 *  Created on: Aug 17, 2023
 *      Author: msnidhin
 */

#ifndef EMFILE_TASK_H_
#define EMFILE_TASK_H_

/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define FILE_SIZE_BYTES ((uint32_t)307254)
#define NUM_OF_FILES (5u)

typedef enum
{
    FILE_READ,
    FILE_WRITE,
	FORCE_FORMAT
} emfile_operation_t;

typedef struct
{
	emfile_operation_t operation;
	uint8_t file_index;
	char * buffer_ptr;
	uint32_t size_bytes;
} emfile_command_t;

extern QueueHandle_t emfile_command_q;


/*******************************************************************************
 * Function prototype
 ******************************************************************************/
void emfile_task(void* param);




#endif /* EMFILE_TASK_H_ */
