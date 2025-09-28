/**
 * @file
 * @brief Hardware definitions for PIC32MX795F512L (Olimex PIC32-RETROBSD board)
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include <stdbool.h>

#ifndef TEST_MODE
/* Only include PIC32 specific headers when not in test mode */
#include <xc.h>
#include <sys/attribs.h>
#endif

/* System Clock Configuration */
#define SYS_FREQ            80000000UL  // 80MHz system clock
#define GetSystemClock()    (SYS_FREQ)
#define GetPeripheralClock() (SYS_FREQ/1)  // FPBDIV = DIV_1

/* UART1 - BACnet MS/TP Interface Pin Assignments */
#define U1RX_PIN    (1 << 2)   // RF2 - UART1 RX
#define U1TX_PIN    (1 << 8)   // RF8 - UART1 TX
#define U1CTS_PIN   (1 << 12)  // RD12 - CTS (flow control)
#define U1RTS_PIN   (1 << 5)   // RD5 - RTS/DE (RS-485 Driver Enable)

/* RS-485 Driver Enable Control */
#define ENA_BACNET  (LATDSET = U1RTS_PIN)    // Enable transmitter
#define DIS_BACNET  (LATDCLR = U1RTS_PIN)    // Enable receiver

/* UART2 - Modbus RTU Interface (if needed) */
#define U2RX_PIN    (1 << 4)   // RF4 - UART2 RX
#define U2TX_PIN    (1 << 5)   // RF5 - UART2 TX
#define U2RTS_PIN   (1 << 13)  // RD13 - RTS/DE (RS-485 Driver Enable)

#ifndef TEST_MODE
/* LED Pin Definitions */
#define LED1_PIN    (1 << 6)   // RD6 - Red LED
#define LED2_PIN    (1 << 7)   // RD7 - Yellow LED
#define LED3_PIN    (1 << 8)   // RD8 - Green LED

/* Button Definitions */
#define USER_BUTTON PORTDbits.RD0  // User button
#define PROG_BUTTON PORTDbits.RD1  // Program/Mode button
#endif

#ifndef TEST_MODE
/* Timing Macros - simple busy wait delays */
#define DelayUs(x)  do { \
    unsigned long _cycles = (unsigned long)(((unsigned long long)(x) * GetPeripheralClock()) / 1000000ULL); \
    unsigned long _start = _CP0_GET_COUNT(); \
    while((_CP0_GET_COUNT() - _start) < (_cycles/2)); \
} while(0)

#define DelayMs(x)  do { \
    unsigned long _ms = (x); \
    while(_ms--) DelayUs(1000); \
} while(0)
#else
/* Test mode stubs */
#define DelayUs(x) do {} while(0)
#define DelayMs(x) do {} while(0)
#endif

/* Interrupt Priority Levels */
#define TIMER1_PRIORITY     6  // System tick (highest)
#define UART1_PRIORITY      5  // BACnet MS/TP
#define UART2_PRIORITY      3  // Modbus RTU
#define TIMER2_PRIORITY     3  // Modbus timing

/* FIFO Buffer Sizes */
#define BACNET_RX_FIFO_SIZE 512
#define BACNET_TX_FIFO_SIZE 512

/* Flash Memory Configuration */
#define FLASH_PAGE_SIZE     4096    // 4KB per page
#define CONFIG_FLASH_ADDR   0x9D070000  // User configuration area

/* Watchdog Configuration */
#define RESTART_WDT()   (WDTCONSET = _WDTCON_WDTCLR_MASK)
#define ENABLE_WDT()    (WDTCONSET = _WDTCON_ON_MASK)
#define DISABLE_WDT()   (WDTCONCLR = _WDTCON_ON_MASK)

/* System Configuration - removed external function declarations as they're not available in XC32 */

/* Peripheral Library Compatibility */
#define mPORTDSetBits(bits)     LATDSET = (bits)
#define mPORTDClearBits(bits)   LATDCLR = (bits)
#define mPORTDToggleBits(bits)  LATDINV = (bits)

#define mPORTFSetBits(bits)     LATFSET = (bits)
#define mPORTFClearBits(bits)   LATFCLR = (bits)
#define mPORTFToggleBits(bits)  LATFINV = (bits)

#endif /* HARDWARE_H */