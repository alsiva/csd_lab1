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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <string.h>
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
const int numCombinaions = 3;
const uint32_t second = 1000;
uint16_t blink_mode = 0; // Номер комбинации
typedef struct {
    int length;
    int currentPhase;
    uint32_t* lamp; //Последовательность ламп в комбинации
    uint32_t* defaultTime; //Дефолтная продолжительность ламп
    uint32_t* leftTime; //Оставшаяся продолжительность ламп
} Combination;

typedef struct {
	uint8_t length;
	char* values;
	char* symbol;
} Buffer;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

Buffer BufferConstructor(int length) {
	Buffer buffer;
	buffer.length = length;
	buffer.values = (char*)malloc(length * sizeof(char));
	buffer.symbol = buffer.values;

	return buffer;
}


Combination CombinationConstructor(int length, uint32_t* lamp, uint32_t* defaultTime) {
    Combination comb;
    comb.length = length;
    comb.currentPhase = 0;
    comb.lamp = (uint32_t*)malloc(length * sizeof(uint32_t));
    comb.defaultTime = (uint32_t*)malloc(length * sizeof(uint32_t));
    comb.leftTime = (uint32_t*)malloc(length * sizeof(uint32_t));

    // Copy values to allocated memory
    for (int i = 0; i < length; i++) {
        comb.lamp[i] = lamp[i];
        comb.defaultTime[i] = defaultTime[i];
        comb.leftTime[i] = defaultTime[i];
    }

    return comb;
}

Combination* CombListGenerator(int numCombinations) {
    Combination* combinationList = (Combination*)malloc(numCombinations * sizeof(Combination));

    // Combination 0
    uint32_t lamps0[] = {GREEN_LIGHT, YELLOW_LIGHT, RED_LIGHT};
    uint32_t defaultTime0[] = {2*second, 2*second, 2*second};
    combinationList[0] = CombinationConstructor(3, lamps0, defaultTime0);

    // Combination 1
    uint32_t lamps1[] = {RED_LIGHT, YELLOW_LIGHT};
    uint32_t defaultTime1[] = {1*second, 1*second};
    combinationList[1] = CombinationConstructor(2, lamps1, defaultTime1);

    // Combination 2
    uint32_t lamps2[] = {GREEN_LIGHT, RED_LIGHT};
    uint32_t defaultTime2[] = {1*second, 1*second};
    combinationList[2] = CombinationConstructor(2, lamps2, defaultTime2);

    // Add more combinations as needed

    return combinationList;
}

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


//Выполняет фазу. Если пользователь нажимает кнопку -- меняет комбинацию
uint16_t completePhase(Combination* comb, uint16_t blink_mode) {
	uint16_t result_blink_mode = blink_mode;


	int currentPhase = comb->currentPhase;
	turnLightOn(comb->lamp[currentPhase]);
	startTime = HAL_GetTick();
	uint32_t duration;
	int pressedButton = 0;

	while((duration = HAL_GetTick() - startTime) < comb->leftTime[currentPhase]) {
		if (HAL_GPIO_ReadPin(GPIOC, BUTTON) == 0) {
			comb->leftTime[currentPhase] -= duration;
			result_blink_mode++; // Если кнопка нажата то меняем комбинацию
			wait(second * 0.5);
			pressedButton = 1;
			break;
		}
	}


	if (pressedButton == 0) {
		comb->leftTime[currentPhase] = comb->defaultTime[currentPhase];
		comb->currentPhase += 1;
		if (comb->currentPhase == comb->length) {
			comb->currentPhase = 0;
		}
	}

	turnLightOff(comb->lamp[currentPhase]);
	return result_blink_mode;
}

void proceedCommand(char* command) {

	char* mes;
	if (strtok(command, " ") == "new") {
		char* light = strtok(NULL, " ");
		if (2 <= sizeof(light) && sizeof(light) >= 8) {
			mes = "Количество ламп в комбинации должно быть от 2 до 8\n";
			HAL_UART_Transmit(&huart6, (uint8_t *) mes, sizeof(mes), 3000);
			return;
		}

		uint32_t lamp[sizeof(light)];

		int i = 0;
		while (light[i] != NULL) {
			if (light[i] == 'g') {
				lamp[i] = GREEN_LIGHT;
			} else if (light[i] == 'y') {
				lamp[i] = YELLOW_LIGHT;
			} else if (light[i] == 'r') {
				lamp[i] = RED_LIGHT;
			} else if (light[i] == 'n') {
				lamp[i] = 0;
			} else {
				mes = "Доступны только следующие символы g, r, y, n \n";
				HAL_UART_Transmit(&huart6, (uint8_t *) mes, sizeof(mes), 3000);
				return;
			}
			i++;
		}

		mes = 'Введите пожалуйста продолжительности для комбинаций';
		HAL_UART_Transmit(&huart6, (uint8_t *) mes, sizeof(mes), 3000);

		char* timeForComb;
		uint32_t defaultTime[sizeof(lamp)]; //Дефолтная продолжительность ламп
		for (int i = 0; i < sizeof(defaultTime); i++) {
			sprintf(mes, "Продолжительность для комбинации №", intValue);
			HAL_UART_Transmit(&huart6, (uint8_t *) mes, sizeof(mes), 3000);
			if (HAL_UART_Receive(&huart6, (uint8_t *) timeForComb, 1, 10) == HAL_OK) {
				int number = atoi(timeForComb);
				if (number == 0) {
					mes = "Вы должны ввести число больше 0";
					HAL_UART_Transmit(&huart6, (uint8_t *) mes, sizeof(mes), 3000);
					return;
				}


			} else {
				return;
			}
		}
	}

	return;
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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  //Combination* combList -- список комбинаций
  //Combination* combList = CombListGenerator(3);
  //uint16_t* start_blink_mode -- текущая комбинация

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  Buffer input_buffer = BufferConstructor(10);
  char overFlowMessage[] = "\nВы видите данное сообщение если произошло переполнение буфера\n";


  while (1) {
	  if (HAL_UART_Receive(&huart6, (uint8_t *) input_buffer.symbol, 1, 10) == HAL_OK) {
		  //Дальнейший код нужно будет каким-то образом переписать
		  HAL_UART_Transmit(&huart6, (uint8_t *) input_buffer.symbol, 1, 100);
		  char letter = *input_buffer.symbol;

		  if (*input_buffer.symbol == '\r') {
			  char command[input_buffer.symbol - input_buffer.values];
			  int i = 0;

			  while (1) {
				  char command_sym = input_buffer.values[i];
				  if (command_sym == '\r') {
					  command[i] = '\0';
					  break;
				  }
				  command[i] = command_sym;
				  i++;
			  }

			  proceedCommand(command);
		  }

		  input_buffer.symbol++;
		  if (input_buffer.symbol >= input_buffer.values + input_buffer.length) {
			  HAL_UART_Transmit(&huart6, (uint8_t *) overFlowMessage, sizeof(overFlowMessage), 1000);
			  input_buffer.symbol = input_buffer.values;
		  }


	  }

	  /*
	  if (blink_mode == 3) blink_mode = 0;
	  Combination* comb = &combList[blink_mode];
	  blink_mode = completePhase(comb, blink_mode);
	  */



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
