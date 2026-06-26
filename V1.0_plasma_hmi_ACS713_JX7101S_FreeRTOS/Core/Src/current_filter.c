#include "current_filter.h"
#include"math.h"
// 初始化函数
void CurrentFilter_Init(CurrentFilter_t *filter, float alpha, float dead_zone)
{
    filter->alpha = alpha;
    filter->dead_zone = dead_zone;
    filter->filtered_value = 0.0f; // 初始状态清零
}

// 核心处理函数
// 死区+一阶滤波：处理霍尔传感器最经典高效的组合
float CurrentFilter_Process(CurrentFilter_t *filter, float raw_value)
{
    float input = raw_value;

    // 1. 死区处理 (Dead Zone)
    // 如果输入值小于死区阈值，直接视为 0，消-除底噪干扰
    if (fabs(input) < filter->dead_zone)
    {
        input = 0.0f;
    }

    // 2. 一阶滞后滤波 (First-Order Lag Filter)
    // 公式：Out = α * In + (1 - α) * Last_Out
    // 这是一个不需要数组的“滑动平均”，非常节省内存
    filter->filtered_value = (filter->alpha * input) +
                             ((1.0f - filter->alpha) * filter->filtered_value);

    return filter->filtered_value;
}

// 重置函数
void CurrentFilter_Reset(CurrentFilter_t *filter)
{
    filter->filtered_value = 0.0f;
}
