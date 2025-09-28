/**
 * @file
 * @brief Main source file for BACnet-stack PIC32MX795F512L port
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include <xc.h>
#include <sys/attribs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bacnet/basic/sys/debug.h"
#include "dlmstp.h"
#include "bacnet/datalink/mstp.h"
#include "bacnet/basic/sys/mstimer.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/dcc.h"
#include "bacnet/iam.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/ai.h"
#include "bacnet/basic/object/ao.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"
#include "rs485.h"
#include "hardware.h"

/* Configuration Bits */
#pragma config FPLLODIV = DIV_1     // PLL Output Divider: 1x
#pragma config FPLLMUL = MUL_20     // PLL Multiplier: 20x
#pragma config FPLLIDIV = DIV_2     // PLL Input Divider: 2x
#pragma config FPBDIV = DIV_1       // Peripheral Bus Divider: 1x
#pragma config FNOSC = PRIPLL       // Oscillator Selection: Primary with PLL
#pragma config POSCMOD = XT         // Primary Oscillator Mode: XT Crystal
#pragma config FCKSM = CSDCMD       // Clock Switching Disabled
#pragma config FWDTEN = OFF         // Watchdog Timer Disabled (enable in production)
#pragma config DEBUG = OFF          // Background Debugger Disabled
#pragma config ICESEL = ICS_PGx2    // ICE/ICD Comm Channel Select
#pragma config BWP = OFF            // Boot Flash Write Protect
#pragma config CP = OFF             // Code Protect

/* Global Variables */
volatile uint32_t millisecond_counter = 0;  /* Made non-static for use in rs485.c */
static volatile uint16_t mstp_timer = 0;
static volatile uint16_t mstp_silence_timer = 0;

/* LEDs for status indication */
#define mLED_1  LATDbits.LATD6  // Red LED
#define mLED_2  LATDbits.LATD7  // Yellow LED
#define mLED_3  LATDbits.LATD8  // Green LED

#define RedLEDOn()      mLED_1 = 1
#define RedLEDOff()     mLED_1 = 0
#define YelLEDOn()      mLED_2 = 1
#define YelLEDOff()     mLED_2 = 0
#define GrnLEDOn()      mLED_3 = 1
#define GrnLEDOff()     mLED_3 = 0

/* Function Prototypes */
static void Init_Hardware(void);
static void Init_Timer1(void);
static void Init_LEDs(void);

/**
 * @brief Initialize hardware peripherals
 */
static void Init_Hardware(void)
{
    /* Configure system for optimal performance */
    /* Set PBDIV to DIV_1 for 80MHz peripheral bus */
    OSCCONbits.PBDIV = 0b00;  // PBCLK = SYSCLK/1

    /* Set Flash Wait States and Prefetch Cache */
    CHECONbits.PFMWS = 2;   // 2 wait states for 80MHz
    CHECONbits.PREFEN = 1;  // Enable prefetch

    /* Disable JTAG to free up pins */
    DDPCONbits.JTAGEN = 0;

    /* Configure all analog pins as digital */
    AD1PCFG = 0xFFFF;

    /* Initialize peripherals */
    Init_LEDs();
    Init_Timer1();

    /* Enable multi-vectored interrupts */
    INTCONbits.MVEC = 1;
    __builtin_enable_interrupts();
}

/**
 * @brief Initialize LED pins
 */
static void Init_LEDs(void)
{
    /* Configure LED pins as outputs */
    LATDCLR = 0x1C0;  // Clear RD6, RD7, RD8
    TRISDCLR = 0x1C0; // Set as outputs

    /* Turn off all LEDs initially */
    RedLEDOff();
    YelLEDOff();
    GrnLEDOff();
}

/**
 * @brief Initialize Timer1 for 1ms system tick
 */
static void Init_Timer1(void)
{
    T1CON = 0x00;                          // Stop timer, prescaler 1:1
    TMR1 = 0x00;                           // Clear timer register
    PR1 = (GetPeripheralClock()/1000) - 1; // 1ms period

    /* Configure interrupt */
    IPC1SET = _IPC1_T1IP_MASK & (6 << _IPC1_T1IP_POSITION);  // Priority 6
    IFS0CLR = _IFS0_T1IF_MASK;            // Clear interrupt flag
    IEC0SET = _IEC0_T1IE_MASK;            // Enable interrupt

    T1CONSET = _T1CON_ON_MASK;            // Start timer
}

/**
 * @brief Timer1 interrupt handler (1ms tick)
 */
void __ISR(_TIMER_1_VECTOR, IPL6AUTO) Timer1Handler(void)
{
    IFS0CLR = _IFS0_T1IF_MASK;  // Clear interrupt flag

    millisecond_counter++;

    /* Update MS/TP timers */
    if (mstp_timer > 0) {
        mstp_timer--;
    }
    if (mstp_silence_timer > 0) {
        mstp_silence_timer--;
    }

    /* Toggle heartbeat LED every 500ms */
    if ((millisecond_counter % 500) == 0) {
        mLED_3 ^= 1;  // Toggle green LED
    }
}

/**
 * @brief Get system timer (milliseconds)
 */
unsigned long mstimer_now(void)
{
    return millisecond_counter;
}

/**
 * @brief Initialize timer system
 */
void mstimer_init(void)
{
    /* Timer already initialized in hardware init */
}

/**
 * @brief Set timer
 */
void mstimer_set(struct mstimer *t, unsigned long interval)
{
    if (t) {
        t->start = millisecond_counter;
        t->interval = interval;
    }
}

/**
 * @brief Check if timer expired
 */
int mstimer_expired(const struct mstimer *t)
{
    if (t) {
        return ((millisecond_counter - t->start) >= t->interval);
    }
    return 1;
}

/**
 * @brief Reset timer
 */
void mstimer_reset(struct mstimer *t)
{
    if (t) {
        t->start = millisecond_counter;
    }
}

/**
 * @brief Get remaining time
 */
unsigned long mstimer_remaining(const struct mstimer *t)
{
    unsigned long elapsed;

    if (!t) {
        return 0;
    }

    elapsed = millisecond_counter - t->start;
    if (elapsed >= t->interval) {
        return 0;
    }

    return (t->interval - elapsed);
}

/**
 * @brief Get elapsed time
 */
unsigned long mstimer_elapsed(const struct mstimer *t)
{
    if (t) {
        return (millisecond_counter - t->start);
    }
    return 0;
}

/**
 * @brief Restart timer
 */
void mstimer_restart(struct mstimer *t)
{
    mstimer_reset(t);
}

/**
 * @brief Expire timer
 */
void mstimer_expire(struct mstimer *t)
{
    if (t) {
        t->start = millisecond_counter - t->interval;
    }
}

/**
 * @brief Get timer interval
 */
unsigned long mstimer_interval(const struct mstimer *t)
{
    if (t) {
        return t->interval;
    }
    return 0;
}

/**
 * @brief Reset MS/TP silence timer (local function)
 */
void mstimer_silence_reset(void)
{
    mstp_silence_timer = 0;
}

/**
 * @brief Main application entry point
 */
int main(void)
{
    uint8_t my_mac_address = 1;  // Default MS/TP MAC address

    /* Initialize hardware */
    Init_Hardware();

    /* Initialize RS-485 interface */
    RS485_Initialize();
    RS485_Set_Baud_Rate(19200);  // Default baud rate per PICS

    /* Initialize BACnet stack */
    Device_Init(NULL);

    /* Set device instance */
    Device_Set_Object_Instance_Number(12345);  // Example device instance

    /* Initialize BACnet objects */
    Analog_Input_Init();
    Analog_Output_Init();
    Binary_Input_Init();
    Binary_Output_Init();

    /* Configure MS/TP datalink */
    dlmstp_set_mac_address(my_mac_address);
    dlmstp_set_max_master(127);
    dlmstp_set_max_info_frames(1);
    dlmstp_init(NULL);

    /* Enable Device Communication Control */
    dcc_set_status_duration(COMMUNICATION_ENABLE, 0);

    /* Send I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);

    /* Main application loop */
    while (1) {
        /* Handle MS/TP datalink */
        dlmstp_task();

        /* Handle received BACnet messages (handled inside dlmstp_task) */

        /* Handle client TSM timeouts */
        tsm_timer_milliseconds(1);

        /* Handle device communication control */
        dcc_timer_seconds(1);

        /* Application specific tasks */
        /* Add any analog/binary input updates here */

        /* Restart watchdog if enabled */
        /* WDTCONSET = _WDTCON_WDTCLR_MASK; */
    }

    return 0;
}