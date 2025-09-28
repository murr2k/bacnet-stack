/**
 * @file
 * @brief BACnet MS/TP datalink layer for PIC32MX795F512L
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dlmstp.h"
#include "bacnet/datalink/mstp.h"
#include "bacnet/basic/sys/mstimer.h"
#include "bacnet/npdu.h"
#include "bacnet/basic/npdu/h_npdu.h"
#include "rs485.h"

/* MS/TP state machine variables */
static struct mstp_port_struct_t MSTP_Port;

/* Transmit and receive buffers */
static uint8_t Receive_Buffer[MAX_MPDU];
static uint8_t Transmit_Buffer[MAX_MPDU];

/* Handler buffer for NPDU */
uint8_t Handler_Transmit_Buffer[MAX_PDU];

/* Received packet ready flag */
static volatile bool Receive_Packet_Ready = false;
static BACNET_ADDRESS Receive_Address;
static uint16_t Receive_PDU_Len;

/* Transmit packet */
static struct {
    bool ready;
    uint8_t destination;
    uint8_t frame_type;
    uint8_t pdu[MAX_MPDU];
    uint16_t pdu_len;
} Transmit_Packet;

/* MS/TP configuration */
static uint8_t This_Station = 1;
static uint8_t Max_Master = 127;
static uint8_t Max_Info_Frames = 1;

/* Statistics */
static uint32_t Valid_Frame_Count = 0;

/**
 * @brief Initialize MS/TP datalink
 */
bool dlmstp_init(char *ifname)
{
    (void)ifname; /* Not used */

    /* Initialize MS/TP port structure */
    MSTP_Init(&MSTP_Port);

    /* Set port configuration */
    MSTP_Port.This_Station = This_Station;
    MSTP_Port.Nmax_master = Max_Master;
    MSTP_Port.Nmax_info_frames = Max_Info_Frames;
    MSTP_Port.InputBuffer = Receive_Buffer;
    MSTP_Port.InputBufferSize = sizeof(Receive_Buffer);
    MSTP_Port.OutputBuffer = Transmit_Buffer;
    MSTP_Port.OutputBufferSize = sizeof(Transmit_Buffer);
    MSTP_Port.SilenceTimer = 0;
    MSTP_Port.SilenceTimerReset = 0;

    /* Clear transmit packet */
    Transmit_Packet.ready = false;

    /* Initialize RS-485 interface */
    RS485_Initialize();

    /* Initialize timer */
    mstimer_init();

    return true;
}

/**
 * @brief MS/TP datalink task - must be called regularly
 */
void dlmstp_task(void)
{
    /* Run MS/TP state machine - handles both receive and master FSM */
    MSTP_Receive_Frame_FSM(&MSTP_Port);

    /* Handle master node state machine if we're a master */
    if (This_Station <= Max_Master) {
        while (MSTP_Master_Node_FSM(&MSTP_Port)) {
            /* Fast state transitions */
        }
    }
}

/**
 * @brief Send BACnet NPDU via MS/TP
 */
int dlmstp_send_pdu(
    BACNET_ADDRESS *dest,
    BACNET_NPDU_DATA *npdu_data,
    uint8_t *pdu,
    unsigned pdu_len)
{
    uint8_t destination_mac;

    /* Validate parameters */
    if (!pdu || pdu_len == 0 || pdu_len > MAX_MPDU) {
        return -1;
    }

    /* Check if transmit buffer is busy */
    if (Transmit_Packet.ready) {
        return -2; /* Busy */
    }

    /* Get destination MAC address */
    if (dest && dest->mac_len == 1) {
        destination_mac = dest->mac[0];
    } else {
        destination_mac = MSTP_BROADCAST_ADDRESS;
    }

    /* Store PDU for transmission */
    memcpy(Transmit_Packet.pdu, pdu, pdu_len);
    Transmit_Packet.pdu_len = pdu_len;
    Transmit_Packet.destination = destination_mac;

    if (npdu_data && npdu_data->data_expecting_reply) {
        Transmit_Packet.frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
    } else {
        Transmit_Packet.frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
    }

    /* Mark as ready for transmission */
    Transmit_Packet.ready = true;

    return pdu_len;
}

/**
 * @brief Get broadcast address
 */
void dlmstp_get_broadcast_address(BACNET_ADDRESS *dest)
{
    if (dest) {
        dest->mac_len = 1;
        dest->mac[0] = MSTP_BROADCAST_ADDRESS;
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;
    }
}

/**
 * @brief Get my address
 */
void dlmstp_get_my_address(BACNET_ADDRESS *my_address)
{
    if (my_address) {
        my_address->mac_len = 1;
        my_address->mac[0] = This_Station;
        my_address->net = 0;
        my_address->len = 0;
    }
}

/**
 * @brief Set MAC address
 */
void dlmstp_set_mac_address(uint8_t mac_address)
{
    if (mac_address <= 254) {
        This_Station = mac_address;
        if (MSTP_Port.This_Station != mac_address) {
            MSTP_Port.This_Station = mac_address;
            MSTP_Init(&MSTP_Port);
        }
    }
}

/**
 * @brief Get MAC address
 */
uint8_t dlmstp_mac_address(void)
{
    return This_Station;
}

/**
 * @brief Set Max Master
 */
void dlmstp_set_max_master(uint8_t max_master)
{
    if (max_master <= 127) {
        Max_Master = max_master;
        MSTP_Port.Nmax_master = max_master;
    }
}

/**
 * @brief Get Max Master
 */
uint8_t dlmstp_max_master(void)
{
    return Max_Master;
}

/**
 * @brief Set Max Info Frames
 */
void dlmstp_set_max_info_frames(uint8_t max_info_frames)
{
    if (max_info_frames >= 1) {
        Max_Info_Frames = max_info_frames;
        MSTP_Port.Nmax_info_frames = max_info_frames;
    }
}

/**
 * @brief Get Max Info Frames
 */
uint8_t dlmstp_max_info_frames(void)
{
    return Max_Info_Frames;
}

/**
 * @brief Set baud rate
 */
bool dlmstp_set_baud_rate(uint32_t baud)
{
    bool valid = false;

    /* Validate baud rate per PICS */
    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            RS485_Set_Baud_Rate(baud);
            valid = true;
            break;
        default:
            break;
    }

    return valid;
}

/**
 * @brief Get baud rate (default 19200)
 */
uint32_t dlmstp_baud_rate(void)
{
    return 19200; /* Default per PICS */
}

/**
 * @brief Cleanup
 */
void dlmstp_cleanup(void)
{
    /* Nothing to cleanup in embedded system */
}

/**
 * @brief RS485 RX event notification (called from ISR)
 */
void dlmstp_rs485_rx_event(void)
{
    /* Reset silence timer on any received byte */
    MSTP_Port.SilenceTimer = 0;
}

/* ========================================================================= */
/* MS/TP Callback Functions Required by MS/TP State Machine                 */
/* ========================================================================= */

/**
 * @brief Callback from MS/TP to get data to send
 * @param mstp_port - MS/TP port structure
 * @param timeout - timeout in milliseconds (not used)
 * @return Number of bytes to send, 0 if nothing to send
 */
uint16_t MSTP_Get_Send(struct mstp_port_struct_t *mstp_port, unsigned timeout)
{
    uint16_t pdu_len = 0;
    (void)timeout; /* Not used */

    /* Check if we have PDU to send */
    if (Transmit_Packet.ready) {
        /* Create frame in output buffer */
        pdu_len = MSTP_Create_Frame(
            mstp_port->OutputBuffer, mstp_port->OutputBufferSize,
            Transmit_Packet.frame_type, Transmit_Packet.destination,
            mstp_port->This_Station, Transmit_Packet.pdu,
            Transmit_Packet.pdu_len);

        if (pdu_len > 0) {
            /* Clear ready flag on successful frame creation */
            Transmit_Packet.ready = false;
        }
    }

    return pdu_len;
}

/**
 * @brief Callback from MS/TP to check if data available and get it
 * @param mstp_port - MS/TP port structure
 * @return Number of bytes available (0 or 1)
 */
uint16_t MSTP_Get_Receive(struct mstp_port_struct_t *mstp_port)
{
    uint32_t timestamp;

    if (RS485_DataAvailable()) {
        /* Get byte and store in DataRegister */
        if (RS485_Receive(&mstp_port->DataRegister, &timestamp)) {
            mstp_port->DataAvailable = true;
            return 1;
        }
    }

    mstp_port->DataAvailable = false;
    return 0;
}

/**
 * @brief Callback from MS/TP to handle a received frame
 * @param mstp_port - MS/TP port structure
 * @return PDU length if frame was for us, 0 otherwise
 */
uint16_t MSTP_Put_Receive(struct mstp_port_struct_t *mstp_port)
{
    uint16_t pdu_len = 0;

    /* Check if frame is for us */
    if ((mstp_port->DestinationAddress == mstp_port->This_Station) ||
        (mstp_port->DestinationAddress == MSTP_BROADCAST_ADDRESS)) {
        switch (mstp_port->FrameType) {
            case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
            case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                /* Process BACnet NPDU */
                if (mstp_port->DataLength > 0) {
                    /* Store source address */
                    Receive_Address.mac_len = 1;
                    Receive_Address.mac[0] = mstp_port->SourceAddress;
                    Receive_Address.net = 0;
                    Receive_Address.len = 0;

                    /* Store PDU length */
                    Receive_PDU_Len = mstp_port->DataLength;
                    pdu_len = mstp_port->DataLength;

                    /* Handle the NPDU */
                    npdu_handler(
                        &Receive_Address, mstp_port->InputBuffer,
                        Receive_PDU_Len);
                }
                break;

            case FRAME_TYPE_TEST_REQUEST:
                /* Test Response is handled by MS/TP state machine */
                pdu_len = mstp_port->DataLength;
                break;

            default:
                break;
        }

        Valid_Frame_Count++;
    }

    return pdu_len;
}

/**
 * @brief Callback from MS/TP to send data
 * @param mstp_port - MS/TP port structure
 * @param buffer - data to send
 * @param nbytes - number of bytes to send
 */
void MSTP_Send_Frame(
    struct mstp_port_struct_t *mstp_port,
    const uint8_t *buffer,
    uint16_t nbytes)
{
    (void)mstp_port;

    if (RS485_Transmit_Ready()) {
        RS485_Send_Frame((uint8_t *)buffer, nbytes);
    }
}

/**
 * @brief Get silence timer value in milliseconds
 * @param mstp_port - MS/TP port structure
 * @return Current timer value
 */
uint32_t MSTP_SilenceTimer(struct mstp_port_struct_t *mstp_port)
{
    (void)mstp_port;
    return mstimer_now();
}

/**
 * @brief Reset silence timer
 * @param mstp_port - MS/TP port structure
 */
void MSTP_SilenceTimerReset(struct mstp_port_struct_t *mstp_port)
{
    (void)mstp_port;
    /* Timer reset is handled internally by mstimer */
}
