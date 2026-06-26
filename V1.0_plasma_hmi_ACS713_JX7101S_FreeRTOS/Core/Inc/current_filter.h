/*
 * current_filter.h
 *
 * Created on: Jan 4, 2026
 * Description: 通用的电流滤波算法模块 (死区 + 一阶滞后滤波)
 */

#ifndef INC_CURRENT_FILTER_H_
#define INC_CURRENT_FILTER_H_

#include <stdint.h>

// 定义滤波对象结构体
typedef struct {
    float alpha;           // 滤波系数 (0.0 ~ 1.0)，越小越平滑，响应越慢，目前使用的0.1
    float dead_zone;       // 死区阈值，小于此值的输入强制归零
    float filtered_value;  // 保存上一次的滤波结果 (状态记忆)
} CurrentFilter_t;         // 结构体的类型别名

// === 函数声明 ===

/**
 * @brief  初始化滤波器
 * @param  filter: 滤波器对象指针
 * @param  alpha: 滤波系数 (推荐 0.1 ~ 0.2)
 * @param  dead_zone: 死区阈值 (例如 0.15A)
 */
void CurrentFilter_Init(CurrentFilter_t *filter, float alpha, float dead_zone);

/**
 * @brief  执行滤波处理
 * @param  filter: 滤波器对象指针
 * @param  raw_value: 当前采样的原始值
 * @return float: 滤波后的平滑值
 */
float CurrentFilter_Process(CurrentFilter_t *filter, float raw_value);

/**
 * @brief  重置滤波器 (例如系统复位或重新开始测量时用)
 * @param  filter: 滤波器对象指针
 */
void CurrentFilter_Reset(CurrentFilter_t *filter);

#endif /* INC_CURRENT_FILTER_H_ */
