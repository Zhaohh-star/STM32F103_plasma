#ifndef INC_GEAR_SWITCH_HAL_H_
#define INC_GEAR_SWITCH_HAL_H_


#include "main.h"

/**
  * @brief  读取挡位拨片开关的状态.
  * @retval 1: 挡位1 (未按下 / 高电平)
  * @retval 2: 挡位2 (按下 / 低电平)
  */
uint8_t Gear_GetSelection_HAL(void);


#endif /* INC_GEAR_SWITCH_HAL_H_ */
