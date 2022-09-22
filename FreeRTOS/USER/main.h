/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 19:59:36
 * @LastEditTime: 2022-02-21 20:12:40
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USER\main.h
 */
#ifndef __MAIN_H
#define __MAIN_H

#include "FreeRTOS.h"
#include "task.h"

//任务优先级
#define START_TASK_PRIO 31
//任务堆栈大小
#define START_STK_SIZE 150
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LED0_TASK_PRIO 2
//任务堆栈大小
#define LED0_STK_SIZE 80
//任务句柄
TaskHandle_t LED0Task_Handler;
//任务函数
void led0_task(void *pvParameters);

//任务优先级
#define WAIT_STARTCMD_TASK_PRIO 6
//任务堆栈大小
#define WAIT_STARTCMD_STK_SIZE 150
//任务句柄
TaskHandle_t WAIT_STARTCMD_Task_Handler;
//任务函数
void wait_Startcmd_task(void *pvParameters);

//任务优先级
#define GET_SENSOR_DATA_TASK_PRIO 5
//任务堆栈大小
#define GET_SENSOR_DATA_STK_SIZE 350
//任务句柄
TaskHandle_t GET_SENSOR_DATA_Task_Handler;
//任务函数
void get_sensor_data_task(void *pvParameters);

#endif
