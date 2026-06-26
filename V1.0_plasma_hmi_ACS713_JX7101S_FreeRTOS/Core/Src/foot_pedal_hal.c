#if 0
#include "foot_pedal_hal.h"

/**
  * @brief  检查脚踏开关 (PA4) 是否被踩下.
  * @note   硬件已在CubeMX中配置为上拉输入.
  * @retval 1: 已踩下 (引脚为低电平)
  * @retval 0: 未踩下 (引脚为高电平)
  */
uint8_t FootPedal_IsPressed_HAL(void)
{
  // PA4 配置为上拉输入，当脚踏踩下接地时，引脚读到低电平 (RESET)
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
  {
    return 1; // 已踩下
  }
  else
  {
    return 0; // 未踩下
  }
}
# endif

#include "foot_pedal_hal.h"

/**
  * @brief  检查脚踏开关 (PA4) 是否被踩下.
  * @note   硬件已在CubeMX中配置为上拉输入.
  * @retval 1: 已踩下 (引脚为低电平)
  * @retval 0: 未踩下 (引脚为高电平)
  */
uint8_t FootPedal_IsPressed_HAL(void)
{
    // 第一次采样
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != GPIO_PIN_RESET)
    {
        return 0; // 没踩
    }
    HAL_Delay(1);// 延时 1ms (避开瞬间干扰)

    // 第二次采样
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != GPIO_PIN_RESET)
    {
        return 0; // 可能是干扰信号，判定为没踩
    }
    HAL_Delay(1); // 延时 1ms

    // 第三次采样 (最终确认)
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
    {
        return 1; // 连续3次检测到低电平，认定确实踩下了
    }

    return 0;
}

