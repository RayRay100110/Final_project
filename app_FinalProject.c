/*
 * app_FinalProject.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rsm0195
 */

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stm32l4xx_hal.h"
#include "app.h"

/* Private define ------------------------------------------------------------*/
#define PWM_DUTYCYCLE_MAX 100
#define PWM_DUTYCYCLE_MIN 0
#define PWM_DUTYCYCLE_STEP 5

int dispTemp=0;
int dispDuty=0;
uint32_t adcResult;
char buff[10];
float mv;
char strBuffer[10];

/* Private function prototypes -----------------------------------------------*/
void ShowCommands(void);
void UART_TransmitString(UART_HandleTypeDef *p_huart, char a_string[], int newline);
void PWM_SetDutyCycle(float dutyCycle);

/* Extern global variables
---------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

volatile char rxData;

volatile float pwmDutyCycle = 50.0;

void App_Init(void) {

	UART_TransmitString(&huart2, "-----------------", 1);
	UART_TransmitString(&huart2, "~ Nucleo-L476RG ~", 1);
	UART_TransmitString(&huart2, "-----------------", 1);

	ShowCommands();

	HAL_UART_Receive_IT(&huart2, (uint8_t*) &rxData, 1); //Start the Rx interrupt.

	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	HAL_ADC_Start_IT(&hadc1);
	PWM_SetDutyCycle(pwmDutyCycle);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *p_hadc1){
	//UART_TransmitString(&huart2, "Triggered!", 1);

			adcResult=HAL_ADC_GetValue(&hadc1);
			mv=((float)adcResult*3300.0f)/4096.0f;
			sprintf(buff, "%7.2f", mv);
			//UART_TransmitString(&huart2, "Temperature is (mv): ", 0);
			//UART_TransmitString(&huart2, buff, 1);
}


void App_MainLoop(void) {

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim3){
	//temp range 1410 ~ 1490
	//minimum duty cycle 30

	if (mv>1470){
		pwmDutyCycle=30;
	}

	if (mv<1470 && mv>1450){
		pwmDutyCycle=47.5;
	}

	if (mv<1450 && mv>1430){
		pwmDutyCycle=65;
	}

	if (mv<1430 && mv>1410){
		pwmDutyCycle=82.5;
	}

	if (mv<1410){
		pwmDutyCycle=100;
	}
	 PWM_SetDutyCycle(pwmDutyCycle);
}


void PWM_SetDutyCycle(float dutyCycle) {
	uint16_t periodValue, compareValue;

	periodValue = __HAL_TIM_GET_AUTORELOAD(&htim2);
	compareValue = periodValue * dutyCycle /100.0;
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compareValue);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *p_huart) {
//Process the data received from UART.
	switch (rxData) {
	case 'T':
	case 't':
		//Display Temperature
		UART_TransmitString(&huart2, "Measured voltage is (mv): ", 0);
		UART_TransmitString(&huart2, buff, 1);
		break;
	case 'D':
	case 'd':
		//Display Duty cycle
		sprintf(strBuffer, "%7.2f", pwmDutyCycle);
		UART_TransmitString(&huart2, "Duty cycle is: ", 0);
		UART_TransmitString(&huart2, strBuffer, 1);
		break;
	case 'H':
	case 'h':
		ShowCommands();
		break;

	}
	HAL_UART_Receive_IT(p_huart, (uint8_t*) &rxData, 1); //Restart the Rx interrupt.
}

void ShowCommands(void) {
	UART_TransmitString(&huart2, "Type on keyboard to send command from PC to MCU:", 1);
	UART_TransmitString(&huart2, "> T: Display measure voltage, D: Display duty cycle, H: Show commands", 1);
	//UART_TransmitString(&huart2, "> P: increase PWM duty cycle, M: decrease PWM duty cycle", 1);
}

void UART_TransmitString(UART_HandleTypeDef *p_huart, char a_string[], int newline) {
	HAL_UART_Transmit(p_huart, (uint8_t*) a_string, strlen(a_string), HAL_MAX_DELAY);
	if (newline != 0) {
		HAL_UART_Transmit(p_huart, (uint8_t*) "\n\r", 2, HAL_MAX_DELAY);
	}
}



