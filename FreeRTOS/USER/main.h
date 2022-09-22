/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 19:59:36
 * @LastEditTime: 2022-02-21 20:12:40
 * @LastEditors: TOTHTOT
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USER\main.h
 */
#ifndef __MAIN_H
#define __MAIN_H

#include "FreeRTOS.h"
#include "task.h"

//�������ȼ�
#define START_TASK_PRIO 31
//�����ջ��С
#define START_STK_SIZE 150
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define LED0_TASK_PRIO 2
//�����ջ��С
#define LED0_STK_SIZE 80
//������
TaskHandle_t LED0Task_Handler;
//������
void led0_task(void *pvParameters);

//�������ȼ�
#define WAIT_STARTCMD_TASK_PRIO 6
//�����ջ��С
#define WAIT_STARTCMD_STK_SIZE 150
//������
TaskHandle_t WAIT_STARTCMD_Task_Handler;
//������
void wait_Startcmd_task(void *pvParameters);

//�������ȼ�
#define GET_SENSOR_DATA_TASK_PRIO 5
//�����ջ��С
#define GET_SENSOR_DATA_STK_SIZE 350
//������
TaskHandle_t GET_SENSOR_DATA_Task_Handler;
//������
void get_sensor_data_task(void *pvParameters);

#endif
