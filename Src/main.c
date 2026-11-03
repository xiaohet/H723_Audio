/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stdio.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
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

I2C_HandleTypeDef hi2c1;

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
DMA_HandleTypeDef hdma_sai1_a;
DMA_HandleTypeDef hdma_sai1_b;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
#define AUDIO_WORDS 512U
#define AUDIO_HALF_WORDS 256U
static uint32_t adc_buffer[AUDIO_WORDS] __attribute__((aligned(32)));
static uint32_t dac_buffer[AUDIO_WORDS] __attribute__((aligned(32)));
static volatile uint32_t audio_rx_ready, audio_tx_free;
volatile uint32_t audio_deadline_misses, audio_error;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SAI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// send log message via UART
int __io_putchar(int ch)
{
    uint8_t byte = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &byte, 1, 100);
    return ch;
}


/* Burst diagnostic: keep receiving through ADC startup, stop before UART output.
 * This deliberately restarts the clocks each burst; it is not a recorder.
 */

//static int32_t signed24(uint32_t word)
//{
//    uint32_t value = word & 0x00FFFFFFu;
//
//    return (value & 0x00800000u)
//        ? (int32_t)value - 16777216
//        : (int32_t)value;
//}

//static void adc_diagnostic(uint32_t *buffer, uint16_t words)
//{
//    HAL_StatusTypeDef status = HAL_OK;
//    uint32_t blocks = 0;
//    uint32_t start = HAL_GetTick();
//
//    /* Allow digital startup plus input coupling and high-pass filter settling.
//     * The previous 300 ms capture still showed a large decaying DC transient.
//     * Drain the FIFO continuously, rather than delaying with SAI running.
//     */
//    do
//    {
//        status = HAL_SAI_Receive(&hsai_BlockA1, (uint8_t *)buffer, words, 100);
//        ++blocks;
//    } while (status == HAL_OK && (HAL_GetTick() - start) < 3000U);
//
//    uint32_t sr = hsai_BlockA1.Instance->SR;
//    uint32_t error = hsai_BlockA1.ErrorCode;
//    HAL_StatusTypeDef stopped = HAL_SAI_Abort(&hsai_BlockA1);
//    printf("ADC status=%d stop=%d blocks=%lu error=%08lX SR=%08lX overrun=%u\r\n",
//           (int)status, (int)stopped, (unsigned long)blocks,
//           (unsigned long)error, (unsigned long)sr,
//           (unsigned int)((sr & SAI_FLAG_OVRUDR) != 0U));
//    if (stopped != HAL_OK)
//    {
//        Error_Handler();
//    }
//    if (status != HAL_OK)
//    {
//        return;
//    }
//
//    for (uint32_t channel = 0; channel < 2U; ++channel)
//    {
//        int32_t minimum = 8388607;
//        int32_t maximum = -8388608;
//        uint32_t zeros = 0;
//        uint32_t ones = 0;
//        uint32_t clipped = 0;
//        for (uint32_t i = channel; i < words; i += 2U)
//        {
//            int32_t value = signed24(buffer[i]);
//            if (value < minimum) minimum = value;
//            if (value > maximum) maximum = value;
//            if (value == 0) ++zeros;
//            if (value == -1) ++ones;
//            if (value == -8388608 || value == 8388607) ++clipped;
//        }
//        printf("%c min=%ld max=%ld pp=%ld zero=%lu minus1=%lu clipped=%lu / %u\r\n",
//               channel == 0U ? 'L' : 'R', (long)minimum, (long)maximum,
//               (long)(maximum - minimum), (unsigned long)zeros,
//               (unsigned long)ones, (unsigned long)clipped,
//               (unsigned int)(words / 2U));
//    }
//    for (uint32_t frame = 192; frame < 200; ++frame)
//    {
//        printf("raw %08lX,%08lX signed %ld,%ld\r\n",
//               (unsigned long)buffer[2U * frame],
//               (unsigned long)buffer[2U * frame + 1U],
//               (long)signed24(buffer[2U * frame]),
//               (long)signed24(buffer[2U * frame + 1U]));
//    }
//}

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

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SAI1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  setvbuf(stdout, NULL, _IONBF, 0);

  /* OLED settings */
  HAL_Delay(100);  // Allow the OLED supply to settle

  if (HAL_I2C_IsDeviceReady(&hi2c1, (0x3C << 1), 3, 100) == HAL_OK)
  {
      printf("OLED detected\r\n");

      ssd1306_Init();
      ssd1306_Fill(Black);

      ssd1306_SetCursor(0, 0);
      ssd1306_WriteString("Hello STM32!", Font_7x10, White);

      ssd1306_SetCursor(0, 16);
      ssd1306_WriteString("H723 + OLED", Font_7x10, White);

      ssd1306_UpdateScreen();
  }
  else
  {
      printf("OLED not detected at 0x3C\r\n");
  }

  /* DAC test sine wave buffer */
//  static uint32_t tone[480 * 2];
//  for (uint32_t i = 0; i < 480; ++i)
//  {
//      float phase = 6.28318530718f * (float)(i % 48) / 48.0f;
//
//      /* Signed 24-bit PCM at 10% of full scale. */
//      int32_t sample = (int32_t)(8388607.0f * 0.1f * sinf(phase));
//      uint32_t packed = (uint32_t)sample & 0x00FFFFFFu;
//
//      tone[2 * i]     = packed;  /* Left */
//      tone[2 * i + 1] = packed;  /* Right */
//  }

  /* Separate aligned buffers live in RAM_D1 through the linker .bss section.
   * Prevent dirty CPU cache lines from overwriting incoming DMA data.
   */
  if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
  {
      SCB_CleanInvalidateDCache_by_Addr(adc_buffer, sizeof(adc_buffer));
      SCB_CleanDCache_by_Addr(dac_buffer, sizeof(dac_buffer));
  }
  __DSB();
  HAL_NVIC_SetPriority(SAI1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(SAI1_IRQn);

  /* Start once: TX preloads silence before RX master starts shared clocks. */
  if (HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t *)dac_buffer,
                           AUDIO_WORDS) != HAL_OK) Error_Handler();
  if (HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)adc_buffer,
                          AUDIO_WORDS) != HAL_OK) Error_Handler();

  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    if (audio_error != 0U) Error_Handler();
    __WFI();
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
  PeriphClkInitStruct.PLL2.PLL2M = 6;
  PeriphClkInitStruct.PLL2.PLL2N = 47;
  PeriphClkInitStruct.PLL2.PLL2P = 4;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 1523;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10C0ECFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SAI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SAI1_Init(void)
{

  /* USER CODE BEGIN SAI1_Init 0 */

  /* USER CODE END SAI1_Init 0 */

  /* USER CODE BEGIN SAI1_Init 1 */

  /* USER CODE END SAI1_Init 1 */
  hsai_BlockA1.Instance = SAI1_Block_A;
  hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_RX;
  hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
  hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
  hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
  if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_24BIT, 2) != HAL_OK)
  {
    Error_Handler();
  }
  hsai_BlockB1.Instance = SAI1_Block_B;
  hsai_BlockB1.Init.AudioMode = SAI_MODESLAVE_TX;
  hsai_BlockB1.Init.Synchro = SAI_SYNCHRONOUS;
  hsai_BlockB1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockB1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockB1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockB1.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockB1.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockB1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  if (HAL_SAI_InitProtocol(&hsai_BlockB1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_24BIT, 2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI1_Init 2 */
  hsai_BlockA1.Init.MckOutput = SAI_MCK_OUTPUT_ENABLE;
  hsai_BlockA1.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
  if (HAL_SAI_Init(&hsai_BlockA1) != HAL_OK) Error_Handler();

  /* USER CODE END SAI1_Init 2 */

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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMAMUX1_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMAMUX1_OVR_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMAMUX1_OVR_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PG7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* Both DMA IRQs must have equal preemption priority. No blocking work here.
 * A half contains 128 stereo frames, giving about 2.67 ms to copy at 48 kHz.
 */
static void audio_copy_half(uint32_t half)
{
    uint32_t bit = 1U << half;
    if ((audio_rx_ready & audio_tx_free & bit) == 0U) return;
    uint32_t rx = __HAL_DMA_GET_COUNTER(&hdma_sai1_a);
    uint32_t tx = __HAL_DMA_GET_COUNTER(&hdma_sai1_b);
    /* Completed half must be inactive in BOTH circular DMA streams. */
    uint32_t safe = half == 0U
        ? (rx > 0U && rx <= AUDIO_HALF_WORDS && tx > 0U && tx <= AUDIO_HALF_WORDS)
        : (rx > AUDIO_HALF_WORDS && tx > AUDIO_HALF_WORDS);
    audio_rx_ready &= ~bit;
    audio_tx_free &= ~bit;
    if (!safe) { ++audio_deadline_misses; return; }
    uint32_t offset = half * AUDIO_HALF_WORDS;
    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
        SCB_InvalidateDCache_by_Addr(&adc_buffer[offset], AUDIO_HALF_WORDS * 4U);
    for (uint32_t i = 0; i < AUDIO_HALF_WORDS; ++i)
        dac_buffer[offset + i] = adc_buffer[offset + i];
    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
        SCB_CleanDCache_by_Addr(&dac_buffer[offset], AUDIO_HALF_WORDS * 4U);
    __DSB();
}

static void audio_rx_complete(uint32_t half)
{
    uint32_t expired = 1U << (half ^ 1U);
    if ((audio_rx_ready & expired) != 0U) ++audio_deadline_misses;
    audio_rx_ready = (audio_rx_ready & ~expired) | (1U << half);
    audio_copy_half(half);
}

static void audio_tx_complete(uint32_t half)
{
    /* Revoke permission for the half that TX has now started consuming. */
    audio_tx_free = (audio_tx_free & ~(1U << (half ^ 1U))) | (1U << half);
    audio_copy_half(half);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &hsai_BlockA1) audio_rx_complete(0U);
}
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &hsai_BlockA1) audio_rx_complete(1U);
}
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &hsai_BlockB1) audio_tx_complete(0U);
}
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &hsai_BlockB1) audio_tx_complete(1U);
}
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    audio_error |= hsai->ErrorCode;
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
#ifdef USE_FULL_ASSERT
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
