/**
 * @file
 * @brief BACnet MS/TP datalink layer header for PIC32MX795F512L
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#ifndef DLMSTP_H
#define DLMSTP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacnet/bacdef.h"
#include "bacnet/npdu.h"

/* defines specific to MS/TP */
#define DLMSTP_HEADER_MAX (2 + 1 + 1 + 1 + 2 + 1)
#define MAX_MPDU 501 /* MS/TP maximum MPDU size */
#define DLMSTP_MPDU_MAX (DLMSTP_HEADER_MAX + MAX_PDU)

typedef struct dlmstp_packet {
    bool ready; /* true if ready to be sent or received */
    BACNET_ADDRESS address; /* source address */
    uint8_t frame_type; /* type of message */
    unsigned pdu_len; /* packet length */
    uint8_t pdu[DLMSTP_MPDU_MAX]; /* packet */
} DLMSTP_PACKET;

/* Handler buffer declaration - removed, as it's already declared in tsm.h */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool dlmstp_init(char *ifname);
void dlmstp_cleanup(void);
void dlmstp_task(void);
void dlmstp_rs485_rx_event(void);

/* returns number of bytes sent on success, negative on failure */
int dlmstp_send_pdu(
    BACNET_ADDRESS *dest, /* destination address */
    BACNET_NPDU_DATA *npdu_data, /* network information */
    uint8_t *pdu, /* any data to be sent - may be null */
    unsigned pdu_len); /* number of bytes of data */

void dlmstp_get_my_address(BACNET_ADDRESS *my_address);
void dlmstp_get_broadcast_address(BACNET_ADDRESS *dest);

void dlmstp_set_mac_address(uint8_t mac_address);
uint8_t dlmstp_mac_address(void);

void dlmstp_set_max_info_frames(uint8_t max_info_frames);
uint8_t dlmstp_max_info_frames(void);

void dlmstp_set_max_master(uint8_t max_master);
uint8_t dlmstp_max_master(void);

bool dlmstp_set_baud_rate(uint32_t baud);
uint32_t dlmstp_baud_rate(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* DLMSTP_H */
