#ifndef INC_JX7101S_H_
#define INC_JX7101S_H_

#include "main.h" // 需要包含 main.h 以获取 HAL 库定义

// --- 宏定义 ---
#define SENSOR_ADDR     0x01
#define READ_CMD        0x03
#define START_REG       0x1000
#define REG_COUNT       0x0016

// --- 函数声明 ---

/**
 * @brief 读取传感器电压值
 * @param huart: 连接传感器的串口句柄 (例如 &huart3)
 * @return float: 返回解析出的电压值。如果通信失败返回 -1.0f
 */
float JX7101S_ReadVoltage(UART_HandleTypeDef *huart);

/**
 * @brief 发送校准指令 (清零电能等)
 * @param huart: 连接传感器的串口句柄
 */
void JX7101S_SendCalibration(UART_HandleTypeDef *huart);

#endif /* INC_JX7101S_H_ */
