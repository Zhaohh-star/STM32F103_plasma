#ifndef INC_PULSE_CONTROL_HAL_H_
#define INC_PULSE_CONTROL_HAL_H_


#include "main.h"

/**
 * @brief  以指定的目标脉冲数，开始一次定脉冲输出.
 * @param  target_pulses 要输出的PWM脉冲总数.
 * @param  gear 挡位. 1: 单极模式, 其他值: 双极模式.
 */
void PulseControl_Start_HAL(uint32_t target_pulses, uint8_t gear);

/**
 * @brief  强制停止当前的定脉冲输出.
 */
void PulseControl_Stop_HAL(void);

/**
 * @brief  查询当前是否正处于定脉冲输出模式.
 * @retval 1: 正在输出
 * @retval 0: 已停止
 */
uint8_t PulseControl_IsActive(void);

/**
 * @brief  TIM1更新中断的处理函数.
 * @note   此函数应在主中断回调(HAL_TIM_PeriodElapsedCallback)中被调用.
 */
void PulseControl_TIM_IRQ_Handler(void); // <-- 新增这一行

#endif /* INC_PULSE_CONTROL_HAL_H_ */
