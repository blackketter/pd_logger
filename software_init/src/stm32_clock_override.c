#include "stm32f1xx_hal.h"

/* 16 MHz HSE  -> PREDIV /2 = 8 MHz -> PLL x9 = 72 MHz
   APB1 = 36 MHz (<=36), APB2 = 72 MHz
*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();   // SWD aktiv, JTAG aus (optional)

  RCC_OscInitStruct.OscillatorType   = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState         = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue   = RCC_HSE_PREDIV_DIV2;  // 16 -> 8
  RCC_OscInitStruct.HSIState         = RCC_HSI_ON;           // Backup an
  RCC_OscInitStruct.PLL.PLLState     = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource    = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL       = RCC_PLL_MUL9;         // 8*9 = 72
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;  // HCLK 72
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;    // PCLK1 36
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // PCLK2 72
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}