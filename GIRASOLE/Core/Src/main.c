/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

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
#define SERIAL
#define BLUETOOTH
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

DFSDM_Channel_HandleTypeDef hdfsdm1_channel1;

I2C_HandleTypeDef hi2c2;

QSPI_HandleTypeDef hqspi;

SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

osThreadId ReadLeftPanelHandle;
osThreadId ReadRightPanelHandle;
osThreadId SerialDebugHandle;
osThreadId SyncButtonTaskHandle;
osThreadId ledTask1Handle;
osThreadId ledTask2Handle;
osThreadId ledTask3Handle;
osMutexId MutexPDHandle;
osMutexId panelsMutexHandle;
osSemaphoreId rpanel_semHandle;
osSemaphoreId lpanel_semHandle;
osSemaphoreId led1_semHandle;
osSemaphoreId led2_semHandle;
osSemaphoreId led3_semHandle;
osSemaphoreId button_semHandle;
/* USER CODE BEGIN PV */
uint8_t* Rx_Data[1];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DFSDM1_Init(void);
static void MX_I2C2_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_SPI3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_UART4_Init(void);
void StartReadLeftPanel(void const * argument);
void StartReadRightPanel(void const * argument);
void StartSerialDebug(void const * argument);
void StartSyncButton(void const * argument);
void StartLedTask1(void const * argument);
void StartLedTask2(void const * argument);
void StartLedTask3(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Structure for panels data
struct panelsData {
	int rightPanelValue, leftPanelValue, threshold, variation;
} pd;

// Function to initialize panel data
void panelDataInit() {
	pd.leftPanelValue = 0;
	pd.rightPanelValue = 0;
	pd.threshold = 10;
	pd.variation = 10;
}

//Structure for precedence constraints management
struct prec_control {
	int lpw, rpw;
	int l1w, l2w, l3w;
	int lpanel_done, rpanel_done;
	int led1_done, led2_done, led3_done;
	int button_wait, button_can_do;
} pcm;

void prec_control_manag_init()
{
	pcm.lpw = pcm.rpw = 0;
	pcm.l1w = pcm.l2w = pcm.l3w = 0;
	pcm.lpanel_done = pcm.rpanel_done = 0;
	pcm.led1_done = pcm.led2_done = pcm.led3_done = 1;
	pcm.button_wait = pcm.button_can_do = 0;
}

// Variable on 1 if button has been pressed
extern int blue_button_pressed;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	// Initialize panels data structure
	panelDataInit();
	// Initialize control data structure
	prec_control_manag_init();
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
  MX_DFSDM1_Init();
  MX_I2C2_Init();
  MX_QUADSPI_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
  __HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&huart4, UART_IT_TC);
  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of MutexPD */
  osMutexDef(MutexPD);
  MutexPDHandle = osMutexCreate(osMutex(MutexPD));

  /* definition and creation of panelsMutex */
  osMutexDef(panelsMutex);
  panelsMutexHandle = osMutexCreate(osMutex(panelsMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  //osMutexWait(panelsMutexHandle, osWaitForever);
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of rpanel_sem */
  osSemaphoreDef(rpanel_sem);
  rpanel_semHandle = osSemaphoreCreate(osSemaphore(rpanel_sem), 1);

  /* definition and creation of lpanel_sem */
  osSemaphoreDef(lpanel_sem);
  lpanel_semHandle = osSemaphoreCreate(osSemaphore(lpanel_sem), 1);

  /* definition and creation of led1_sem */
  osSemaphoreDef(led1_sem);
  led1_semHandle = osSemaphoreCreate(osSemaphore(led1_sem), 1);

  /* definition and creation of led2_sem */
  osSemaphoreDef(led2_sem);
  led2_semHandle = osSemaphoreCreate(osSemaphore(led2_sem), 1);

  /* definition and creation of led3_sem */
  osSemaphoreDef(led3_sem);
  led3_semHandle = osSemaphoreCreate(osSemaphore(led3_sem), 1);

  /* definition and creation of button_sem */
  osSemaphoreDef(button_sem);
  button_semHandle = osSemaphoreCreate(osSemaphore(button_sem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of ReadLeftPanel */
  osThreadDef(ReadLeftPanel, StartReadLeftPanel, osPriorityNormal, 0, 128);
  ReadLeftPanelHandle = osThreadCreate(osThread(ReadLeftPanel), NULL);

  /* definition and creation of ReadRightPanel */
  osThreadDef(ReadRightPanel, StartReadRightPanel, osPriorityNormal, 0, 128);
  ReadRightPanelHandle = osThreadCreate(osThread(ReadRightPanel), NULL);

  /* definition and creation of SerialDebug */
  osThreadDef(SerialDebug, StartSerialDebug, osPriorityNormal, 0, 128);
  SerialDebugHandle = osThreadCreate(osThread(SerialDebug), NULL);

  /* definition and creation of SyncButtonTask */
  osThreadDef(SyncButtonTask, StartSyncButton, osPriorityAboveNormal, 0, 128);
  SyncButtonTaskHandle = osThreadCreate(osThread(SyncButtonTask), NULL);

  /* definition and creation of ledTask1 */
  osThreadDef(ledTask1, StartLedTask1, osPriorityNormal, 0, 128);
  ledTask1Handle = osThreadCreate(osThread(ledTask1), NULL);

  /* definition and creation of ledTask2 */
  osThreadDef(ledTask2, StartLedTask2, osPriorityNormal, 0, 128);
  ledTask2Handle = osThreadCreate(osThread(ledTask2), NULL);

  /* definition and creation of ledTask3 */
  osThreadDef(ledTask3, StartLedTask3, osPriorityNormal, 0, 128);
  ledTask3Handle = osThreadCreate(osThread(ledTask3), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_DFSDM1
                              |RCC_PERIPHCLK_USB|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_PCLK;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 24;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK|RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief DFSDM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DFSDM1_Init(void)
{

  /* USER CODE BEGIN DFSDM1_Init 0 */

  /* USER CODE END DFSDM1_Init 0 */

  /* USER CODE BEGIN DFSDM1_Init 1 */

  /* USER CODE END DFSDM1_Init 1 */
  hdfsdm1_channel1.Instance = DFSDM1_Channel1;
  hdfsdm1_channel1.Init.OutputClock.Activation = ENABLE;
  hdfsdm1_channel1.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_SYSTEM;
  hdfsdm1_channel1.Init.OutputClock.Divider = 2;
  hdfsdm1_channel1.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hdfsdm1_channel1.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
  hdfsdm1_channel1.Init.Input.Pins = DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS;
  hdfsdm1_channel1.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
  hdfsdm1_channel1.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hdfsdm1_channel1.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
  hdfsdm1_channel1.Init.Awd.Oversampling = 1;
  hdfsdm1_channel1.Init.Offset = 0;
  hdfsdm1_channel1.Init.RightBitShift = 0x00;
  if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DFSDM1_Init 2 */

  /* USER CODE END DFSDM1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10909CEC;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 255;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 1;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

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
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, M24SR64_Y_RF_DISABLE_Pin|M24SR64_Y_GPO_Pin|ISM43362_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ISM43362_BOOT0_Pin|ISM43362_WAKEUP_Pin|LED2_Pin|SPSGRF_915_SDN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, USB_OTG_FS_PWR_EN_Pin|PMOD_RESET_Pin|STSAFE_A100_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPBTLE_RF_SPI3_CSN_GPIO_Port, SPBTLE_RF_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, VL53L0X_XSHUT_Pin|LED3_WIFI__LED4_BLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPBTLE_RF_RST_GPIO_Port, SPBTLE_RF_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPSGRF_915_SPI3_CSN_GPIO_Port, SPSGRF_915_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ISM43362_SPI3_CSN_GPIO_Port, ISM43362_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : M24SR64_Y_RF_DISABLE_Pin M24SR64_Y_GPO_Pin ISM43362_RST_Pin ISM43362_SPI3_CSN_Pin */
  GPIO_InitStruct.Pin = M24SR64_Y_RF_DISABLE_Pin|M24SR64_Y_GPO_Pin|ISM43362_RST_Pin|ISM43362_SPI3_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OTG_FS_OVRCR_EXTI3_Pin SPSGRF_915_GPIO3_EXTI5_Pin ISM43362_DRDY_EXTI1_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_OVRCR_EXTI3_Pin|SPSGRF_915_GPIO3_EXTI5_Pin|ISM43362_DRDY_EXTI1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTBLUE_Pin */
  GPIO_InitStruct.Pin = BUTBLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTBLUE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ISM43362_BOOT0_Pin ISM43362_WAKEUP_Pin LED2_Pin SPSGRF_915_SDN_Pin
                           SPSGRF_915_SPI3_CSN_Pin */
  GPIO_InitStruct.Pin = ISM43362_BOOT0_Pin|ISM43362_WAKEUP_Pin|LED2_Pin|SPSGRF_915_SDN_Pin
                          |SPSGRF_915_SPI3_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : INTERNAL_UART3_TX_Pin INTERNAL_UART3_RX_Pin */
  GPIO_InitStruct.Pin = INTERNAL_UART3_TX_Pin|INTERNAL_UART3_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : LPS22HB_INT_DRDY_EXTI0_Pin LSM6DSL_INT1_EXTI11_Pin HTS221_DRDY_EXTI15_Pin PMOD_IRQ_EXTI12_Pin */
  GPIO_InitStruct.Pin = LPS22HB_INT_DRDY_EXTI0_Pin|LSM6DSL_INT1_EXTI11_Pin|HTS221_DRDY_EXTI15_Pin|PMOD_IRQ_EXTI12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OTG_FS_PWR_EN_Pin SPBTLE_RF_SPI3_CSN_Pin PMOD_RESET_Pin STSAFE_A100_RESET_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_PWR_EN_Pin|SPBTLE_RF_SPI3_CSN_Pin|PMOD_RESET_Pin|STSAFE_A100_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : VL53L0X_XSHUT_Pin LED3_WIFI__LED4_BLE_Pin */
  GPIO_InitStruct.Pin = VL53L0X_XSHUT_Pin|LED3_WIFI__LED4_BLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : VL53L0X_GPIO1_EXTI7_Pin LSM3MDL_DRDY_EXTI8_Pin */
  GPIO_InitStruct.Pin = VL53L0X_GPIO1_EXTI7_Pin|LSM3MDL_DRDY_EXTI8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SPBTLE_RF_RST_Pin */
  GPIO_InitStruct.Pin = SPBTLE_RF_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPBTLE_RF_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PMOD_SPI2_SCK_Pin */
  GPIO_InitStruct.Pin = PMOD_SPI2_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PMOD_SPI2_SCK_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void PostLedTasks()
{
    if (pcm.l1w)
    {
        pcm.l1w--;
        osSemaphoreRelease(led1_semHandle);
    }

    if (pcm.l2w)
    {
        pcm.l2w--;
        osSemaphoreRelease(led2_semHandle);
    }

    if (pcm.l3w)
    {
        pcm.l3w--;
        osSemaphoreRelease(led3_semHandle);
    }
}

void PostPanelTasks()
{
    if (pcm.lpw)
    {
        pcm.lpw--;
        osSemaphoreRelease(lpanel_semHandle);
    }

    if (pcm.rpw)
    {
        pcm.rpw--;
        osSemaphoreRelease(rpanel_semHandle);
    }
}

void ReadLPStart()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	if (pcm.lpanel_done)
		pcm.lpw++;
	else
		osSemaphoreRelease(lpanel_semHandle);
	osMutexRelease(MutexPDHandle);
	osSemaphoreWait(lpanel_semHandle, osWaitForever);
}

void ReadLPEnd()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	pcm.lpanel_done = 1;
	if (pcm.lpanel_done && pcm.rpanel_done)
	{
		pcm.button_can_do = 1;
		if (pcm.button_wait)
		{
			pcm.button_wait--;
			osSemaphoreRelease(button_semHandle);
		}
	 }
	osMutexRelease(MutexPDHandle);
}

void ReadRPStart()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	if (pcm.rpanel_done)
		pcm.rpw++;
	else
		osSemaphoreRelease(rpanel_semHandle);
	osMutexRelease(MutexPDHandle);
	osSemaphoreWait(rpanel_semHandle, osWaitForever);
}

void ReadRPEnd()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	pcm.rpanel_done = 1;
	if (pcm.lpanel_done && pcm.rpanel_done)
	{
		pcm.button_can_do = 1;
		if (pcm.button_wait)
		{
			pcm.button_wait--;
			osSemaphoreRelease(button_semHandle);
		}
	}
	osMutexRelease(MutexPDHandle);
}

void WriteL1Start()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	if (pcm.led1_done)
		pcm.l1w++;
	else
		osSemaphoreRelease(led1_semHandle);
	osMutexRelease(MutexPDHandle);
	osSemaphoreWait(led1_semHandle, osWaitForever);
}

void WriteL2Start()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	if (pcm.led2_done)
		pcm.l2w++;
	else
		osSemaphoreRelease(led2_semHandle);
	osMutexRelease(MutexPDHandle);
	osSemaphoreWait(led2_semHandle, osWaitForever);
}

void WriteL3Start()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	if (pcm.led3_done)
		pcm.l3w++;
	else
		osSemaphoreRelease(led3_semHandle);
	osMutexRelease(MutexPDHandle);
	osSemaphoreWait(led3_semHandle, osWaitForever);
}

void WriteL1End()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	pcm.led1_done = 1;
	if (pcm.led1_done && pcm.led2_done && pcm.led3_done)
	{
		pcm.lpanel_done = pcm.rpanel_done = 0; //sblocco i pannelli
		PostPanelTasks();
	}
	osMutexRelease(MutexPDHandle);
}

void WriteL2End()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	pcm.led2_done = 1;
	if (pcm.led1_done && pcm.led2_done && pcm.led3_done)
	{
		pcm.lpanel_done = pcm.rpanel_done = 0; //sblocco i pannelli
		PostPanelTasks();
	}
	osMutexRelease(MutexPDHandle);
}

void WriteL3End()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	pcm.led3_done = 1;
	if (pcm.led1_done && pcm.led2_done && pcm.led3_done)
	{
		pcm.lpanel_done = pcm.rpanel_done = 0; //sblocco i pannelli
		PostPanelTasks();
	}
	osMutexRelease(MutexPDHandle);
}

void StartReadButton()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	if (pcm.button_can_do)
		osSemaphoreRelease(button_semHandle);
	else
		pcm.button_wait++;
	osMutexRelease(MutexPDHandle);
	osSemaphoreWait(button_semHandle, osWaitForever);
}

void EndReadButton()
{
	osMutexWait(MutexPDHandle, osWaitForever);
	pcm.button_can_do = 0; //non può andare
	pcm.button_wait = 0; //non è più in attesa
	pcm.led1_done = pcm.led2_done = pcm.led3_done = 0; //indico che devono eseguire i led
	PostLedTasks();
	osMutexRelease(MutexPDHandle);
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartReadLeftPanel */
/**
  * @brief  Function implementing the ReadLeftPanel thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartReadLeftPanel */
void StartReadLeftPanel(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */

	for(;;)
	{
		ReadLPStart();

		 // Get left panel value
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
		pd.leftPanelValue = HAL_ADC_GetValue(&hadc2)*100/2400;

		ReadLPEnd();

		// Delay time (msec)
		osDelay(100);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartReadRightPanel */
/**
* @brief Function implementing the ReadRightPanel thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReadRightPanel */
void StartReadRightPanel(void const * argument)
{
  /* USER CODE BEGIN StartReadRightPanel */
  /* Infinite loop */
  for(;;)
  {
	ReadRPStart();
	 // Get right panel value
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	pd.rightPanelValue = HAL_ADC_GetValue(&hadc1)*100/2400;

	ReadRPEnd();

    osDelay(100);
  }
  /* USER CODE END StartReadRightPanel */
}

/* USER CODE BEGIN Header_StartSerialDebug */
/**
* @brief Function implementing the SerialDebug thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSerialDebug */
void StartSerialDebug(void const * argument)
{
  /* USER CODE BEGIN StartSerialDebug */
  /* Infinite loop */
  for(;;)
  {
	char msg[50];
	
	/** Update panel data structure values **/
	// Lock data reads semaphore
  	
	#ifdef SERIAL
	osMutexWait(MutexPDHandle, osWaitForever);
	
  	// Get data
	sprintf(msg, "Light Panel Right = %hu\r\nLight Panel Left = %hu\r\nThr = %hu\r\nVar = %hu\r\n", pd.rightPanelValue, pd.leftPanelValue, pd.threshold, pd.variation);

	// Unlock data reads semaphore
	osMutexRelease(MutexPDHandle);

	// Print data
	HAL_UART_Transmit(&huart1, (uint8_t *) msg, strlen(msg), HAL_MAX_DELAY);
	#endif
	
	#ifdef BLUETOOTH
    /** Update panel data structure values **/
	osMutexWait(MutexPDHandle, osWaitForever);
	
    // Get data
    sprintf(msg, "Light Panel Right = %hu\r\nLight Panel Left = %hu\r\nThr = %hu\r\nVar = %hu\r\n", pd.rightPanelValue, pd.leftPanelValue, pd.threshold, pd.variation);

	// Unlock data reads semaphore
    osMutexRelease(MutexPDHandle);
    
    // Print data
    HAL_UART_Transmit(&huart4, (uint8_t *) msg, strlen(msg), HAL_MAX_DELAY);
	#endif


	// Delay time (msec)
	osDelay(3000);
  }
  /* USER CODE END StartSerialDebug */
}

/* USER CODE BEGIN Header_StartSyncButton */
/**
* @brief Function implementing the SyncButtonTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSyncButton */
void StartSyncButton(void const * argument)
{
  /* USER CODE BEGIN StartSynkButton */
  /* Infinite loop */
  for(;;)
  {
	  StartReadButton();
	  if (blue_button_pressed) {
		  // Reset button pressed variable
		  blue_button_pressed = 0;
		  // Set threshold
		  pd.threshold = pd.leftPanelValue<pd.rightPanelValue?pd.leftPanelValue:pd.rightPanelValue;
		  // Set variation
		  pd.variation = abs(pd.leftPanelValue - pd.rightPanelValue);
	  }
	  EndReadButton();
	  // Delay time (msec)
	  osDelay(100);
  }
  /* USER CODE END StartSynkButton */
}

/* USER CODE BEGIN Header_StartLedTask1 */
/**
* @brief Function implementing the ledTask1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask1 */
void StartLedTask1(void const * argument)
{
  /* USER CODE BEGIN StartLedTask1 */
  /* Infinite loop */
  for(;;)
  {
	  WriteL1Start();
	  int lpv = pd.leftPanelValue;
	  int rpv = pd.rightPanelValue;
	  int var = pd.variation;
	  int th = pd.threshold;
	  if ((abs(rpv-lpv)<var) && (rpv>th && lpv>th))
	  {
		  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	  }
	  WriteL1End();
	  osDelay(100);
  }
  /* USER CODE END StartLedTask1 */
}

/* USER CODE BEGIN Header_StartLedTask2 */
/**
* @brief Function implementing the ledTask2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask2 */
void StartLedTask2(void const * argument)
{
  /* USER CODE BEGIN StartLedTask2 */
  /* Infinite loop */
  for(;;)
  {
	  WriteL2Start();
	  int lpv = pd.leftPanelValue;
	  int rpv = pd.rightPanelValue;
	  int var = pd.variation;
	  int th = pd.threshold;
	  if (abs(rpv-lpv)>var && (rpv>=lpv))
	  {
		  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	  	  HAL_GPIO_WritePin(LED3_WIFI__LED4_BLE_GPIO_Port, LED3_WIFI__LED4_BLE_Pin, GPIO_PIN_SET);
	  }
	  WriteL2End();
	  osDelay(100);
  }
  /* USER CODE END StartLedTask2 */
}

/* USER CODE BEGIN Header_StartLedTask3 */
/**
* @brief Function implementing the ledTask3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask3 */
void StartLedTask3(void const * argument)
{
  /* USER CODE BEGIN StartLedTask3 */
  /* Infinite loop */
  for(;;)
  {
	  WriteL3Start();
	  int lpv = pd.leftPanelValue;
	  int rpv = pd.rightPanelValue;
	  int var = pd.variation;
	  int th = pd.threshold;
	  if (abs(rpv-lpv)>var && rpv<lpv)
	  {
		  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(LED3_WIFI__LED4_BLE_GPIO_Port, LED3_WIFI__LED4_BLE_Pin, GPIO_PIN_RESET);
	  }
	  WriteL3End();
	  osDelay(100);
  }
  /* USER CODE END StartLedTask3 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
