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
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "qpcpp.hpp"
#include "accel.hpp"
#include "blink.hpp"
#include "bsp.hpp"
#include "events.hpp"
#include "glow.h"

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

/* Subscriber list storage (only needed for public events) */
static QP::QSubscrList SubscrSto[MAX_PUB_SIG];

/* Event pool(s) - a maximum of 3 is allowed, and they must be created with
 * increasing block size. See: "Practical UML Statecharts in C/C++, Second
 * Edition", by Dr Miro Samek, page 13 notes (7,8) and Chapter 7 (spec. 7.5).
 */

#if 0
/* Small event pool */
static QF_MPOOL_EL(AccelDataEvt) smlPoolSto[2];
#endif


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

using namespace QP;

extern "C"
{

Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, 10000U);

    while (true) {
       BSP::UserLed.set_high();
       HAL_Delay(250);
       BSP::UserLed.set_low();
       HAL_Delay(250);
    }
    NVIC_SystemReset();
}

//............................................................................
void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    QK_ISR_ENTRY();   // inform QK about entering an ISR
    QTimeEvt::TICK_X(0U, nullptr); // process time events for rate 0
    QK_ISR_EXIT();  // inform QK about exiting an ISR
}

} // extern "C"

void QF::onStartup(void)
{
   SysTick_Config(SystemCoreClock / 100U);

   // assign all priority bits for preemption-prio. and none to sub-prio.
   NVIC_SetPriorityGrouping(0U);

   // set priorities of ALL ISRs used in the system, see NOTE1
   //
   // !!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
   // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
   //

   /* Kernel-unaware interrupts */
   NVIC_SetPriority(DMA1_Channel3_IRQn, 0U);

   /* Kernel-aware interrupts below */
   NVIC_SetPriority(EXTI0_IRQn, QF_AWARE_ISR_CMSIS_PRI);
   NVIC_SetPriority(USART1_IRQn, QF_AWARE_ISR_CMSIS_PRI + 1);
   NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI + 2);
}

void QK::onIdle()
{
   QF_INT_DISABLE();

   BSP::UserLed.set_high();

   QF_INT_ENABLE();

   /* Wait for interrupt to wake up the system */
   __WFI();

   BSP::UserLed.set_low();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

   static QEvt const *accelQSto[10]; // Event queue storage for Accel
   static QEvt const *blinkQSto[10]; // Event queue storage for Blinky

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /* Initialise QF framework */
  QP::QF::init();

  QP::QF::psInit(SubscrSto, Q_DIM(SubscrSto));

#if 0
  QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
#endif

  /* Initialise Active Objects here */
  AO_Blink->start(1U, blinkQSto, Q_DIM(blinkQSto), nullptr, 0U);

  AO_Accel->start(2U, accelQSto, Q_DIM(accelQSto), nullptr, 0U);

  /* Start the kernel (this doesn't return) */
  QF::run();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   static QP::QEvt const accelIntEvt = { ACCEL_INT_SIG, 0U, 0U };

   if (GPIO_Pin == INT_KX132_Pin)
   {
      QP::QF::PUBLISH(&accelIntEvt, nullptr);
   }
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
   static QP::QEvt const accelDMARxDoneEvent = { ACCEL_DMA_RX_DONE_SIG, 0U, 0U };

   if (hspi->Instance == SPI1)
   {
      QP::QF::PUBLISH(&accelDMARxDoneEvent, nullptr);
   }
}

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
