/**
 ******************************************************************************
 * File Name          : main.c
 * Description        : Main program body
 ******************************************************************************
 ** This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * COPYRIGHT(c) 2018 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "HLK_M35.h"
#include "string.h"
#include "DS3231.h"
#include "SHT21.h"
#include "BH1750FVI.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t RXCh;
uint8_t rxBuffer[50] = { };
uint8_t counter = 0;
uint8_t newCommand = 0;
uint8_t pompNumber = 0;
RTC_TimeTypeDef RXtime;
RTC_DateTypeDef RXdate;
float sht21Humidity, Sht21Temperature, Lux;
uint16_t sensors[8] = { 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* Command
 * all commands need \r\n at  the end
 * 1  HTML request @html
 * 2  Set time @setTime:18, 9, 1,10,22,06 - year ,month ,day ,hour ,Minute ,Second
 * 3  turn on pomp  @pompOn:3  ,3 is number of pomp
 * 4  turn off pomp  @pompOff:3  ,3 is number of pomp
 * 5  turn on lamp @lampOn
 * 6  turn off lamp @lampOff
 * 7  turn on fan @fanOn
 * 8  turn off fan @fanOff
 *
 * 20 transfer data
 * */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (RXCh == '\r') {
	} else if (RXCh != '\n') {
		if (counter == 50)
			counter = 0;

		rxBuffer[counter++] = (char) RXCh;
		rxBuffer[counter] = 0;

	} else {
		if (strncmp((const char*) rxBuffer, (const char*) "@html", 5) == 0) {
			newCommand = 1;
		} else if (strncmp((const char*) rxBuffer, (const char*) "@setTime", 8)
				== 0) {
			newCommand = 2;
			RXdate.Year = atoi((const char*) &rxBuffer[9]);
			RXdate.Month = atoi((const char*) &rxBuffer[12]);
			RXdate.Date = atoi((const char*) &rxBuffer[15]);
			RXtime.Hours = atoi((const char*) &rxBuffer[18]);
			RXtime.Minutes = atoi((const char*) &rxBuffer[21]);
			RXtime.Seconds = atoi((const char*) &rxBuffer[24]);

		} else if (strncmp((const char*) rxBuffer, (const char*) "@pompOn", 7)
				== 0) {
			newCommand = 3;
			pompNumber = atoi((const char*) &rxBuffer[8]);
		} else if (strncmp((const char*) rxBuffer, (const char*) "@pompOff", 8)
				== 0) {
			newCommand = 4;
			pompNumber = atoi((const char*) &rxBuffer[9]);
		} else if (strncmp((const char*) rxBuffer, (const char*) "@lampOn", 7)
				== 0) {
			newCommand = 5;
		} else if (strncmp((const char*) rxBuffer, (const char*) "@lampOff", 8)
				== 0) {
			newCommand = 6;
		} else if (strncmp((const char*) rxBuffer, (const char*) "@fanOn", 6)
				== 0) {
			newCommand = 7;
		} else if (strncmp((const char*) rxBuffer, (const char*) "@fanOff", 7)
				== 0) {
			newCommand = 8;
		} else if (strncmp((const char*) rxBuffer, (const char*) "@ack", 3)
				== 0) {
			newCommand = 9;
		}
		counter = 0;
		rxBuffer[counter] = 0;
	}

	HAL_UART_Receive_IT(&huart3, &RXCh, 1);
}

void checkCommands() {
	RTC_TimeTypeDef RTC_time;
	RTC_DateTypeDef RTC_date;
	char tempCh[160] = { 0 };
	uint8_t AM_PM;

	switch (newCommand) {
	case 1: //html
		HAL_UART_Transmit(&huart3, (uint8_t*) "@ok:@html\r\n", 11, 100);
		break;
	case 2: //set date and time
		DS3231_setTime(&RXtime, 0, _24_hour_format);
		DS3231_setDate(&RXdate);
		DS3231_getDate(&RTC_date);
		DS3231_getTime(&RTC_time, &AM_PM, _24_hour_format);

		sprintf(tempCh, "@ok:@settime:%2d-%2d-%2d-%2d-%2d-%2d\r\n",
				RTC_date.Year, RTC_date.Month, RTC_date.Date, RTC_time.Hours,
				RTC_time.Minutes, RTC_time.Seconds);
		HAL_UART_Transmit(&huart3, (uint8_t*) tempCh, 32, 100);
		break;
	case 3: // pomp on
		switch (pompNumber) {
		case 1:
			HAL_GPIO_WritePin(pomp1_GPIO_Port, pomp1_Pin, 1);
			break;
		case 2:
			HAL_GPIO_WritePin(pomp2_GPIO_Port, pomp2_Pin, 1);
			break;
		case 3:
			HAL_GPIO_WritePin(pomp3_GPIO_Port, pomp3_Pin, 1);
			break;
		case 4:
			HAL_GPIO_WritePin(pomp4_GPIO_Port, pomp4_Pin, 1);
			break;
		case 5:
			HAL_GPIO_WritePin(pomp5_GPIO_Port, pomp5_Pin, 1);
			break;
		}

		break;
	case 4:  //pomp off
		switch (pompNumber) {
		case 1:
			HAL_GPIO_WritePin(pomp1_GPIO_Port, pomp1_Pin, 0);
			break;
		case 2:
			HAL_GPIO_WritePin(pomp2_GPIO_Port, pomp2_Pin, 0);
			break;
		case 3:
			HAL_GPIO_WritePin(pomp3_GPIO_Port, pomp3_Pin, 0);
			break;
		case 4:
			HAL_GPIO_WritePin(pomp4_GPIO_Port, pomp4_Pin, 0);
			break;
		case 5:
			HAL_GPIO_WritePin(pomp5_GPIO_Port, pomp5_Pin, 0);
			break;
		}
		break;
	case 5: // lamp on
		HAL_GPIO_WritePin(lamp_GPIO_Port, lamp_Pin, 1);
		break;
	case 6: // lamp off
		HAL_GPIO_WritePin(lamp_GPIO_Port, lamp_Pin, 0);
		break;
	case 7: // fan on
		HAL_GPIO_WritePin(fan_GPIO_Port, fan_Pin, 1);
		break;
	case 8: // fan off
		HAL_GPIO_WritePin(fan_GPIO_Port, fan_Pin, 0);
		break;
	case 9: // transfer data
		HAL_GPIO_WritePin(fan_GPIO_Port, fan_Pin, 0);
		sprintf(tempCh,
				"I:101, L:%d, H1:%d, H2:%d, H3:%d, H4:%d, H5:%d, H:%d, T:%d, V:%d\n",
				(int)Lux,(int)sensors[0],(int)sensors[1],(int)sensors[2]
			    ,(int)sensors[3],(int)sensors[4],
				(int)sht21Humidity,(int)Sht21Temperature,(int)sensors[5] );
		HAL_UART_Transmit(&huart3, (uint8_t*) tempCh, strlen(tempCh), 200);
		break;
	}
	newCommand = 0;
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void) {

	/* USER CODE BEGIN 1 */
	HLK_t hlk;
	BH1750FVI_t BH1750FVI;
	uint32_t transferCounter = 0;

	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_I2C1_Init();
	MX_I2C2_Init();
	MX_USART1_UART_Init();
	MX_RTC_Init();
	MX_USART3_UART_Init();

	/* USER CODE BEGIN 2 */
//
	hlk_init(&hlk, hlkDefault_GPIO_Port, hlkDefault_Pin, hlkRst_GPIO_Port,
	hlkRst_Pin, &huart1); //Initialize HLK pins and UART

	//hlk_setAtCommandMode(&hlk);

	HAL_UART_Receive_IT(&huart3, &RXCh, 1);
	BH1750FVI_Init(&BH1750FVI, BH1750FVI_Addr_LOW, &hi2c2,
	BH1750FVI_Continuous_H);

	HAL_ADC_Start_DMA(&hadc1, sensors, 8);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		transferCounter++;
		if (newCommand != 0)
			checkCommands();
		sht21Humidity = SHT21_GetHumidity(&hi2c1, HOLD_MODE);
		Sht21Temperature = SHT21_GetTemperature(&hi2c1, HOLD_MODE);
		Lux = GetLux(&BH1750FVI);

		HAL_Delay(10);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */

}

/** System Clock Configuration
 */
void SystemClock_Config(void) {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time
	 */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	/**Configure the Systick
	 */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(char * file, int line) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */

}

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
