#include "ACS712.h"
#include <math.h>  // 必须包含，用于 sqrt 函数

// 初始化函数
void ACS712_Init(ACS712_t *sensor, float sensitivity, float v_ref)
{
    sensor->sensitivity = sensitivity;   // 箭头操作符 通过指针，去访问它指向的那个结构体里的成员 顺着指针指引的方向，去操作里面的成员
    sensor->v_ref = v_ref;

    // 给一些默认值，防止没校准前计算出错
    sensor->adc_midpoint = 2048; // 12bit ADC 的一半
    sensor->zero_voltage = v_ref / 2.0f;
}

// 零点校准函数
void ACS712_Calibrate(ACS712_t *sensor, uint16_t *buffer, uint16_t length)
{
    uint32_t sum = 0;

    // 累加求平均
    for(int i = 0; i < length; i++)
    {
        sum += buffer[i];
    }

    sensor->adc_midpoint = sum / length;

    // 反推零点电压 (仅作记录，计算RMS其实主要用 midpoint) 定锚
    sensor->zero_voltage = (float)sensor->adc_midpoint * (sensor->v_ref / 4096.0f);
}

#if 0
// RMS 计算函数   均方根：代表交流信号的真实有效值
float ACS712_ReadRMS(ACS712_t *sensor, uint16_t *buffer, uint16_t length)
{
    float sum_squares = 0.0f;
    float current_inst = 0.0f;
    float voltage_inst = 0.0f;
    int16_t adc_diff = 0;

    for(int i = 0; i < length; i++)
    {
        // 1. 计算与零点的偏差值 (ADC层面的差值)
        adc_diff = buffer[i] - sensor->adc_midpoint;

        // 2. 将 ADC 差值转为 电压差值
        voltage_inst = adc_diff * (sensor->v_ref / 4096.0f);

        // 3. 将 电压差值 转为 电流值
        current_inst = voltage_inst / sensor->sensitivity;

        // 4. 累加平方
        sum_squares += current_inst * current_inst;
    }

    // 5. 均方根
    return sqrt(sum_squares / (float)length);
}
#endif

// 1. 把我之前给你的“极速整数开方”神仙函数贴在这个文件前面
uint32_t Fast_Integer_Sqrt(uint32_t x)
{
    uint32_t res = 0;
    uint32_t bit = 1 << 30; // 假设 x 是 32 位无符号整数

    while (bit > x) bit >>= 2;
    while (bit != 0) {
        if (x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}

// 2. 这是为你优化后的 RMS 计算函数（极速版）
float ACS712_ReadRMS(ACS712_t *sensor, uint16_t *buffer, uint16_t length)
{
    uint64_t sum_squares = 0; // 必须用 64 位存，防止 200 个平方加起来溢出
    int32_t adc_diff = 0;

    // 第一步：核心循环，纯整数运算！F103 处理它只需要几微秒！
    for(int i = 0; i < length; i++)
    {
        adc_diff = (int32_t)buffer[i] - sensor->adc_midpoint;

        // 🚨【新增】：ADC 层面的数字死区
        // 假设正常底噪在 ±120 个 AD                                C 刻度以内跳动，直接把它们抹平为 0
        if(adc_diff > -120 && adc_diff < 120)
        {
            adc_diff = 0;
        }

        sum_squares += (adc_diff * adc_diff); // 整数乘法，极快！
    }

    // 第二步：求平均的整数平方值
    uint32_t mean_square = (uint32_t)(sum_squares / length);

    // 第三步：使用极速整数开方，得到 ADC 差值的 RMS
    uint32_t rms_adc = Fast_Integer_Sqrt(mean_square);

    // 第四步：全程只做这唯一的一次浮点运算，计算真实电流！
    float voltage_rms = (float)rms_adc * (sensor->v_ref / 4096.0f);
    float current_rms = voltage_rms / sensor->sensitivity;

    return current_rms;

}