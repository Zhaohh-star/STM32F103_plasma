#include "pwm_hal.h"

// 从 main.c 中引入由CubeMX生成的TIM1句柄
extern TIM_HandleTypeDef htim1;

/**
 * @brief  启动两组互补PWM输出.
 */
void PWM_Start_HAL(uint8_t gear)
{
    // 1. 将所有PWM引脚重新配置为“复用推挽输出”模式，将控制权交给TIM1
    GPIO_InitTypeDef GPIO_InitStruct = {0};       // 创建并清零一个配置蓝图
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;       // 填写申请表上的关键栏目
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 2. 启动PWM通道 互补通道
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    if (gear != 1) // 双极模式 额外开启
    {
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    }
    else // 单极模式下，确保通道2是关闭的
    {
        //HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        //HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
        // --- 模式: 手动输出(OUTPUT_PP), 停止PWM ---

           // 1. 停止TIM1在通道2上的输出
          HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
          HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

          // 2. 重新配置GPIO为普通推挽输出
          GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
          GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

          // 3. 夺取 PA9 控制权, 并强制拉高
          GPIO_InitStruct.Pin = GPIO_PIN_9;
          HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

          // 4. 夺取 PB14 控制权, 并强制拉低
           GPIO_InitStruct.Pin = GPIO_PIN_14;
           HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    }

    // 3. 启用定时器的主输出，信号才会真正出现在引脚上
    __HAL_TIM_MOE_ENABLE(&htim1);
}

/**
 * @brief  安全地停止所有PWM输出.
 */
void PWM_Stop_HAL(void)
{
    // 1. 立即禁用定时器的主输出，这是最快的关断方式
    __HAL_TIM_MOE_DISABLE(&htim1);

    // 2. 停止所有PWM通道
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

    // 3. 将所有PWM引脚的控制权从TIM1收回，配置为普通的“推挽输出”模式
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 4. 将所有引脚强制拉低，确保处于安全状态
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_RESET);
}

/**
 * @brief  动态设置PWM的占空比 (安全版本)
 * @param  duty: 目标占空比, 范围应为 0.0f 到 1.0f
 */
void PWM_SetDuty_HAL(float duty)
{
    // --- 检查 1: 限制占空比在 [0.0f, 1.0f] 范围 ---
    // 防止传入负值或大于100%的值
    if (duty < 0.0f)
    {
        duty = 0.0f;
    }
    if (duty > 1.0f)
    {
        duty = 1.0f;
    }

    // --- 检查 2: 获取当前0的周期 (ARR) ---
    uint32_t arr_value = __HAL_TIM_GET_AUTORELOAD(&htim1);

    // --- 检查 3: 计算 CCR ---
    // (arr_value + 1) 是为了精确计算,
    // 例如 ARR=99, 周期是100个计数。50%占空比应为 100 * 0.5 = 50
    uint32_t ccr_value = (uint32_t)((arr_value + 1) * duty);

    // --- 检查 4: 最终保护 (最关键) ---
    // 确保CCR值 (由于浮点数计算精度) 绝对不会超过ARR值
    // 同时确保值在16位寄存器范围内
    if (ccr_value > arr_value)
    {
        ccr_value = arr_value; // 最大占空比 (100%)
    }
    if (ccr_value > 65535)
    {
        ccr_value = 65535;
    }

    // 同时更新两个通道的CCR值
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_value);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr_value);
}

/**
 * @brief  动态设置PWM的频率 (安全/健壮的版本)
 * @note   此函数会自动保持当前的占空比百分比不变.
 * @param  freq: 目标频率 (Hz)
 */
void PWM_SetFreq_HAL(float freq)
{
    // --- 1. 获取旧的 ARR 和 CCR (用于计算当前占空比) ---
    uint32_t old_arr = __HAL_TIM_GET_AUTORELOAD(&htim1);
    uint32_t old_ccr = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1);

    // 计算当前的实际占空比 (例如 0.25f, 0.4f 等)
    float current_duty_percent = 0.0f;
    if (old_arr > 0)
    {
        current_duty_percent = (float)old_ccr / (float)(old_arr + 1);
    }

    // 确保占空比百分比安全
    if (current_duty_percent < 0.0f) current_duty_percent = 0.0f;
    if (current_duty_percent > 1.0f) current_duty_percent = 1.0f;


    // --- 2. 安全检查和计算新的 ARR ---
    // 您的 main.c 循环已确保 freq 在 [20k, 80k] Hz 之间
    // 我们在此再做一次钳位，确保函数健壮性
    if (freq < 20000.0f) freq = 20000.0f;
    if (freq > 120000.0f) freq = 120000.0f;

    // 假设 TIM1 时钟为 72MHz 且 PSC 预分频器为 0
    uint32_t new_arr = (uint32_t)(72000000.0f / freq) - 1;

    // 强制确保 ARR 值在16位定时器的有效范围内 [1, 65535]
    if (new_arr > 65535) new_arr = 65535;
    if (new_arr == 0)   new_arr = 1;


    // --- 3. 根据新 ARR 和保持的占空比，计算新 CCR ---
    // (例如，如果之前是 25%, 这里会计算 new_arr * 0.25f)
    uint32_t new_ccr = (uint32_t)((new_arr + 1) * current_duty_percent);

    // 再次确保安全
    if (new_ccr > new_arr) new_ccr = new_arr;


    // --- 4. 按正确顺序更新寄存器 (消除竞争) ---
    __HAL_TIM_SET_AUTORELOAD(&htim1, new_arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, new_ccr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, new_ccr);
}




#if 0
/**
 * @brief  测试频率的地方使其可以调整到2HZ，肉眼可以看到频率的变化
 * @note   此函数会自动保持当前的占空比百分比不变
 */
void PWM_SetFreq_HAL(float freq)
{
    // 1. 限制输入范围 (为了测试闪烁，我们把下限放宽到 1Hz)
    if (freq < 1.0f) freq = 1.0f;
    if (freq > 120000.0f) freq = 120000.0f;

    // 获取旧参数用于保持占空比
    uint32_t old_arr = __HAL_TIM_GET_AUTORELOAD(&htim1);
    uint32_t old_ccr = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1);
    float current_duty_percent = 0.0f;
    if (old_arr > 0) current_duty_percent = (float)old_ccr / (float)(old_arr + 1);

    uint32_t new_arr;

    // === 核心修改：动态调整 PSC ===
    if (freq < 1000.0f)
    {
        // [低频模式] 想要看闪烁？必须降速！
        // 设置 PSC = 7199 (即 7200分频)
        // 此时定时器时钟 = 72MHz / 7200 = 10 kHz (一秒钟数 10000 下)
        __HAL_TIM_SET_PRESCALER(&htim1, 7199);

        // 计算 ARR: 时钟是 10000Hz
        new_arr = (uint32_t)(10000.0f / freq) - 1;
    }
    else
    {
        // [高频模式] 正常工作
        // 设置 PSC = 0
        // 时钟 = 72MHz
        __HAL_TIM_SET_PRESCALER(&htim1, 0);

        // 计算 ARR: 时钟是 72000000Hz
        new_arr = (uint32_t)(72000000.0f / freq) - 1;
    }

    // 防止 ARR 越界
    if (new_arr > 65535) new_arr = 65535;
    if (new_arr == 0)   new_arr = 1;

    // 计算新 CCR
    uint32_t new_ccr = (uint32_t)((new_arr + 1) * current_duty_percent);

    // 更新寄存器 (PSC的修改需要产生更新事件才能生效，但这里我们不管那么细，多发几次指令就行)
    __HAL_TIM_SET_AUTORELOAD(&htim1, new_arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, new_ccr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, new_ccr);

    // 强制产生一个更新事件，让 PSC 立即生效 (可能会稍微切断一下波形，但在调试时无所谓)
    HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_UPDATE);
}
#endif

