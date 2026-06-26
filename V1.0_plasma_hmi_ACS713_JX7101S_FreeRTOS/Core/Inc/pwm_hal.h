#ifndef INC_PWM_HAL_H_
#define INC_PWM_HAL_H_


#include "main.h"

/**
 * @brief  启动两组互补PWM输出.
 * @param   1: 单极模式(仅CH1/CH1N) 2.
 */
void PWM_Start_HAL(uint8_t gear);

/**
 * @brief  安全地停止所有PWM输出.
 * @note   此函数会禁用PWM主输出，停止所有通道，并将引脚配置为推挽输出并拉低.
 */void PWM_Stop_HAL(void);

/**
 * @brief  动态设置PWM的频率.
 * @param  freq 目标频率 (Hz), 例如 50000.0f.
 */
void PWM_SetFreq_HAL(float freq);

/**
 * @brief  动态设置PWM的占空比.
 * @param  duty 目标占空比 (0.0f to 1.0f), 例如 0.35f.
 * @note   此函数会同时更新CH1和CH2的占空比.
 */
void PWM_SetDuty_HAL(float duty);

#endif /* INC_PWM_HAL_H_ */


