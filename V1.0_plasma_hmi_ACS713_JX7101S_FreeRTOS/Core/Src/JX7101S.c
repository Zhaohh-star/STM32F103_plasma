/*
 * JX7101S.c
 * 电压传感器驱动实现文件
 */
#include "JX7101S.h"
#include <stdio.h>

// --- 私有变量 ---
// 发送缓冲区
static uint8_t mRequest[8] = {SENSOR_ADDR, READ_CMD, (START_REG>>8), (START_REG&0xFF), (REG_COUNT>>8), (REG_COUNT&0xFF), 0, 0};
static uint8_t rxData[256] = {0};

// --- 私有函数声明 (内部使用，不给外部看) ---
static uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t len);
static int32_t BytesToInt32(uint8_t *data, uint16_t index);

/**
 * @brief 读取传感器电压值 (核心函数)
 */
float JX7101S_ReadVoltage(UART_HandleTypeDef *huart)
{
    // 1. 动态计算 CRC (防止 mRequest 被意外修改，或者首次运行初始化)
    uint16_t crc = Modbus_CRC16(mRequest, 6);
    mRequest[6] = crc & 0xFF;
    mRequest[7] = (crc >> 8) & 0xFF;

    // 2. 发送请求
    HAL_UART_Transmit(huart, mRequest, 8, 100);

    // 3. 接收数据
    uint16_t len = 0;
    HAL_StatusTypeDef result = HAL_UARTEx_ReceiveToIdle(huart, rxData, 100, &len, 200);

    // 4. 解析数据
    if(result == HAL_OK && len >= 5)
    {
        uint16_t crc_calc = Modbus_CRC16(rxData, len - 2);
        uint16_t crc_recv = (uint16_t)rxData[len - 2] | ((uint16_t)rxData[len - 1] << 8);

        if(crc_calc != crc_recv)
        {
            return -1.0f;
        }

        // 校验地址和功能码
        if(rxData[0] == SENSOR_ADDR && rxData[1] == READ_CMD && rxData[2] == (REG_COUNT * 2))
        {
            // 数据解析：电压在第 3 个字节开始
            float v = BytesToInt32(rxData, 3) / 100.0f;    //idx += 4;
#if 0
	        float i_val = BytesToInt32(rxData, idx) / 1000.0f;  idx += 4;
	        float p = BytesToInt32(rxData, idx) / 10000.0f;     idx += 4;
	        float fp = BytesToInt32(rxData, idx) / 10000.0f;    idx += 4;
	        float sp = BytesToInt32(rxData, idx) / 10000.0f;    idx += 4;
	        float pf = BytesToInt32(rxData, idx) / 1000.0f;     idx += 4;
	        float f = BytesToInt32(rxData, idx) / 100.0f;       idx += 4;
#endif

            // 软件死区处理 (滤除感应电)
            if(v < 0.2f) v = 0.0f;

            return v; // 返回电压值
        }

        if(rxData[0] == SENSOR_ADDR && rxData[1] == (READ_CMD | 0x80))
        {
            return -1.0f;
        }
    }

    return -1.0f; // 返回负数表示通信错误
}

/**
 * @brief 发送校准指令
 */
void JX7101S_SendCalibration(UART_HandleTypeDef *huart)
{
    uint8_t calRequest[] = {0x01, 0x06, 0xA0, 0x00, 0x00, 0x02, 0x2A, 0x0B};
    uint16_t len = 0;

    HAL_UART_Transmit(huart, calRequest, sizeof(calRequest), 100);
    // 接收回执，但不做处理，只是为了清空缓冲区
    HAL_UARTEx_ReceiveToIdle(huart, rxData, 60, &len, 500);
}

// --- 内部辅助函数实现 ---

static uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for(int i = 0; i < len; i++) {
        crc ^= buffer[i];
        for(int j = 0; j < 8; j++) {
            if(crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

static int32_t BytesToInt32(uint8_t *data, uint16_t index)
{
    return (int32_t)( ((uint32_t)data[index]   << 24) |
                      ((uint32_t)data[index+1] << 16) |
                      ((uint32_t)data[index+2] << 8)  |
                      ((uint32_t)data[index+3])       );
}

