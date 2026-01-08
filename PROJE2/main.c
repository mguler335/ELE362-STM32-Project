/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// --- ASANSÖR DEĞİŞKENLERİ ---
volatile int current_floor = 0;   // Mevcut Kat (0-9)
volatile int target_floor = -1;   // Hedef Kat (-1: Hedef Yok)
int stop_requests[10] = {0};      // Ara durak listesi

// Durumlar: 0: BEKLE (IDLE), 1: YUKARI (UP), 2: AŞAĞI (DOWN)
volatile int elevator_state = 0;

// Zaman Sayaçları (10ms'lik birimler)
volatile int led_timer = 0;       // LED yanıp sönme hızı için
volatile int move_timer = 0;      // Kat değiştirme süresi için
volatile int wait_timer = 0;      // Katta bekleme süresi için

// --- KEYPAD DEĞİŞKENLERİ ---
char key_map[4][3] = {
    {'1', '2', '3'},  // Satır 1
    {'4', '5', '6'},  // Satır 2
    {'7', '8', '9'},  // Satır 3
    {'*', '0', '#'}   // Satır 4
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
char Read_Keypad(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim3); // Zamanlayıcıyı başlat
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // 1. TUŞ TAKIMINI OKU VE HEDEF BELİRLE
      char key = Read_Keypad();

      if(key != 0 && key >= '0' && key <= '9')
      {
          int requested_floor = key - '0';

          // A. ACİL DURUM: Bulunduğum kata basıldıysa
          if(requested_floor == current_floor)
          {
              if(elevator_state != 0)
              {
                  elevator_state = 0; // Dur
                  wait_timer = 300;   // Kapıyı aç (3 sn)
                  stop_requests[current_floor] = 0; // İsteği temizle
                  if(target_floor == current_floor) target_floor = -1;
              }
              continue;
          }

          // B. İSTEĞİ KAYDET
          stop_requests[requested_floor] = 1;

          // C. HEDEF YÖNETİMİ
          if(target_floor == -1)
          {
              target_floor = requested_floor;
          }
          else
          {
              // Mevcut hedeften daha uzağa bir istek gelirse hedefi güncelle
              int current_direction = elevator_state;
              if(current_direction == 0) // Duruyorsak yön bul
              {
                  if(target_floor > current_floor) current_direction = 1;
                  else current_direction = 2;
              }

              if(current_direction == 1 && requested_floor > target_floor)
              {
                  target_floor = requested_floor; // Hedefi uzat
              }
              else if(current_direction == 2 && requested_floor < target_floor)
              {
                  target_floor = requested_floor; // Hedefi uzat
              }
          }
      }

      // 2. HAREKET KARARI VER (STATE MACHINE)
      if(elevator_state == 0 && target_floor != -1 && wait_timer == 0)
      {
          if(target_floor > current_floor)
          {
              elevator_state = 1; // YUKARI
          }
          else if(target_floor < current_floor)
          {
              elevator_state = 2; // AŞAĞI
          }
      }

  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 47999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
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

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(Asansor_LED_GPIO_Port, Asansor_LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = Asansor_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Asansor_LED_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
// Keypad Okuma Fonksiyonu
char Read_Keypad(void)
{
    for(int col=0; col<3; col++)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, (GPIO_PIN_4 << col), GPIO_PIN_RESET);
        HAL_Delay(1); // Sinyal oturma süresi

        for(int row=0; row<4; row++)
        {
            if(HAL_GPIO_ReadPin(GPIOC, (GPIO_PIN_0 << row)) == GPIO_PIN_RESET)
            {
                HAL_Delay(30); // Debounce
                if(HAL_GPIO_ReadPin(GPIOC, (GPIO_PIN_0 << row)) == GPIO_PIN_RESET)
                {
                    while(HAL_GPIO_ReadPin(GPIOC, (GPIO_PIN_0 << row)) == GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);
                    return key_map[row][col];
                }
            }
        }
    }
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);
    return 0;
}

// Timer Kesmesi (10ms)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        led_timer++;
        // LED Kontrolü
        if(elevator_state == 1 && led_timer >= 50) { // YUKARI
            HAL_GPIO_TogglePin(Asansor_LED_GPIO_Port, Asansor_LED_Pin); led_timer = 0;
        } else if(elevator_state == 2 && led_timer >= 20) { // AŞAĞI
            HAL_GPIO_TogglePin(Asansor_LED_GPIO_Port, Asansor_LED_Pin); led_timer = 0;
        } else if(elevator_state == 0) {
            HAL_GPIO_WritePin(Asansor_LED_GPIO_Port, Asansor_LED_Pin, GPIO_PIN_SET);
        }

        // Hareket Mantığı
        if(elevator_state != 0)
        {
            move_timer++;
            int required_time = (elevator_state == 1) ? 500 : 200;

            if(move_timer >= required_time)
            {
                move_timer = 0;
                if(elevator_state == 1) current_floor++; else current_floor--;

                // A. Hedefe Vardık mı kontrol
                if(current_floor == target_floor)
                {
                    elevator_state = 0;
                    wait_timer = 300;
                    stop_requests[current_floor] = 0;
                    target_floor = -1;

                    // Kata ulaşınca yeni bir istek var mı kontrol
                    for(int i=0; i<10; i++) {
                         if(stop_requests[i] == 1) { target_floor = i; break; }
                    }
                }
                // B. Ara Durak var mı kontrol
                else if(stop_requests[current_floor] == 1)
                {
                    elevator_state = 0;
                    wait_timer = 300;
                    stop_requests[current_floor] = 0;
                }
            }
        }

        // Bekleme Sayacı
        if(wait_timer > 0) wait_timer--;
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
