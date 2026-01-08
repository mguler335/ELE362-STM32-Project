/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "sine_model.h"
#include "sine_model_data.h"
#include <stdio.h>
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
CRC_HandleTypeDef hcrc;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// --- TINYML MODEL OBJECTS & BUFFERS ---
ai_handle sine_model = AI_HANDLE_NULL; // Yapay zeka modelinin ana nesnesi (Handle)
AI_ALIGNED(32) ai_u8 activations[AI_SINE_MODEL_DATA_ACTIVATIONS_SIZE]; // Modelin ara işlem belleği (Activation Buffer)
AI_ALIGNED(32) ai_float in_data[AI_SINE_MODEL_IN_1_SIZE];
AI_ALIGNED(32) ai_float out_data[AI_SINE_MODEL_OUT_1_SIZE];

// X-CUBE-AI kütüphane Pointerları
ai_buffer *ai_input;
ai_buffer *ai_output;

// Kontrol Bayrakları ve mantıksal değişkenler
volatile uint8_t recording_active = 1; // 1: Timer veri topluyor, 0: Veri toplama bitti
volatile uint8_t sample_index = 0;     // O anki örnekleme indeksi (0-39 arası)
volatile uint8_t inference_ready = 0;  // 1: Veri hazır, yapay zeka çalışabilir

// Morse decoding
char letter_buffer[10] = "";    // Anlık harf parçalarını tutar (Örn: ".-..")
int letter_idx = 0;             // Harf tamponu indeksi
char final_sentence[100] = "";  // Çözümlenen cümlenin tamamı
int sentence_idx = 0;           // Cümle indeksi
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

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
  MX_CRC_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
      // Timer Kesmesini Başlat (25ms periyotla veri toplamak için)
      HAL_TIM_Base_Start_IT(&htim3);

      // 1. Model Parametrelerini Yapılandır
      ai_network_params params = {0};
      params.params.data = AI_HANDLE_PTR(ai_sine_model_data_weights_get()); // Ağırlıkları (Weights) yükle
      params.activations.data = AI_HANDLE_PTR(activations); // Aktivasyon belleğini bağla

      // 2. Modeli Oluştur
      ai_error err;
      err = ai_sine_model_create(&sine_model, AI_SINE_MODEL_DATA_CONFIG);
      if (err.type != AI_ERROR_NONE) {
          while(1);
      }

      // 3. Modeli Başlat
      if (!ai_sine_model_init(sine_model, &params)) {
          while(1);
      }

      // 4. Giriş/Çıkış Tamponlarını Bağla
      ai_input = ai_sine_model_inputs_get(sine_model, NULL);
      ai_output = ai_sine_model_outputs_get(sine_model, NULL);

      // Kullanıcı veri dizilerini modelin giriş/çıkış pointer'larına ata
      ai_input[0].data = AI_HANDLE_PTR(in_data);
      ai_output[0].data = AI_HANDLE_PTR(out_data);

      // --- AI MODEL INITIALIZATION END ---
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
      while (1)
      {
          if(inference_ready)
          {
              // 1. YAPAY ZEKAYI ÇALIŞTIR
              ai_i32 batch = ai_sine_model_run(sine_model, ai_input, ai_output);

              if (batch != 1) {

              }

              // 2. EN YÜKSEK OLASILIĞI BUL
              float max_prob = 0.0f;
              int result_class = -1;

              for(int i=0; i<4; i++) {
                  if(out_data[i] > max_prob) {
                      max_prob = out_data[i];
                      result_class = i;
                  }
              }


              // Eğer yapay zeka %50'den emin değilse, sonucu zorla "BOŞ" (3) yap.
              if(max_prob < 0.50f)
              {
                  result_class = 3; // 3: Boş (Sessizlik)
              }


              // 3. MORS ÇÖZÜMLEME
              if(result_class == 0) // NOKTA (.)
              {
                  letter_buffer[letter_idx++] = '.';
              }
              else if(result_class == 2) // ÇİZGİ (-)
              {
                  letter_buffer[letter_idx++] = '-';
              }
              else if(result_class == 3) // BOŞ (Harf Sonu)
              {
                  // Tampon doluysa harfe çevir
                  if(letter_idx > 0)
                  {
                      letter_buffer[letter_idx] = '\0'; // Stringi kapat

                      char decoded = '?'; // Bilinmeyen girdi olursa soru işareti koy

                      // Sözlük
                      if(strcmp(letter_buffer, ".-") == 0) decoded = 'A';
                      else if(strcmp(letter_buffer, "-...") == 0) decoded = 'B';
                      else if(strcmp(letter_buffer, "-.-.") == 0) decoded = 'C';
                      else if(strcmp(letter_buffer, "-..") == 0) decoded = 'D';
                      else if(strcmp(letter_buffer, ".") == 0) decoded = 'E';
                      else if(strcmp(letter_buffer, "..-.") == 0) decoded = 'F';
                      else if(strcmp(letter_buffer, "--.") == 0) decoded = 'G';
                      else if(strcmp(letter_buffer, "....") == 0) decoded = 'H';
                      else if(strcmp(letter_buffer, "..") == 0) decoded = 'I';
                      else if(strcmp(letter_buffer, ".---") == 0) decoded = 'J';
                      else if(strcmp(letter_buffer, "-.-") == 0) decoded = 'K';
                      else if(strcmp(letter_buffer, ".-..") == 0) decoded = 'L';
                      else if(strcmp(letter_buffer, "--") == 0) decoded = 'M';
                      else if(strcmp(letter_buffer, "-.") == 0) decoded = 'N';
                      else if(strcmp(letter_buffer, "---") == 0) decoded = 'O';
                      else if(strcmp(letter_buffer, ".--.") == 0) decoded = 'P';
                      else if(strcmp(letter_buffer, "--.-") == 0) decoded = 'Q';
                      else if(strcmp(letter_buffer, ".-.") == 0) decoded = 'R';
                      else if(strcmp(letter_buffer, "...") == 0) decoded = 'S';
                      else if(strcmp(letter_buffer, "-") == 0) decoded = 'T';
                      else if(strcmp(letter_buffer, "..-") == 0) decoded = 'U';
                      else if(strcmp(letter_buffer, "...-") == 0) decoded = 'V';
                      else if(strcmp(letter_buffer, ".--") == 0) decoded = 'W';
                      else if(strcmp(letter_buffer, "-..-") == 0) decoded = 'X';
                      else if(strcmp(letter_buffer, "-.--") == 0) decoded = 'Y';
                      else if(strcmp(letter_buffer, "--..") == 0) decoded = 'Z';

                      // Cümleye ekle
                      final_sentence[sentence_idx++] = decoded;
                      final_sentence[sentence_idx] = '\0';

                      // Temizle
                      letter_idx = 0;
                      memset(letter_buffer, 0, 10);
                  }
              }
              else if(result_class == 1) // ÇİFT TIK (Boşluk)
              {
                  final_sentence[sentence_idx++] = ' ';
                  final_sentence[sentence_idx] = '\0';
              }


              // 4. SONRAKİ KAYIT İÇİN HAZIRLAN
                        inference_ready = 0;


                        // Her yeni kayıt başladığında LED durum değiştirsin.
                        // Kullanıcıya "yeni 1 saniyelik pencere başladı" bilgisini vermek için durum LED'ini toggle ediyorum.
                        // LED değiştiği anda kullanıcı nokta/çizgi/çift tık girişini o pencere boyunca uyguluyor.

                        HAL_GPIO_TogglePin(Status_LED_GPIO_Port, Status_LED_Pin);
                        // --------------------------

                        recording_active = 1; // Kaydı başlat
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 48000 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 25 - 1;
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
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Status_LED_GPIO_Port, Status_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Blue_Button_Pin */
  GPIO_InitStruct.Pin = Blue_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Blue_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Status_LED_Pin */
  GPIO_InitStruct.Pin = Status_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Status_LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
    	// 25 ms aralıklarla buton durumunu örnekle
    	// 40 örnek ≈ 1 saniyelik pencere oluşturuyor; model giriş boyutu buna göre (AI_SINE_MODEL_IN_1_SIZE).
    	// HAL_GetTick kullanılmıyor; örnekleme tamamen timer kesmesiyle yapılıyor.

        if(recording_active)
        {
            // 1. BUTONU OKU
            GPIO_PinState btn_state = HAL_GPIO_ReadPin(Blue_Button_GPIO_Port, Blue_Button_Pin);

            // 2. TERSLEME VE KAYDETME

            if(btn_state == GPIO_PIN_RESET) // Buton BASILI (0 Volt)
            {
                in_data[sample_index] = 1.0f;
            }
            else // Buton BIRAKILDI (3.3 Volt)
            {
                in_data[sample_index] = 0.0f;
            }

            sample_index++;

            // 3. 1 Saniye Doldu mu kontrol (40 Örnek)
            if(sample_index >= AI_SINE_MODEL_IN_1_SIZE)
            {
                sample_index = 0;
                recording_active = 0; // Kaydı durdur
                inference_ready = 1;  // Tahmin yap
            }
        }
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
