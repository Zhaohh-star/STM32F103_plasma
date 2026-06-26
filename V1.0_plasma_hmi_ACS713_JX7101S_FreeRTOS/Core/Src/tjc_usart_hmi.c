/**
 * @file    tjc_usart_hmi.c
 * @brief   陶晶驰串口屏驱动文件
 * @note    使用注意事项:
 * 1. 请确保工程中包含 tjc_usart_hmi.h
 * 2. 请在 main.h 或 tjc_usart_hmi.h 中定义 TJC_UART 和 TJC_UART_INS
 * 例如:
 * #define TJC_UART     huart2
 * #define TJC_UART_INS USART2
 * 3. 确保 HAL_UART_Receive_IT 已经在初始化时被调用
 */

#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include <stdio.h>
#include "tjc_usart_hmi.h"

// --- 环形缓冲区结构体定义 ---
typedef struct
{
    uint16_t Head;           // 队头
    uint16_t Tail;           // 队尾
    uint16_t Length;         // 当前数据长度
    uint8_t  Ring_data[RINGBUFFER_LEN]; // 数据缓冲区
} RingBuffer_t;

RingBuffer_t ringBuffer;     // 定义一个 ringBuffer 的变量
uint8_t RxBuffer[1];         // 串口接收临时缓存 (单字节)


/********************************************************
 * 函数名：   intToStr
 * 功能：     将整数转换为字符串
 * 参数：     num: 要转换的整数
 * str: 存储转换结果的字符数组
 * 返回值：   无
 ********************************************************/
void intToStr(int num, char* str)
{
    int i = 0;
    int isNegative = 0;

    // 处理负数
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    // 获取每一位数字 (倒序)
    do {
        str[i++] = (num % 10) + '0';
        num /= 10;
    } while (num);

    // 如果是负数，追加负号
    if (isNegative) {
        str[i++] = '-';
    }

    // 添加字符串结束符
    str[i] = '\0';

    // 反转字符串
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    return;
}

/********************************************************
 * 函数名：   uart_send_char
 * 功能：     串口发送单个字符
 * 参数：     ch: 要发送的字符
 * 返回值：   无
 ********************************************************/
void uart_send_char(char ch)
{
    uint8_t ch2 = (uint8_t)ch;
    // 等待发送完成 (TC Flag)
    while(__HAL_UART_GET_FLAG(&TJC_UART, UART_FLAG_TC) == RESET);
    // 发送字符
    HAL_UART_Transmit_IT(&TJC_UART, &ch2, 1);
    return;
}

/********************************************************
 * 函数名：   uart_send_string
 * 功能：     发送字符串 (内部调用)
 * 参数：     str: 要发送的字符串
 * 返回值：   无
 ********************************************************/
void uart_send_string(char* str)
{
    while(*str != 0 && str != 0)
    {
        uart_send_char(*str++);
    }
    return;
}

/********************************************************
 * 函数名：   tjc_send_string
 * 功能：     发送字符串指令到串口屏 (自动附加结束符 0xff 0xff 0xff)
 * 参数：     str: 指令字符串
 * 示例：     tjc_send_string("page main");
 ********************************************************/
void tjc_send_string(char* str)
{
    while(*str != 0 && str != 0)
    {
        uart_send_char(*str++);
    }
    // 发送帧尾
    uart_send_char(0xff);
    uart_send_char(0xff);
    uart_send_char(0xff);
}

/********************************************************
 * 函数名：   tjc_send_txt
 * 功能：     修改串口屏控件的 txt 属性
 * 参数：     objname: 控件名 (如 "t0")
 * attribute: 属性名 (通常是 "txt")
 * txt: 要显示的文本
 * 示例：     tjc_send_txt("t0", "txt", "Hello"); -> 发送 t0.txt="Hello"
 ********************************************************/
void tjc_send_txt(char* objname, char* attribute, char* txt)
{
    uart_send_string(objname);
    uart_send_char('.');
    uart_send_string(attribute);
    uart_send_string("=\"");
    uart_send_string(txt);
    uart_send_char('\"');
    // 发送帧尾
    uart_send_char(0xff);
    uart_send_char(0xff);
    uart_send_char(0xff);
}

/********************************************************
 * 函数名：   tjc_send_val
 * 功能：     修改串口屏控件的 val 属性 (数值)
 * 参数：     objname: 控件名 (如 "n0")
 * attribute: 属性名 (通常是 "val")
 * val: 数值
 * 示例：     tjc_send_val("n0", "val", 100); -> 发送 n0.val=100
 ********************************************************/
void tjc_send_val(char* objname, char* attribute, int val)
{
    uart_send_string(objname);
    uart_send_char('.');
    uart_send_string(attribute);
    uart_send_char('=');

    char txt[12] = "";
    intToStr(val, txt);
    uart_send_string(txt);

    // 发送帧尾
    uart_send_char(0xff);
    uart_send_char(0xff);
    uart_send_char(0xff);
}

/********************************************************
 * 函数名：   tjc_send_nstring
 * 功能：     发送指定长度的字符串 (透传数据用)
 * 参数：     str: 数据缓冲区
 * str_length: 长度
 ********************************************************/
void tjc_send_nstring(char* str, unsigned char str_length)
{
    for (int var = 0; var < str_length; ++var)
    {
        uart_send_char(*str++);
    }
    uart_send_char(0xff);
    uart_send_char(0xff);
    uart_send_char(0xff);
}


/********************************************************
 * 函数名：   initRingBuffer
 * 功能：     初始化环形缓冲区
 ********************************************************/
void initRingBuffer(void)
{
    ringBuffer.Head = 0;
    ringBuffer.Tail = 0;
    ringBuffer.Length = 0;
}

/********************************************************
 * 函数名：   write1ByteToRingBuffer
 * 功能：     向缓冲区写入一个字节
 * 参数：     data: 要写入的数据
 ********************************************************/
void write1ByteToRingBuffer(uint8_t data)
{
    if(ringBuffer.Length >= RINGBUFFER_LEN) // 缓冲区满
    {
        return;
    }
    ringBuffer.Ring_data[ringBuffer.Tail] = data;
    ringBuffer.Tail = (ringBuffer.Tail + 1) % RINGBUFFER_LEN; // 移动尾指针，防止越界
    ringBuffer.Length++;
}

/********************************************************
 * 函数名：   deleteRingBuffer
 * 功能：     从缓冲区头部删除指定长度的数据
 * 参数：     size: 要删除的字节数
 * 修复：     原代码有严重 bug (循环内 return)，已修复
 ********************************************************/
void deleteRingBuffer(uint16_t size)
{
    if(size >= ringBuffer.Length)
    {
        initRingBuffer(); // 如果删除长度大于现有长度，直接清空
        return;
    }

    for(int i = 0; i < size; i++)
    {
        ringBuffer.Head = (ringBuffer.Head + 1) % RINGBUFFER_LEN;
        ringBuffer.Length--;
    }
    // 注意：原代码这里的 return 写在 for 循环里面了，导致只能删除 1 个字节。
    // 这里已经修正到了循环外面。
    return;
}

/********************************************************
 * 函数名：   read1ByteFromRingBuffer
 * 功能：     读取缓冲区中指定位置的数据 (不删除)
 * 参数：     position: 距离队头的偏移量 (0 代表读取第一个)
 * 返回值：   读取到的字节
 ********************************************************/
uint8_t read1ByteFromRingBuffer(uint16_t position)
{
    uint16_t realPosition = (ringBuffer.Head + position) % RINGBUFFER_LEN;
    return ringBuffer.Ring_data[realPosition];
}

/********************************************************
 * 函数名：   getRingBufferLength
 * 功能：     获取当前缓冲区的数据长度
 ********************************************************/
uint16_t getRingBufferLength()
{
    return ringBuffer.Length;
}

/********************************************************
 * 函数名：   isRingBufferOverflow
 * 功能：     判断缓冲区是否溢出 (未满)
 * 返回值：   0: 已满, 1: 未满 (原代码逻辑如此)
 ********************************************************/
uint8_t isRingBufferOverflow()
{
    return ringBuffer.Length < RINGBUFFER_LEN;
}
