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
#include <stdio.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define send 0xc1ae
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t version_receive[26];
uint8_t version_send[4] = {0xAE, 0xC1, 0x0e, 0x00},			resolution_send[] = {0xAE, 0xC1, 0x0C, 0x01, 0x00},
		resolution_receive[10],
		brightness_send[] = {0xAE, 0xC1, 0x10, 0x01, 0x00},
		brightness_receive[10],
		blocks_send[] = {0xAE, 0xC1, 0x20, 0x02, 0x03, 0xFF},
		blocks_receive[40];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);

void getVersion();
void getResolution();
void setCameraBrightness(uint8_t brightness);
void getBlocks(uint8_t sigmap,uint8_t maxBlocks);
uint8_t stop[4] = {0x7A, 0x0A, 50, 0x00}, //array with data for stop
      forward[4] = {0x7A, 0x01, 50, 0x00}, //array with data for forward
      reverse[4] = {0x7A, 0x02, 50, 0x00}, //array with data for reverse
      turn_left[4] = {0x7A, 0x04, 50, 0x00}, //array with data for turn left
      turn_right[4] = {0x7A, 0x03, 50, 0x00}, //array with data for turn right
      top_left[4] = {0x7A, 0x07, 50, 0x00}, //array with data for top left
      top_right[4] = {0x7A, 0x06, 50, 0x00}, //array with data for top right
      bot_left[4] = {0x7A, 0x08, 50, 0x00}, //array with data for bottom left
      bot_right[4] = {0x7A, 0x09, 50, 0x00}, //array with data for bottom right
      clockwise[4] = {0x7A, 0x10, 50, 0x00}, //array with data for clockwise
      count_clockwise[4] = {0x7A, 0x11, 50, 0x00}, //array with data for counter clockwise
      side_left[4] = {0x7A, 0x13, 50, 0x00}, //array with data for side left
      side_right[4] = {0x7A, 0x12, 50, 0x00}; //array with data for side right

uint16_t object_present, x_combined, y_combined, width_combined, height_combined, signature;

int count = 1;
int countb = 1;

int state = 0;
int state2 = 1;

int changespeed = 0;
int changespeed2 = 1;
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */


void stopp(){

}

uint8_t calculate_xor_checksum(uint8_t *data, uint32_t length) //checksum algorithm function using XOR. Takes in the pointer to the first element of the array and size of the array
{
    uint8_t checksum = 0; //initialise the checksum to be zero

    for (uint32_t i = 0; i < length; i++)
    {
        checksum ^= data[i]; //runs through each element and does the XOR operation. Updates the value of checksum accordingly.
    }

    return checksum; //returns the value of the checksum
}
void move(uint8_t* movement, int count) //function that will coordinate the movement. Takes in the specific direction array and the unit counts for the movement.
{

	  movement[3] = calculate_xor_checksum(movement, sizeof(*movement)* 4); //updates the value of the 4th element with the checksum value by calling the checksum function.

  uint8_t flag = 0x00; //declare an uint8_t type variable flag and initialise it to 0x00

  uint8_t ok = 0x01; //declare an uint8_t type variable ok and initialise it to 0x01

  for (int k = 0; k < count;) //loops through till it reaches count as desired by the function call parameter
  {
    HAL_UART_Receive(&huart1,&flag,sizeof(flag),10); //checks the state of the flag

    if(flag == ok) //if flag is equals to ok, signifies that there is no blockage
    {
      for (int i = 0; i <= 3; i++)
      {
        HAL_UART_Transmit(&huart1, &movement[i], 1, HAL_MAX_DELAY); //transmits the movement data one element at a time
      }

      k++; //increase the value of k
      HAL_Delay(10);
    }

    else ////if flag is not equals to ok, signifies that there is a blockage
    {
      for (int i = 0; i <= 3; i++)
      {
        HAL_UART_Transmit(&huart1, &stop[i], 1, HAL_MAX_DELAY); //transmits the stop movement
       // HAL_Delay(10);
      }
    }
  }
}

void getVersion()
{
	  HAL_UART_Transmit(&huart3, version_send, sizeof(version_send), 10);
	  HAL_UART_Receive(&huart3, version_receive, sizeof(version_receive), 10);
}

void getResolution()
{
    for (int i = 0; i < sizeof(resolution_send); i++)
    {
        if (HAL_UART_Transmit(&huart3, &(resolution_send[i]), 1, HAL_MAX_DELAY) != HAL_OK)
        {
            char error_msg[50];
            sprintf(error_msg, "Error transmitting byte %d in getResolution()\n", i);
            HAL_UART_Transmit(&huart2, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
            return;
        }
        HAL_Delay(10);
    }
    for (int k = 0; k < sizeof(resolution_receive); k++)
    {
        if (HAL_UART_Receive(&huart3, &(resolution_receive[k]), 1, 10) != HAL_OK)
        {
            char error_msg[50];
            sprintf(error_msg, "Error receiving byte %d in getResolution()\n", k);
            HAL_UART_Transmit(&huart2, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
            return;
        }
    }
}

void setCameraBrightness(uint8_t brightness)
{
	  if (brightness <= 255 && brightness >= 0)
	  {
		  brightness_send[4] = brightness;
	  }

	  else
	  {
        char error_msg[50];
        sprintf(error_msg, "Brightness value is out of range 0-255 %d in setCameraBrightness()\n", brightness);
        HAL_UART_Transmit(&huart2, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
        return;
	  }

    for (int i = 0; i < sizeof(brightness_send); i++)
    {
        if (HAL_UART_Transmit(&huart3, &(brightness_send[i]), 1, HAL_MAX_DELAY) != HAL_OK)
        {
            char error_msg[50];
            sprintf(error_msg, "Error transmitting byte %d in setCameraBrightness()\n", i);
            HAL_UART_Transmit(&huart2, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
            return;
        }
        HAL_Delay(10);
    }

    char success_msg[50];
    sprintf(success_msg, "Camera brightness set to %d\n", brightness);
    HAL_UART_Transmit(&huart2, (uint8_t*)success_msg, strlen(success_msg), HAL_MAX_DELAY);

    for (int k = 0; k < sizeof(brightness_receive); k++)
    {
        if (HAL_UART_Receive(&huart3, &(brightness_receive[k]), 1, 10) != HAL_OK)
        {
            char error_msg[50];
            sprintf(error_msg, "Error receiving byte %d in setCameraBrightness()\n", k);
            HAL_UART_Transmit(&huart2, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
            return;
        }

    }
}

void getBlocks(uint8_t sigmap,uint8_t maxBlocks)
  {
		HAL_UART_Transmit(&huart3, blocks_send, sizeof(blocks_send), 50);
	    HAL_UART_Receive(&huart3, blocks_receive, sizeof(blocks_receive), 50);

	    object_present = blocks_receive[3];
	    signature = ((uint16_t)blocks_receive[7] << 8) | blocks_receive[6];
		x_combined = ((uint16_t)blocks_receive[9] << 8) | blocks_receive[8];
		y_combined = ((uint16_t)blocks_receive[11] << 8) | blocks_receive[10];
		width_combined = ((uint16_t)blocks_receive[13] << 8) | blocks_receive[12];
		height_combined = ((uint16_t)blocks_receive[15] << 8) | blocks_receive[14];



  }

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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //    move(forward, 50); //calls for move forward
	  //    move(top_right, 50); //calls for move top right
	  //    move(side_right, 50); //calls for move side right
	  //    move(bot_left, 50); //calls for bottom left
	  //  move(reverse, 50); //calls for move reverse
	  //    move(bot_left, 50); //calls for move bottom left
	  //    move(side_left, 50); //calls for move side left
	  //    move(top_left, 50); //calls for move top left
	  //    move(stop, 100); //calls for stop

	  //	  getBlocks(1, 1);
	  //
	  //      char blocks_msg[50];
	  //      sprintf(blocks_msg, "Forward");
	  //      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), HAL_MAX_DELAY);



	  	 getBlocks(1, 1);

		 if (object_present == 0) //no object
		 {
			 while (object_present == 0)
			 {
			      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
			      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
			      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
//			      HAL_Delay(50);
			      getBlocks(1, 1);


			      move(stop, count);
			 }


		 }

	  	 switch (signature)
	  	 {
	  		 case 1:

	  			    // if the LED is on, turn it off

	  			      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	  			      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	  			      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
	  			      HAL_Delay(50);



	  			 if (object_present == 0) //no object
	  			 {
	  				 move(stop, count);

	  			      char blocks_msg[50];
	  			      sprintf(blocks_msg, " Stop ");
	  			      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  			      //getBlocks(1, 1);
	  			 }

	  			 else //there is an object
	  			 {

	  				 //if (width_combined > 120 && width_combined < 70) //out of range
	  				 //{
	  					 if (width_combined > 120) //if the object is too near, move reverse
	  					 {
	  						 if (x_combined < 100) //if the object is center but to the left, slide left
	  						 {
	  							 if (state == 1)
	  							 {
	  								 move(stop, count);
	  								 state = 0;
	  							 }
	  							 if (state2 == 0)
	  							 {
	  								 move(stop, count);
	  								 state2 = 1;
	  							 }

	  							move(bot_left,countb);

	  						      char blocks_msg[50];
	  						      sprintf(blocks_msg, " Bot-Left ");
	  						      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  						      //getBlocks(1, 1);
	  						 }

	  						 else if (x_combined > 200) //if the object is center but to the right, slide right
	  						 {
	  							 if (state == 1)
	  							 {
	  								 state2 = 1;
	  								 move(stop, count);
	  								 state = 0;
	  							 }
	  							 if (state2 == 0)
	  							 {
	  								 move(stop, count);
	  								 state2 = 1;
	  							 }

	  							 move(bot_right, countb);

	  						      char blocks_msg[50];
	  						      sprintf(blocks_msg, " Bot-Right ");
	  						      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  						      //getBlocks(1, 1);
	  						 }

	  						 else if (x_combined > 100 && x_combined < 200)
	  						 {

	  							 state = 1;
	  							 state2 = 0;

	  							 if (width_combined < 140)
	  							 {
	  								 reverse[2] = 40;
	  								 move(reverse, count);
	  							 }
	  							 else if (width_combined >= 140 )
	  							 {
	  								 reverse[2] = 80;
	  								 move(reverse, count);
	  							 }

	  							  char blocks_msg[50];
	  							  sprintf(blocks_msg, " Reverse ");
	  							  HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  							  //getBlocks(1, 1);
	  						 }


	  					 } // end of if for too near

	  					 else if (width_combined < 70) //if the object is too far, move forward
	  					 {

	  						 if (x_combined < 80) //if the object is center but to the left, slide left
	  						 {
	  							 if (state == 1)
	  							 {
	  								 move(stop, count);
	  								 state = 0;
	  							 }
	  							 if (state2 == 0)
	  							 {
	  								 move(stop, count);
	  								 state2 = 1;
	  							 }

	  							 move(top_left,countb);

	  						      char blocks_msg[50];
	  						      sprintf(blocks_msg, " Top-Left ");
	  						      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  						      //getBlocks(1, 1);
	  						 }

	  						 else if (x_combined > 220) //if the object is center but to the right, slide right
	  						 {
	  							 if (state == 1)
	  							 {
	  								 move(stop, count);
	  								 state = 0;
	  							 }
	  							 if (state2 == 0)
	  							 {
	  								 move(stop, count);
	  								 state2 = 1;
	  							 }

	  							 move(top_right, countb);

	  						      char blocks_msg[50];
	  						      sprintf(blocks_msg, " Top-Right ");
	  						      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  						      //getBlocks(1, 1);
	  						 }

	  						 else
	  						 {
	  							 state = 1;
	  							 state2 = 0;

	  							 if (width_combined > 50)
	  							 {
	  								 forward[2] = 40;
	  								 move(forward, count);
	  							 }
	  							 else if (width_combined <= 50 )
	  							 {
	  								 forward[2] = 80;
	  								 move(forward, count);
	  							 }

	  						      char blocks_msg[50];
	  						      sprintf(blocks_msg, " Forward ");
	  						      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  						      //getBlocks(1, 1);
	  						 }
	  					 } //end of if for too far

	  				 //} //end of if for out of range

	  				 else if (width_combined <= 120 && width_combined >= 70) //within desired range
	  				 {

	  					 if (x_combined >=80 && x_combined <= 220) //if the object is in the center, stop
	  					 {
	  						 move(stop,count);

	  					      char blocks_msg[50];
	  					      sprintf(blocks_msg, " Stop ");
	  					      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  					      //getBlocks(1, 1);
	  					 }

	  					 else if (x_combined < 80) //if the object is center but to the left, slide left
	  					 {
	  						state = 1;

	  						 if (x_combined > 70)
	  						 {
	  							 side_left[2] = 40;
	  							 move(side_left, count);
	  						 }
	  						 else if (x_combined <= 70 )
	  						 {
	  							 side_left[2] = 80;
	  							 move(side_left, count);
	  						 }

	  					      char blocks_msg[50];
	  					      sprintf(blocks_msg, " Slide-Left ");
	  					      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  					      //getBlocks(1, 1);
	  					 }

	  					 else if (x_combined > 220) //if the object is center but to the right, slide right
	  					 {
	  						 state = 1;


	  						 if (x_combined < 240)
	  						 {
	  							 side_right[2] = 40;
	  							 move(side_right, count);
	  						 }
	  						 else if (x_combined >= 240 )
	  						 {
	  							 side_right[2] = 80;
	  							 move(side_right, count);
	  						 }

	  					      char blocks_msg[50];
	  					      sprintf(blocks_msg, " Slide-Right ");
	  					      HAL_UART_Transmit(&huart2, (uint8_t*)blocks_msg, strlen(blocks_msg), 10);

	  					      //getBlocks(1, 1);
	  					 }
	  				 } //end of if for desired range


	  			 }

	  			 break;


	  		 case 2:
	  			 //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
	  			     //HAL_Delay(1000);
	  			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	  			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	  			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	  			HAL_Delay(50);

	  			move(stop, count);




	  		break;

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
