#include "Data_Solve.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"



uint16_t card_type;
uint32_t card_id;

void calculate_checksum(uint8_t *buf, uint8_t length)
{
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length - 2; i++) // 从第一个字节到倒数第二个字节（不包括校验值字节）
    {
        checksum ^= buf[i];
    }
    buf[length - 2] = ~checksum; // 取反后存入校验值位置
}

bool verify_checksum(uint8_t *buf, uint8_t length)
{
    uint8_t received_checksum = buf[length - 2];
    uint8_t calculated_checksum = 0;
    for (uint8_t i = 0; i < length - 2; i++)
    {
        calculated_checksum ^= buf[i];
    }
    calculated_checksum = ~calculated_checksum;
    return (received_checksum == calculated_checksum);
}
void handle_card_leave(void)
{
  // 卡离开事件处理逻辑
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,GPIO_PIN_SET);  // 例如点亮LED指示
}
void handle_card_reach(void)
{
  // 卡到达事件处理逻辑
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,GPIO_PIN_RESET);  // 例如点亮LED指示
}
void process_receive_data(uint8_t *buffer)
{
  // 这里根据协议解析数据，例如检测卡号或卡离开事件
  // 示例：检测卡号离开事件
  if (buffer[0] == 0x01 && buffer[2] == 0xA6 )
  {
    // 卡离开事件处理
    handle_card_leave();
  }
  else if (buffer[0] == 0x04 && buffer[2] == 0x02)
  {
    // 卡号读取事件处理
		card_type=(buffer[5] << 8) | buffer[6];
    card_id = (buffer[7] << 24) | (buffer[8] << 16) | (buffer[9] << 8) | buffer[10];
		if((card_id==0x341105E3||card_id==0x540E07E3||card_id==0xC4FE03E3||card_id==0x24D189E2||card_id==0x44C1EBE9||card_id==0x449D56EB||card_id==0xB4C80FE3))
		handle_card_reach();	
  }
	else
	{
    handle_card_leave();	
	}
}
// 串口接收缓冲区
// 接收缓冲区
uint8_t rx_buffer[20];  // 假设最大接收256字节
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // 收到1字节数据后，继续接收下一字节
    HAL_UART_Receive_IT(&huart1, rx_buffer, 13);

      if ((rx_buffer[1] == 0x0d) && (rx_buffer[12] == 0x55&&verify_checksum(rx_buffer,rx_buffer[1])))
      {
        // 检测到帧结束，处理数据
        process_receive_data(rx_buffer);
				memset(rx_buffer, 0, sizeof(rx_buffer)); // 将数组全部元素初始化为 0

      }
   }
  
}


