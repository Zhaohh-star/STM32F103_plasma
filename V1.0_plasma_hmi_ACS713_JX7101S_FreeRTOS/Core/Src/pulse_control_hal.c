#include "pulse_control_hal.h"
#include "pwm_hal.h" // 需要调用PWM底层函数
#include "tim.h" // 引用 htim1

// 从 main.c 中引入由CubeMX生成的TIM1句柄
extern TIM_HandleTypeDef htim1;

// --- 模块内部状态变量 ---
// 使用 volatile 关键字是因为这些变量会在主循环和中断服务程序之间共享
static volatile uint8_t  pulse_mode_active = 0;     //0：停止；1：开始
static volatile uint32_t pulse_count = 0;
static volatile uint32_t pulse_target = 0;


/**
 * @brief  以指定的目标脉冲数，开始一次定脉冲输出.
 */
void PulseControl_Start_HAL(uint32_t target_pulses, uint8_t gear)
{
	// 1. 安全检查：如果已经在发脉冲，或者处于连续运行模式(A指令)，则拒绝
	if (pulse_mode_active || target_pulses == 0)
    {
        return; // 如果目标为0或已经在运行，则忽略
    }

	// 2. 初始化状态
	    pulse_target = target_pulses;
	    pulse_count = 0;
	    pulse_mode_active = 1;

	// 3. 复位定时器计数器 (让第一个波形完整)
	    __HAL_TIM_SET_COUNTER(&htim1, 0);

	// 4. 清除中断标志位 (防止开启瞬间误触发一次中断)
	    __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);

	// 5. 启动 PWM 硬件 (根据挡位 gear 选择单/双极)
	    PWM_Start_HAL(gear);

	// 6. 开启定时器更新中断 (开始计数)
	    HAL_TIM_Base_Start_IT(&htim1);
}

/**
 * @brief  强制停止当前的定脉冲输出.
 */
void PulseControl_Stop_HAL(void)
{
    if (pulse_mode_active == 0)
    {
        return; // 如果不在运行，则无需停止
    }

    // 1. 关闭中断 (停止计数)
        HAL_TIM_Base_Stop_IT(&htim1);

        // 2. 停止 PWM 硬件
        PWM_Stop_HAL();

        // 3. 复位状态
        pulse_mode_active = 0;
}

/**
 * @brief  查询当前是否正处于定脉冲输出模式.
 */
uint8_t PulseControl_IsActive(void)
{
    return pulse_mode_active;
}


/**
  * @brief  TIM1更新中断的处理函数.每发一个波，进一次这里
  * @note   此函数由 HAL_TIM_PeriodElapsedCallback 统一调用.
  * 它处理脉冲计数和自动停止的逻辑.
  */
void PulseControl_TIM_IRQ_Handler(void) // <-- 修改函数名，并移除 htim 参数
{
    // 不再需要 if (htim->Instance == TIM1)，因为调用者已经保证了这一点
    if (pulse_mode_active)
    {
        pulse_count++; // 脉冲计数 +1

        if (pulse_count >= pulse_target)
        {
            // 已达到目标脉冲数，自动停止
            PulseControl_Stop_HAL();
        }
    }
}
