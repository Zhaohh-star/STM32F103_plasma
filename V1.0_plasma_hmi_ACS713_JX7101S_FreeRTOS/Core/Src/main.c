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
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tjc_usart_hmi.h"
#include <stdio.h>
#include <string.h>
#include "pwm_hal.h"
#include "gear_switch_hal.h"
#include "pulse_control_hal.h"
#include "foot_pedal_hal.h"
#include "ACS712.h"
#include "current_filter.h"
#include <math.h>
#include "JX7101S.h"
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
// === 1. 系统控制变量 ===
uint8_t rx_byte;

// === 新增的状态机变量放在这里 ===
/* 定义接收状态机的状态 */
uint8_t rx_state = 0;       // 0:找帧头, 1:找类型, 2:接收数据, 3:找帧尾1, 4:找帧尾2
uint8_t cmd_type = 0;       // 暂存指令类型
uint8_t data_buf[4];        // 暂存4字节数据
uint8_t data_idx = 0;       // 数据索引

// 这里的 PWM 变量建议保留，用于主循环或调试查看
volatile uint32_t current_freq = 100000;      // 默认上电：100KHZ
volatile uint32_t current_duty = 50;          // 默认占空比：100KHZ

// 系统使能标志位 (0:停止/关机, 1:运行/开机)
// 上电默认是 0，保证安全
uint8_t system_enable = 0;

//用于记录上一次的硬件档位，初始化为一个不可能的值(比如255)，确保开机能同步一次
uint8_t last_mode = 255;


// === 新增：电流采样变量 ===
#define BLOCK_SIZE    200
#define TOTAL_BUFFER  (BLOCK_SIZE * 2)

uint16_t adc_buffer[TOTAL_BUFFER]; // 双倍大小的大缓冲区 全局变量 存放在全局静态存储区

ACS712_t myCurrentSensor;          // 传感器对象
CurrentFilter_t myFilter;          // 滤波器对象

float current_display = 0.0f;      // 最终滤波后的值，用于显示/保护 最终电流显示

float voltage_display = 0.0f;      // 最终电压显示

extern osSemaphoreId_t Sem_ADC_HalfHandle;
extern osSemaphoreId_t Sem_ADC_FullHandle;

// 引入 CubeMX 帮我们生成的队列句柄
extern osMessageQueueId_t PulseQueueHandle;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void Bluetooth_Send_Current(float *current);
void HMI_Sync_Hardware_Mode(uint8_t current_gear);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ============================================================
// 1. HMI 底层发送函数 (自动加 ff ff ff)
// ============================================================
void HMI_Send_Cmd(char* cmd_str)
{
    // 发送字符串
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd_str, strlen(cmd_str), 100);

    // 发送结束符
    uint8_t end_code[3] = {0xFF, 0xFF, 0xFF};
    HAL_UART_Transmit(&huart2, end_code, 3, 100);
}

// ============================================================
// 2. 核心逻辑：根据硬件档位，设置屏幕图标
// ============================================================
void HMI_Sync_Hardware_Mode(uint8_t current_gear)
{
    // 1. 强制锁定 bt2 为消融 (保持不变)
    HMI_Send_Cmd("bt2.val=0");

    // 2. 根据 gear 切换 bt1
    switch(current_gear)
    {
        // --------------------------------------------
        // 情况 A: 收到 1 -> 代表单极 (Monopolar)
        // --------------------------------------------
        case 1:
            HMI_Send_Cmd("bt1.val=0"); // 屏幕显示单极图标
            break;

        // --------------------------------------------
        // 情况 B: 收到 2 -> 代表双极 (Bipolar)
        // --------------------------------------------
        case 2:
            HMI_Send_Cmd("bt1.val=1"); // 屏幕显示双极图标
            break;

        // --------------------------------------------
        // 容错处理 (比如刚上电可能是0或255)
        // --------------------------------------------
        default:
            // 默认显示单极，比较安全
            HMI_Send_Cmd("bt1.val=0");
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        // 1. 优先处理总开关指令 ('A' 和 'B')
        if(rx_byte == 'A')
        {
            system_enable = 1;
            PWM_Stop_HAL();
            rx_state = 0;
            HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
            return;
        }
        else if(rx_byte == 'B')
        {
            system_enable = 0;
            PWM_Stop_HAL();
            rx_state = 0;
            HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
            return;
        }

        // 2. 处理参数调节指令 (55 ...)
        switch(rx_state)
        {
            case 0: if(rx_byte == 0x55) rx_state = 1; break;
            case 1: cmd_type = rx_byte; data_idx = 0; rx_state = 2; break;
            case 2: data_buf[data_idx++] = rx_byte; if(data_idx >= 4) rx_state = 3; break;
            case 3: if(rx_byte == 0x0D) rx_state = 4; else rx_state = 0; break;

            case 4: // 帧尾 0x0A
                if(rx_byte == 0x0A)
                {
                    uint32_t val = (uint32_t)data_buf[0] | ((uint32_t)data_buf[1] << 8) |
                                   ((uint32_t)data_buf[2] << 16) | ((uint32_t)data_buf[3] << 24);

                    if(system_enable == 1)
                    {
                        if(cmd_type == 0x00) // 【调节频率】
                        {
                            PWM_SetFreq_HAL((float)val * 1000.0f);
                        }
                        else if(cmd_type == 0x01) // 【调节占空比】
                        {
                            PWM_SetDuty_HAL((float)val / 100.0f);
                        }
                        else if(cmd_type == 0x02) // 【发射脉冲】(重点修改在这里！)
                        {
                            // // 绝对不要在这里执行耗时操作！只竖起旗标并保存数据！
                            // target_pulse_counts = val;
                            // flag_trigger_pulse = 1;
                            // 1. 把接收到的值存入局部变量
                            uint32_t counts = val;

                            // 2. 将数据塞入消息队列
                            // 参数：队列句柄, 数据指针, 优先级(0), 超时时间(中断里必须是0!)
                            osMessageQueuePut(PulseQueueHandle, &counts, 0, 0);
                        }
                    }

                    // 强制下一轮主循环重刷屏幕图标（配合不崩溃的系统，这次它会生效了！）
                    last_mode = 255;
                    rx_state = 0;
                }
                else rx_state = 0;
                break;

            default: rx_state = 0;
        }
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

   // 1. 开启串口接收
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

    // 2. 初始化系统状态
    PWM_Stop_HAL();
    system_enable = 0; // 标记为关机状态 正常上电保证安全状态

    //system_enable = 1; // 标记为开机状态

    PWM_SetFreq_HAL(2000.0f);// 假设初始频率 2000Hz (人眼看不出闪烁)，占空比 50% (0.5f)
    PWM_SetDuty_HAL(0.5f);

    // ===3. 初始化电流传感器硬件 ===
      // 1. 初始化对象
      ACS712_Init(&myCurrentSensor, 0.1f, 3.3f);       // 20A 传感器
      CurrentFilter_Init(&myFilter, 0.1f, 0.1f);      // 带 0.1A 死区的滤波器

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
      // 你的系统现在由 FreeRTOS 全权接管，主循环正式光荣退休！
      osDelay(1000);

#if 0
	  // --- 01---PWM正常生成的测试代码开始：呼吸灯 ---

	        // 1. 逐渐变亮 (0% -> 100%)
	        for(int i = 0; i <= 100; i++)
	        {
	            PWM_SetDuty_HAL((float)i / 100.0f); // 设置占空比
	            HAL_Delay(10); // 延时一下，不然变化太快看不清
	        }

	        HAL_Delay(500); // 最亮停顿一下

	        // 2. 逐渐变暗 (100% -> 0%)
	        for(int i = 100; i >= 0; i--)
	        {
	            PWM_SetDuty_HAL((float)i / 100.0f);
	            HAL_Delay(10);
	        }

	        HAL_Delay(500); // 最暗停顿一下

#endif
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// ===============================================
// 1. DMA 传输过半中断回调 (前 200 个数据准备好了)
// ===============================================
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        // 【纯血 RTOS 架构兜底】释放信号量唤醒任务
        osStatus_t status = osSemaphoreRelease(Sem_ADC_HalfHandle);

        // 如果释放失败，说明任务还没来得及拿走上一次的钥匙！发生 Overrun(追尾)！
        if (status != osOK)
        {
            PWM_Stop_HAL();    // 硬件急停
            system_enable = 0; // 软件闭锁
        }
    }
}

// =============================================
// 2. DMA 传输完成中断回调 (后 200 个数据准备好了)
// =============================================
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        // 【纯血 RTOS 架构兜底】释放信号量唤醒任务
        osStatus_t status = osSemaphoreRelease(Sem_ADC_FullHandle);

        // 检测是否发生追尾
        if (status != osOK)
        {
            PWM_Stop_HAL();
            system_enable = 0;
        }
    }
}
// 定时器更新中断回调函数

// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
//     // 检查是不是 TIM1 产生的中断
//     if (htim->Instance == TIM1)
//     {
//         // 调用脉冲计数处理
//         PulseControl_TIM_IRQ_Handler();
//     }
// }
// 1. DMA 传输过半中断回调 (前 200 个数据准备好了)

// === 蓝牙发送函数 (只发电流) ===
void Bluetooth_Send_Current(float* current)
{
    char tx_buffer[32]={0}; // 缓冲区不用太大，32字节够了
    //char tx_buffer[32]={'a','b','c',13,10,0};
    // 格式化为：数值 + 换行符
    // 例如发送：0.450\r\n
    // 这种格式最适合 SerialPlot 或 串口助手 自动绘图

    sprintf(tx_buffer, "%.3f\r\n", *current);


    // 通过 USART1 (已重映射到 PB6/PB7) 发送
    HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	// 检查是不是 TIM1 产生的中断
	if (htim->Instance == TIM1)
	{
		// 调用脉冲计数处理
		PulseControl_TIM_IRQ_Handler();
	}
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
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
