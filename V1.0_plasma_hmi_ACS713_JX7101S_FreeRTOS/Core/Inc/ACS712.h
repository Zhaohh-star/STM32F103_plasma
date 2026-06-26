#ifndef INC_ACS712_H_
#define INC_ACS712_H_

#include <stdint.h> // 为了使用 uint16_t 等类型

// 定义 ACS712 对象结构体
typedef struct {
    float sensitivity;      // 灵敏度 (V/A)，例如 5A量程为 0.185，20A量程是 0.100V/A
    float zero_voltage;     // 零点电压 (V)，理论值是 2.50
    uint16_t adc_midpoint;  // 零点对应的 ADC 原始值 (0-4095)
    float v_ref;            // 系统参考电压，通常是 3.3V ADC的参考电压
} ACS712_t;                 // 结构体的类型别名

// === 函数声明 ===

/**
 * @brief  初始化传感器参数
 * @param  sensor: 结构体指针
 * @param  sensitivity: 灵敏度 (0.185 for 5A, 0.100 for 20A, 0.066 for 30A)
 * @param  v_ref: 系统参考电压 (通常 3.3)
 */
void ACS712_Init(ACS712_t *sensor, float sensitivity, float v_ref);

/**
 * @brief  执行零点校准 (需要在无电流流过时调用)
 * @param  sensor: 结构体指针
 * @param  buffer: DMA 采集到的 ADC 数据数组
 * @param  length: 数组长度
 */
void ACS712_Calibrate(ACS712_t *sensor, uint16_t *buffer, uint16_t length);

/**
 * @brief  计算 RMS 电流值
 * @param  sensor: 结构体指针
 * @param  buffer: DMA 采集到的 ADC 数据数组
 * @param  length: 数组长度
 * @return float: 计算出的电流有效值 (Amps)
 */
float ACS712_ReadRMS(ACS712_t *sensor, uint16_t *buffer, uint16_t length);

#endif /* INC_ACS712_H_ */
