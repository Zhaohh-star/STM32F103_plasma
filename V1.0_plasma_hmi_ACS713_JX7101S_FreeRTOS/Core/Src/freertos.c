/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ACS712.h"
#include "current_filter.h"
#include "pwm_hal.h"
#include <math.h>

#include "JX7101S.h"
#include "gear_switch_hal.h"
#include <stdio.h>
#include <string.h>

#include "foot_pedal_hal.h"
#include "pulse_control_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BLOCK_SIZE    200

// 看门狗打卡位定义
#define WDG_BIT_CURRENT   (1 << 0)  // 0x01 (二进制 001) - 电流任务打卡位
#define WDG_BIT_CONTROL   (1 << 1)  // 0x02 (二进制 010) - 控制任务打卡位
#define WDG_BIT_UI        (1 << 2)  // 0x04 (二进制 100) - 屏幕任务打卡位

// 三个任务都打完卡的状态：001 | 010 | 100 = 111 (0x07)
#define WDG_BITS_ALL      (WDG_BIT_CURRENT | WDG_BIT_CONTROL | WDG_BIT_UI)

#define TOTAL_BUFFER  (BLOCK_SIZE * 2)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern ACS712_t myCurrentSensor;
extern uint16_t adc_buffer[TOTAL_BUFFER];  // TOTAL_BUFFER 的大小是 400

extern float current_display;
extern CurrentFilter_t myFilter;
extern uint8_t system_enable;
extern void PWM_Stop_HAL(void); // 如果提示找不到函数，也 extern 一下

// 👇新增的外部变量：串口句柄、电压变量、还有你在 main.c 写的几个函数
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern float voltage_display;
extern uint8_t last_mode;
extern void Bluetooth_Send_Current(float *current);
extern void HMI_Sync_Hardware_Mode(uint8_t current_gear);

extern IWDG_HandleTypeDef hiwdg;

// 告诉编译器，这几个硬件外设句柄在 main.c 中已经存在了
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

// 告诉编译器，你的 ADC 缓冲区也在外面（注意数据类型，如果是 uint16_t 就写 uint16_t）
extern uint16_t adc_buffer[];

/* USER CODE END Variables */
/* Definitions for Task_UI_Comm */
osThreadId_t Task_UI_CommHandle;
const osThreadAttr_t Task_UI_Comm_attributes = {
  .name = "Task_UI_Comm",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Current */
osThreadId_t Task_CurrentHandle;
const osThreadAttr_t Task_Current_attributes = {
  .name = "Task_Current",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_Control */
osThreadId_t Task_ControlHandle;
const osThreadAttr_t Task_Control_attributes = {
  .name = "Task_Control",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Task_Watchdog */
osThreadId_t Task_WatchdogHandle;
const osThreadAttr_t Task_Watchdog_attributes = {
  .name = "Task_Watchdog",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for PulseQueue */
osMessageQueueId_t PulseQueueHandle;
const osMessageQueueAttr_t PulseQueue_attributes = {
  .name = "PulseQueue"
};
/* Definitions for Mutex_Current */
osMutexId_t Mutex_CurrentHandle;
const osMutexAttr_t Mutex_Current_attributes = {
  .name = "Mutex_Current"
};
/* Definitions for Sem_ADC_Half */
osSemaphoreId_t Sem_ADC_HalfHandle;
const osSemaphoreAttr_t Sem_ADC_Half_attributes = {
  .name = "Sem_ADC_Half"
};
/* Definitions for Sem_ADC_Full */
osSemaphoreId_t Sem_ADC_FullHandle;
const osSemaphoreAttr_t Sem_ADC_Full_attributes = {
  .name = "Sem_ADC_Full"
};
/* Definitions for EventGroup_WDG */
osEventFlagsId_t EventGroup_WDGHandle;
const osEventFlagsAttr_t EventGroup_WDG_attributes = {
  .name = "EventGroup_WDG"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartUICOmmTask(void *argument);
void StartCurrentTask(void *argument);
void StartControlTask(void *argument);
void StartWatchdogTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of Mutex_Current */
  Mutex_CurrentHandle = osMutexNew(&Mutex_Current_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Sem_ADC_Half */
  Sem_ADC_HalfHandle = osSemaphoreNew(1, 1, &Sem_ADC_Half_attributes);

  /* creation of Sem_ADC_Full */
  Sem_ADC_FullHandle = osSemaphoreNew(1, 1, &Sem_ADC_Full_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of PulseQueue */
  PulseQueueHandle = osMessageQueueNew (5, sizeof(uint32_t), &PulseQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_UI_Comm */
  Task_UI_CommHandle = osThreadNew(StartUICOmmTask, NULL, &Task_UI_Comm_attributes);

  /* creation of Task_Current */
  Task_CurrentHandle = osThreadNew(StartCurrentTask, NULL, &Task_Current_attributes);

  /* creation of Task_Control */
  Task_ControlHandle = osThreadNew(StartControlTask, NULL, &Task_Control_attributes);

  /* creation of Task_Watchdog */
  Task_WatchdogHandle = osThreadNew(StartWatchdogTask, NULL, &Task_Watchdog_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of EventGroup_WDG */
  EventGroup_WDGHandle = osEventFlagsNew(&EventGroup_WDG_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartUICOmmTask */
/**
  * @brief  Function implementing the Task_UI_Comm thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartUICOmmTask */
void StartUICOmmTask(void *argument)
{
  /* USER CODE BEGIN StartUICOmmTask */

  /* Infinite loop */
  for(;;)
  {
  	// ============================================================
  	// 1. 【核心安全保护】使用替身安全读取全局变量
  	// ============================================================
  	float safe_current = 0.0f; // 准备一个局部替身变量

  	// 顾客要进厕所了，尝试拿钥匙 (等待最多 10ms)
  	if (osMutexAcquire(Mutex_CurrentHandle, 10) == osOK)
  	{
  		safe_current = current_display;      // 瞬间把数据拷贝到自己的替身里
  		osMutexRelease(Mutex_CurrentHandle); // 拷贝完立刻交出钥匙！绝不占着茅坑不拉屎！
  	}

  	// 2. 读取电压 (这是阻塞操作，放在这个低优先级任务里绝对安全，不会卡控制)
    //	float v_read = JX7101S_ReadVoltage(&huart3);
  	// 强行给一个假电压，先让程序跑下去！
  	float v_read = 12.34f;

  	if(v_read >= 0.0f) {
  		voltage_display = v_read;
  	} else {
  		voltage_display = 0.0f;
  	}

  	// ============================================================
  	// 3. 发送数据到串口屏 (使用刚刚拿到的替身 safe_current)
  	// ============================================================

#if 0
  	uint8_t cmd_buffer[100];

  	if (system_enable == 1)
  	{
  		// 注意这里：用的是 safe_current，而不是 current_display 啦！
  		sprintf((char*)cmd_buffer, "t8.txt=\"%.2f\"\xff\xff\xfft9.txt=\"%.2f\"\xff\xff\xff", safe_current, voltage_display);
  	}
  	else
  	{
  		sprintf((char*)cmd_buffer, "t8.txt=\"0.00\"\xff\xff\xfft9.txt=\"0.00\"\xff\xff\xff");
  	}
  	HAL_UART_Transmit(&huart2, cmd_buffer, strlen((char*)cmd_buffer), 100);
#endif
  	// 3. 安全发送数据到串口屏 (使用整数替换法，告别死机！)
  	// ============================================================
  	uint8_t cmd_buffer[100];

  	if (system_enable == 1)
  	{
  		// 把电流和电压拆成 整数部分 和 小数部分
  		int cur_int = (int)safe_current;                         // 电流整数部分
  		int cur_dec = (int)(safe_current * 100.0f) % 100;        // 电流小数部分 (2位)

  		int vol_int = (int)voltage_display;                      // 电压整数部分
  		int vol_dec = (int)(voltage_display * 100.0f) % 100;     // 电压小数部分 (2位)

  		// 🚨 核心看这里：用 %d.%02d 代替 %.2f ！！！绝对不会死机！
  		sprintf((char*)cmd_buffer, "t8.txt=\"%d.%02d\"\xff\xff\xfft9.txt=\"%d.%02d\"\xff\xff\xff",
				  cur_int, cur_dec, vol_int, vol_dec);
  	}
  	else
  	{
  		sprintf((char*)cmd_buffer, "t8.txt=\"0.00\"\xff\xff\xfft9.txt=\"0.00\"\xff\xff\xff");
  	}
  	HAL_UART_Transmit(&huart2, cmd_buffer, strlen((char*)cmd_buffer), 100);
  	// ============================================================
  	// 4. 蓝牙发送 (同样使用替身 safe_current)
  	// ============================================================
  	if (system_enable == 1)
  	{
  		Bluetooth_Send_Current(&safe_current);
  	}
  	// 4. 硬件状态同步逻辑
  	uint8_t current_mode = Gear_GetSelection_HAL();
  	if (current_mode != last_mode) {
  		HMI_Sync_Hardware_Mode(current_mode);
  		last_mode = current_mode;
  	}

  	// ============================================================
  	// 【核心灵魂代码】完美的 RTOS 延时！
  	// 彻底释放 CPU！在这 200 毫秒内，这个任务相当于“死”了，
  	// 把 100% 的算力全让给高频的电流保护和 PWM 控制任务！
  	// ============================================================
  	// 【新增】：UI任务汇报存活！
  	osEventFlagsSet(EventGroup_WDGHandle, WDG_BIT_UI);
  	// 【删除旧代码】：osDelay(200);
  	// 【换成新代码】：等待线程标志位 0x01，超时时间为 200 毫秒
  	// 如果没有过流报警，它就像 osDelay(200) 一样睡满 200ms 后自动醒来，做常规屏幕刷新。
  	// 如果电流任务发来了 0x01 标志，它瞬间被唤醒，0 延迟刷新屏幕！
  	osThreadFlagsWait(0x01, osFlagsWaitAny, 200);
  }
  /* USER CODE END StartUICOmmTask */
}

/* USER CODE BEGIN Header_StartCurrentTask */
/**
* @brief Function implementing the Task_Current thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCurrentTask */
void StartCurrentTask(void *argument)
{
  /* USER CODE BEGIN StartCurrentTask */
	// 1. 在这里启动硬件，此时 RTOS 已经完全就绪，不怕中断了！
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, TOTAL_BUFFER);
	HAL_TIM_Base_Start(&htim3);

	// 2. 延时等待稳定 (注意：在 RTOS 里必须用 osDelay，不能用 HAL_Delay)
	osDelay(1000);

	// 3. 进行零点校准
	ACS712_Calibrate(&myCurrentSensor, adc_buffer, TOTAL_BUFFER);

  /* Infinite loop */
  for(;;)
  {
      // ============================================================
	  // 第一部分：高速电流处理 (双缓冲 Ping-Pong Buffer)
  	 // 只有DMA发出信号后，CPU等待数据，再往下走，占用率0%
	  // ============================================================
	  //      --- 处理上半段 (0 - 199) ---
	        if (osSemaphoreAcquire(Sem_ADC_HalfHandle, osWaitForever) == osOK)
	        {

#if 0
	        	uint64_t sum_squares = 0;
	            int32_t zero_offset = myCurrentSensor.adc_midpoint;

	            for (int i = 0; i < BLOCK_SIZE; i++)
	            {
	                int32_t val = (int32_t)adc_buffer[i] - zero_offset;
	                sum_squares += (val * val);
	            }

	            float rms_bits = sqrtf((float)sum_squares / BLOCK_SIZE);
	            // 计算真实电流
	            float current_rms = (rms_bits * 3.3f / 4096.0f) / myCurrentSensor.sensitivity;
#endif

	        	// 🚨 直接调用极速版函数！传入 adc_buffer 的首地址，长度 200 (即 BLOCK_SIZE)
	        	float current_rms = ACS712_ReadRMS(&myCurrentSensor, adc_buffer, BLOCK_SIZE);
	        	// 1. 【核心】先算出最新值存到局部变量，绝对不受互斥锁的干扰！
	        	float latest_filtered_current = CurrentFilter_Process(&myFilter, current_rms);

	        	// 2. 【最高优先级保护】用最新的局部变量做判断，过流立刻急停，零延迟！
	        	if (latest_filtered_current > 10.0f)
	        	{
	        		PWM_Stop_HAL();
	        		system_enable = 0;
	        		// 【新增】：紧急唤醒 UI 任务！
	        		// 假设用第 0 位 (0x01) 代表“紧急刷新”标志
	        		osThreadFlagsSet(Task_UI_CommHandle, 0x01);
	        	}

	        	// 3. 【更新UI显示】只锁这 1 句赋值代码，瞬间交接给屏幕，不耽误下一次循环
	        	if (osMutexAcquire(Mutex_CurrentHandle, 10) == osOK)
	        	{
	        		current_display = latest_filtered_current;
	        		osMutexRelease(Mutex_CurrentHandle);
	        	}
	        }
	        // --- 处理下半段 (200 - 399) ---
  	        // 处理完上半段之后，DMA 会继续往下半段写数据，直到满了才发信号，所以这里也是完全同步的，不会有数据竞争问题
	        if (osSemaphoreAcquire(Sem_ADC_FullHandle, osWaitForever) == osOK)
	        {
#if 0
	        	uint64_t sum_squares = 0;
	            int32_t zero_offset = myCurrentSensor.adc_midpoint;

	            for (int i = 200; i < 400; i++)
	            {
	                int32_t val = (int32_t)adc_buffer[i] - zero_offset;
	                sum_s quares += (val * val);
	            }

	            float rms_bits = sqrtf((float)sum_squares / BLOCK_SIZE);
	            float current_rms = (rms_bits * 3.3f / 4096.0f) / myCurrentSensor.sensitivity;
#endif
	        	// 🚨 注意看这里！传入的是 &adc_buffer[BLOCK_SIZE]，也就是从第 200 个数据开始算！
	        	float current_rms = ACS712_ReadRMS(&myCurrentSensor, &adc_buffer[BLOCK_SIZE], BLOCK_SIZE);
	        	// 1. 局部变量存最新值
	        	float latest_filtered_current = CurrentFilter_Process(&myFilter, current_rms);

	        	// 2. 过流立刻急停
	        	if (latest_filtered_current > 10.0f)
	        	{
	        		PWM_Stop_HAL();
	        		system_enable = 0;
	        		// 【新增】：紧急唤醒 UI 任务！
	        		// 假设用第 0 位 (0x01) 代表“紧急刷新”标志
	        		osThreadFlagsSet(Task_UI_CommHandle, 0x01);
	        	}

	        	// 3. 给 UI 全局变量赋值（上锁）
	        	if (osMutexAcquire(Mutex_CurrentHandle, 10) == osOK)
	        	{
	        		current_display = latest_filtered_current;
	        		osMutexRelease(Mutex_CurrentHandle);
	        	}
	        	// 【新增】：电流任务汇报存活！
	        	osEventFlagsSet(EventGroup_WDGHandle, WDG_BIT_CURRENT);
	        }
  }
  /* USER CODE END StartCurrentTask */
}

/* USER CODE BEGIN Header_StartControlTask */
/**
* @brief Function implementing the Task_Control thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartControlTask */
void StartControlTask(void *argument)
{
  /* USER CODE BEGIN StartControlTask */
	uint8_t last_pedal_state = 0;
	uint32_t received_pulse_counts = 0; // 用于存放从队列接出来的脉冲数量

  /* Infinite loop */
  for(;;)
  {
  	// ============================================================
  	// 1. 【事件驱动核心】阻塞等待串口发来的脉冲指令，超时时间 20ms
  	// ============================================================
  	osStatus_t status = osMessageQueueGet(PulseQueueHandle, &received_pulse_counts, NULL, 20);

  	// 如果拿到了数据，说明串口来指令了！立刻执行，0延迟！
  	if (status == osOK)
  	{
  		// 只有在系统开启且没踩脚踏时，才允许屏幕触发定脉冲
  		if (system_enable == 1 && FootPedal_IsPressed_HAL() == 0)
  		{
  			uint8_t current_mode = Gear_GetSelection_HAL();
  			PulseControl_Start_HAL(received_pulse_counts, current_mode);
  		}
  	}

  	// ============================================================
  	// 2. 原有的脚踏控制逻辑 (保持不变)
  	// ============================================================
  	// 只有在 (系统开机) 且 (没有跑自动脉冲) 时，允许脚踏控制
  	if (system_enable == 1 && PulseControl_IsActive() == 0)
  	{
  		uint8_t current_pedal = FootPedal_IsPressed_HAL();
  		if (current_pedal != last_pedal_state)
  		{
  			osDelay(10); // 消抖
  			current_pedal = FootPedal_IsPressed_HAL();
  			if (current_pedal != last_pedal_state)
  			{
  				if (current_pedal == 1) {
  					uint8_t mode = Gear_GetSelection_HAL();
  					PWM_Start_HAL(mode);
  				} else {
  					PWM_Stop_HAL();
  				}
  				last_pedal_state = current_pedal;
  			}
  		}
  	}
  	else
  	{
  		last_pedal_state = FootPedal_IsPressed_HAL(); // 同步状态，防止带载启动
  	}

  	// ============================================================
  	// 3. 任务汇报存活！
  	// ============================================================
  	osEventFlagsSet(EventGroup_WDGHandle, WDG_BIT_CONTROL);

  }
  /* USER CODE END StartControlTask */
}

/* USER CODE BEGIN Header_StartWatchdogTask */
/**
* @brief Function implementing the Task_Watchdog thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWatchdogTask */
void StartWatchdogTask(void *argument)
{
  /* USER CODE BEGIN StartWatchdogTask */
	// 开机先擦一次黑板，确保初始状态干净
	osEventFlagsClear(EventGroup_WDGHandle, WDG_BITS_ALL);
  /* Infinite loop */
  for(;;)
  {

  	// 查岗大爷在黑板前死等，直到三个勾 (WDG_BITS_ALL) 都被画上。
  	// 如果等了 500ms 还没凑齐，说明有任务卡死了，大爷就不等了，直接引发后续的看门狗复位重启！
  	uint32_t flags = osEventFlagsWait(EventGroup_WDGHandle,
										WDG_BITS_ALL,
										osFlagsWaitAll | osFlagsNoClear, // 等待所有标志，并且拿到后先不自动清除
										500); // 500ms 超时时间（根据你系统最慢任务的周期调整）

  	if (flags == WDG_BITS_ALL)
  	{
  		// 恭喜！三个任务都活着！
  		// 1. 喂真狗，保平安
  		HAL_IWDG_Refresh(&hiwdg);

  		// 2. 擦黑板，准备下一轮查岗
  		osEventFlagsClear(EventGroup_WDGHandle, WDG_BITS_ALL);
  	}
  	else
  	{
  		// 发生严重故障！有任务没打卡（超时了）！
  		// 这里什么也不做，不喂狗。
  		// 几百毫秒后，硬件看门狗 IWDG 就会因为饥饿而咬下复位键，系统重启。

  		// (可选：如果你有 LED，可以在这里点亮一个红灯表示系统即将复位)
  	}

  	osDelay(100); // 每 100ms 喂一次，稳如泰山
  }
  /* USER CODE END StartWatchdogTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

