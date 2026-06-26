#include "gear_switch_hal.h"

/**
  * @brief  读取挡位拨片开关的状态 (PC13).
  * @note   硬件已在CubeMX中配置为上拉输入.
  * @retval 1: 挡位1 (未按下 / 高电平)
  * @retval 2: 挡位2 (按下 / 低电平)
  */
#if 0
uint8_t Gear_GetSelection_HAL(void)
{
  // PC13 配置为上拉输入，当开关按下接地时，引脚读到低电平 (RESET)
  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
  {
    return 2;   //双
  }
  else
  {
    return 1;   //单
  }
}
#endif

uint8_t Gear_GetSelection_HAL(void)
{
    int low_count = 0; // 记录读到低电平的次数

    // === 软件滤波核心 ===
    // 连续读取 10 次，每次间隔 1ms
    // 如果真的是人为接地，这 10 次应该都是低电平
    // 如果是干扰噪音，通常是高低乱跳的
    for(int i = 0; i < 10; i++)
    {
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
        {
            low_count++;
        }
        HAL_Delay(1); // 稍微延时，避开瞬间的干扰尖峰
    }

    // === 判定逻辑 ===
    // 只有 10 次里有 8 次以上是低电平，才认为是真的接地了（双极）
    if (low_count >= 8)
    {
        return 2; // 双极 (Bipolar) - 适配 PWM 驱动
    }
    else
    {
        return 1; // 单极 (Monopolar) - 默认/悬空状态
    }
}
