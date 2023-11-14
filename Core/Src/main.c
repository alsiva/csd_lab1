/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t GREEN_LIGHT = GPIO_PIN_13;
uint16_t YELLOW_LIGHT = GPIO_PIN_14;
uint16_t RED_LIGHT = GPIO_PIN_15;
uint16_t BUTTON = GPIO_PIN_15;

uint32_t startTime = 0;
const uint32_t second = 1000;

const uint32_t greenLightDuration = 2 * second;
const uint32_t yellowLightDuration = 2 * second;
const uint32_t redLightDuration = 2 * second;

uint16_t blink_mode = 0; // Номер комбинации
uint32_t lamp[] = {0, 0, 0, 0}; // Элемент массива - фаза комбинации
uint32_t leftTime[] = {greenLightDuration, redLightDuration, yellowLightDuration, greenLightDuration}; // Элемент массива - оставшееся время фазы

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void wait(uint32_t duration) {
	uint32_t begin = HAL_GetTick();
	while ((HAL_GetTick() - begin) < duration) {

	}
}

void turnLightOff(uint16_t light_type) {
	HAL_GPIO_WritePin(GPIOD, light_type, GPIO_PIN_RESET);
}

void turnLightOn(uint16_t light_type) {
	HAL_GPIO_WritePin(GPIOD, light_type, GPIO_PIN_SET);
}

// Нужно передавать текущую фазу
//start_blink_mode -- фаза
void completePhase(uint16_t light, uint16_t* start_blink_mode, uint32_t nextLightDuration) {
	uint16_t blink_mode = *start_blink_mode;
	turnLightOn(light);
	startTime = HAL_GetTick();
	uint32_t duration;
	while((duration = HAL_GetTick() - startTime) < leftTime[blink_mode]) {
		if (HAL_GPIO_ReadPin(GPIOC, BUTTON) == 0) {
			leftTime[blink_mode] -= duration;
		  	blink_mode++; // Если кнопка нажата то меняем комбинацию
		  	wait(second * 0.5);
		  	break;
		}
	}

	turnLightOff(light);
	if (blink_mode != *start_blink_mode) {
		if (blink_mode == 1) {
			*start_blink_mode = 1;
		} else if (blink_mode == 2) {
			*start_blink_mode = 2;
		} else if (blink_mode == 3) {
			*start_blink_mode = 3;
		} else if (blink_mode == 4) {
			*start_blink_mode = 0;
		}
		return;
	}

	leftTime[blink_mode] = nextLightDuration;
	lamp[blink_mode]++; // переключаемся на следущую лампу в фазе
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
	  if (blink_mode == 0) {
		  if ((lamp[0] == 0 || lamp[0] > 2) && blink_mode == 0) {
			  lamp[0] = 0;
			  completePhase(GREEN_LIGHT, &blink_mode, yellowLightDuration);
		  }
		  if (lamp[0] == 1 && blink_mode == 0) {
			  completePhase(YELLOW_LIGHT, &blink_mode, redLightDuration);
		  }
		  if (lamp[0] == 2 && blink_mode == 0) {
			  completePhase(RED_LIGHT, &blink_mode, greenLightDuration);
		  }
	  }

	  if (blink_mode == 1) {
		  if ((lamp[1] == 0 || lamp[1] > 1) && blink_mode == 1) {
			  lamp[1] = 0;
			  completePhase(RED_LIGHT, &blink_mode, yellowLightDuration);
		  }
		  if (lamp[1] == 1 && blink_mode == 1) {
			  completePhase(YELLOW_LIGHT, &blink_mode, redLightDuration);
		  }
	  }

	  if (blink_mode == 2) {
		  if ((lamp[2] == 0 || lamp[2] > 1) && blink_mode == 2) {
			  lamp[2] = 0;
			  completePhase(YELLOW_LIGHT, &blink_mode, redLightDuration);
		  }
		  if (lamp[2] == 1 && blink_mode == 2) {
			  completePhase(RED_LIGHT, &blink_mode, yellowLightDuration);
		  }
	  }

	  if (blink_mode == 3) {
		  if ((lamp[3] == 0 || lamp[3] > 1) && blink_mode == 3) {
			  lamp[3] = 0;
			  completePhase(GREEN_LIGHT, &blink_mode, redLightDuration);
		  		  }
		  if (lamp[3] == 1 && blink_mode == 3) {
			  completePhase(RED_LIGHT, &blink_mode, greenLightDuration);
		  }
	  }

    /* USER CODE END WHILE */



    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
