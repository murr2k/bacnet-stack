/**
 * @file
 * @brief RS-485 Interface header for PIC32MX795F512L
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#ifndef RS485_H
#define RS485_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
void RS485_Initialize(void);
void RS485_Set_Baud_Rate(uint32_t baud);
bool RS485_Transmit_Ready(void);
void RS485_Send_Frame(uint8_t *buffer, uint16_t nbytes);
bool RS485_DataAvailable(void);
bool RS485_Receive(uint8_t *data_byte, uint32_t *timestamp);
bool RS485_ReceiveError(void);
void RS485_Get_Statistics(uint32_t *p_rx_bytes, uint32_t *p_tx_bytes);
void RS485_Clear_Statistics(void);

#ifdef __cplusplus
}
#endif

#endif /* RS485_H */
