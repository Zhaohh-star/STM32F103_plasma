#ifndef INC_FOOT_PEDAL_HAL_H_
#define INC_FOOT_PEDAL_HAL_H_

#include "main.h"

/**
  * @brief  检查脚踏开关是否被踩下.
  * @retval 1: 已踩下
  * @retval 0: 未踩下
  */
uint8_t FootPedal_IsPressed_HAL(void);



#endif /* INC_FOOT_PEDAL_HAL_H_ */
