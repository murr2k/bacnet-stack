/**
 * @file
 * @brief MS/TP Interface API regression tests
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include "bacnet/datalink/mstp.h"
#include "bacnet/datalink/dlmstp.h"
#include "bacnet/bacdef.h"
#include <string.h>

/* External functions from dlmstp.c */
extern void dlmstp_init(char *ifname);
extern void dlmstp_cleanup(void);
extern int dlmstp_send_pdu(BACNET_ADDRESS *dest, BACNET_NPDU_DATA *npdu_data,
                           uint8_t *pdu, unsigned pdu_len);
extern uint16_t dlmstp_receive(BACNET_ADDRESS *src, uint8_t *pdu,
                               uint16_t max_pdu, unsigned timeout);
extern void dlmstp_get_my_address(BACNET_ADDRESS *my_address);
extern void dlmstp_get_broadcast_address(BACNET_ADDRESS *dest);
extern void dlmstp_set_mac_address(uint8_t mac_address);
extern void dlmstp_set_max_info_frames(uint8_t max_info_frames);
extern void dlmstp_set_max_master(uint8_t max_master);
extern void dlmstp_set_baud_rate(uint32_t baud);

/* Test that all required MSTP API functions exist with correct signatures */
static bool test_mstp_api_signatures(void)
{
    /* Initialization and cleanup */
    VERIFY_API_SIGNATURE(dlmstp_init, void, char*);
    VERIFY_API_SIGNATURE(dlmstp_cleanup, void, void);

    /* Send and receive */
    VERIFY_API_SIGNATURE(dlmstp_send_pdu, int, BACNET_ADDRESS*, BACNET_NPDU_DATA*,
                        uint8_t*, unsigned);
    VERIFY_API_SIGNATURE(dlmstp_receive, uint16_t, BACNET_ADDRESS*, uint8_t*,
                        uint16_t, unsigned);

    /* Address management */
    VERIFY_API_SIGNATURE(dlmstp_get_my_address, void, BACNET_ADDRESS*);
    VERIFY_API_SIGNATURE(dlmstp_get_broadcast_address, void, BACNET_ADDRESS*);

    /* Configuration */
    VERIFY_API_SIGNATURE(dlmstp_set_mac_address, void, uint8_t);
    VERIFY_API_SIGNATURE(dlmstp_set_max_info_frames, void, uint8_t);
    VERIFY_API_SIGNATURE(dlmstp_set_max_master, void, uint8_t);
    VERIFY_API_SIGNATURE(dlmstp_set_baud_rate, void, uint32_t);

    /* MSTP callback functions */
    VERIFY_API_SIGNATURE(MSTP_Get_Send, uint16_t, struct mstp_port_struct_t*, unsigned);
    VERIFY_API_SIGNATURE(MSTP_Get_Receive, uint16_t, struct mstp_port_struct_t*);
    VERIFY_API_SIGNATURE(MSTP_Put_Receive, uint16_t, struct mstp_port_struct_t*);
    VERIFY_API_SIGNATURE(MSTP_Send_Frame, void, struct mstp_port_struct_t*,
                        const uint8_t*, uint16_t);

    /* RS485 functions */
    VERIFY_API_SIGNATURE(rs485_init, void, void);
    VERIFY_API_SIGNATURE(rs485_cleanup, void, void);
    VERIFY_API_SIGNATURE(rs485_send_break, void, void);
    VERIFY_API_SIGNATURE(rs485_bytes_available, bool, void);
    VERIFY_API_SIGNATURE(rs485_byte_available, bool, uint8_t*);
    VERIFY_API_SIGNATURE(rs485_transmit_bytes, void, const uint8_t*, uint16_t);
    VERIFY_API_SIGNATURE(rs485_rts_enable, void, bool);
    VERIFY_API_SIGNATURE(rs485_silence_reset, void, void);
    VERIFY_API_SIGNATURE(rs485_silence_elapsed, uint32_t, void);
    VERIFY_API_SIGNATURE(rs485_baud_rate, bool, uint32_t);

    return true;
}

/* Test MSTP initialization */
static bool test_mstp_initialization(void)
{
    /* Initialize MSTP */
    dlmstp_init(NULL);

    /* Get initial address */
    BACNET_ADDRESS my_address;
    dlmstp_get_my_address(&my_address);

    /* Verify address fields */
    TEST_ASSERT_EQUAL(2, my_address.net);  /* MS/TP is network 2 */
    TEST_ASSERT(my_address.mac_len > 0, "MAC length should be set");
    TEST_ASSERT(my_address.mac_len <= MAX_MAC_LEN, "MAC length should be valid");

    /* Get broadcast address */
    BACNET_ADDRESS broadcast;
    dlmstp_get_broadcast_address(&broadcast);
    TEST_ASSERT_EQUAL(BACNET_BROADCAST_NETWORK, broadcast.net);
    TEST_ASSERT_EQUAL(0xFF, broadcast.mac[0]);  /* MS/TP broadcast is 0xFF */
    TEST_ASSERT_EQUAL(1, broadcast.mac_len);

    /* Cleanup */
    dlmstp_cleanup();

    return true;
}

/* Test MSTP configuration */
static bool test_mstp_configuration(void)
{
    /* Initialize */
    dlmstp_init(NULL);

    /* Test MAC address setting */
    dlmstp_set_mac_address(5);
    BACNET_ADDRESS my_address;
    dlmstp_get_my_address(&my_address);
    TEST_ASSERT_EQUAL(5, my_address.mac[0]);

    /* Test different MAC addresses */
    dlmstp_set_mac_address(127);
    dlmstp_get_my_address(&my_address);
    TEST_ASSERT_EQUAL(127, my_address.mac[0]);

    /* Test max info frames setting */
    dlmstp_set_max_info_frames(10);
    /* Note: No getter to verify, but function should not crash */

    /* Test max master setting */
    dlmstp_set_max_master(127);
    /* Note: No getter to verify, but function should not crash */

    /* Test baud rate setting */
    dlmstp_set_baud_rate(38400);
    /* Note: No getter to verify, but function should not crash */

    /* Cleanup */
    dlmstp_cleanup();

    return true;
}

/* Test MSTP send PDU */
static bool test_mstp_send_pdu(void)
{
    /* Initialize */
    dlmstp_init(NULL);

    /* Prepare test data */
    BACNET_ADDRESS dest;
    BACNET_NPDU_DATA npdu_data;
    uint8_t pdu[50];
    int result;

    /* Setup destination address */
    dest.net = 0;  /* Local network */
    dest.mac_len = 1;
    dest.mac[0] = 10;  /* Destination MAC */

    /* Setup NPDU data */
    memset(&npdu_data, 0, sizeof(npdu_data));
    npdu_data.data_expecting_reply = false;
    npdu_data.priority = BACNET_MESSAGE_PRIORITY_NORMAL;

    /* Setup PDU */
    memset(pdu, 0xAA, sizeof(pdu));

    /* Send PDU */
    result = dlmstp_send_pdu(&dest, &npdu_data, pdu, sizeof(pdu));
    TEST_ASSERT(result >= 0, "Send should succeed or return valid error");

    /* Test broadcast */
    dlmstp_get_broadcast_address(&dest);
    result = dlmstp_send_pdu(&dest, &npdu_data, pdu, sizeof(pdu));
    TEST_ASSERT(result >= 0, "Broadcast send should succeed or return valid error");

    /* Test NULL parameters */
    result = dlmstp_send_pdu(NULL, &npdu_data, pdu, sizeof(pdu));
    TEST_ASSERT(result < 0, "Should fail with NULL destination");

    result = dlmstp_send_pdu(&dest, &npdu_data, NULL, sizeof(pdu));
    TEST_ASSERT(result < 0, "Should fail with NULL PDU");

    result = dlmstp_send_pdu(&dest, &npdu_data, pdu, 0);
    TEST_ASSERT(result <= 0, "Should fail or return 0 with zero length");

    /* Cleanup */
    dlmstp_cleanup();

    return true;
}

/* Test MSTP receive */
static bool test_mstp_receive(void)
{
    /* Initialize */
    dlmstp_init(NULL);

    /* Prepare receive buffer */
    BACNET_ADDRESS src;
    uint8_t pdu[MAX_MPDU];
    uint16_t pdu_len;

    /* Try to receive with no data (should timeout) */
    pdu_len = dlmstp_receive(&src, pdu, sizeof(pdu), 0);
    TEST_ASSERT_EQUAL(0, pdu_len);  /* No data should be available */

    /* Test with NULL parameters */
    pdu_len = dlmstp_receive(NULL, pdu, sizeof(pdu), 0);
    TEST_ASSERT_EQUAL(0, pdu_len);

    pdu_len = dlmstp_receive(&src, NULL, sizeof(pdu), 0);
    TEST_ASSERT_EQUAL(0, pdu_len);

    pdu_len = dlmstp_receive(&src, pdu, 0, 0);
    TEST_ASSERT_EQUAL(0, pdu_len);

    /* Cleanup */
    dlmstp_cleanup();

    return true;
}

/* Test RS485 interface */
static bool test_rs485_interface(void)
{
    /* Initialize RS485 */
    rs485_init();

    /* Test bytes available */
    bool available = rs485_bytes_available();
    TEST_ASSERT_FALSE(available);  /* Should be empty initially */

    /* Test byte available */
    uint8_t byte;
    bool got_byte = rs485_byte_available(&byte);
    TEST_ASSERT_FALSE(got_byte);  /* Should be empty */

    /* Test RTS control */
    rs485_rts_enable(true);
    /* Note: No way to verify, but should not crash */
    rs485_rts_enable(false);

    /* Test silence timer */
    rs485_silence_reset();
    uint32_t elapsed = rs485_silence_elapsed();
    TEST_ASSERT(elapsed >= 0, "Elapsed time should be non-negative");

    /* Test baud rate setting */
    bool result = rs485_baud_rate(9600);
    TEST_ASSERT_TRUE(result);

    result = rs485_baud_rate(38400);
    TEST_ASSERT_TRUE(result);

    result = rs485_baud_rate(76800);
    TEST_ASSERT_TRUE(result);

    /* Test invalid baud rate */
    result = rs485_baud_rate(0);
    TEST_ASSERT_FALSE(result);

    /* Cleanup */
    rs485_cleanup();

    return true;
}

/* Test MSTP port structure operations */
static bool test_mstp_port_struct(void)
{
    struct mstp_port_struct_t port;
    uint16_t result;

    /* Initialize port structure */
    memset(&port, 0, sizeof(port));

    /* Test MSTP_Get_Send with empty queue */
    result = MSTP_Get_Send(&port, 0);
    TEST_ASSERT_EQUAL(0, result);

    /* Test MSTP_Get_Receive with empty queue */
    result = MSTP_Get_Receive(&port);
    TEST_ASSERT_EQUAL(0, result);

    /* Test MSTP_Put_Receive with empty data */
    result = MSTP_Put_Receive(&port);
    TEST_ASSERT_EQUAL(0, result);

    /* Test MSTP_Send_Frame */
    uint8_t frame[50];
    memset(frame, 0x55, sizeof(frame));
    MSTP_Send_Frame(&port, frame, sizeof(frame));
    /* Note: No way to verify, but should not crash */

    /* Test with NULL port */
    result = MSTP_Get_Send(NULL, 0);
    TEST_ASSERT_EQUAL(0, result);

    result = MSTP_Get_Receive(NULL);
    TEST_ASSERT_EQUAL(0, result);

    result = MSTP_Put_Receive(NULL);
    TEST_ASSERT_EQUAL(0, result);

    MSTP_Send_Frame(NULL, frame, sizeof(frame));
    /* Should handle gracefully */

    return true;
}

/* Test MSTP timing constraints */
static bool test_mstp_timing(void)
{
    /* Initialize */
    dlmstp_init(NULL);
    rs485_init();

    /* Test silence timer */
    rs485_silence_reset();
    uint32_t start = rs485_silence_elapsed();

    /* Small delay to test timer */
    for (volatile int i = 0; i < 10000; i++);

    uint32_t end = rs485_silence_elapsed();
    TEST_ASSERT(end >= start, "Timer should advance");

    /* Reset and verify */
    rs485_silence_reset();
    uint32_t after_reset = rs485_silence_elapsed();
    TEST_ASSERT(after_reset < end, "Timer should reset");

    /* Cleanup */
    rs485_cleanup();
    dlmstp_cleanup();

    return true;
}

/* Main test runner for MSTP API */
void run_mstp_api_tests(TestSuite *suite)
{
    current_suite = suite;

    RUN_TEST(test_mstp_api_signatures);
    RUN_TEST(test_mstp_initialization);
    RUN_TEST(test_mstp_configuration);
    RUN_TEST(test_mstp_send_pdu);
    RUN_TEST(test_mstp_receive);
    RUN_TEST(test_rs485_interface);
    RUN_TEST(test_mstp_port_struct);
    RUN_TEST(test_mstp_timing);
}