# Board Support Package (BSP) Documentation

## Olimex PIC32-RETROBSD Hardware Configuration for CETCI BACnet Module

Author: Murray Kopit (murr2k@gmail.com)
Date: September 23, 2025
Version: 1.6.0

---

## Executive Summary

This document details the Board Support Package (BSP) configuration for the Olimex PIC32-RETROBSD development board (formerly referred to as PIC-32-PINGUINO-MICRO in legacy code comments) as configured for the CETCI BACnet-to-Modbus gateway module. The BSP encompasses hardware initialization, peripheral configuration, pin assignments, and timing requirements specific to the BACnet MS/TP and Modbus RTU protocols.

---

## Hardware Platform

### Board Specifications

**Development Board**: Olimex PIC32-RETROBSD Rev A
**Microcontroller**: Microchip PIC32MX795F512H
**Architecture**: MIPS32 M4K core
**Operating Frequency**: 80 MHz system clock
**Memory Configuration**:
- Flash: 512 KB
- RAM: 128 KB
- Boot Flash: 12 KB

### Clock Configuration

```c
// Configuration bits in main.c
#pragma config FPLLODIV = DIV_1   // PLL Output Divider: 1x
#pragma config FPLLMUL = MUL_20   // PLL Multiplier: 20x
#pragma config FPLLIDIV = DIV_2   // PLL Input Divider: 2x
#pragma config FPBDIV = DIV_1     // Peripheral Bus Divider: 1x
#pragma config FNOSC = PRIPLL     // Oscillator Selection: Primary with PLL
#pragma config POSCMOD = XT       // Primary Oscillator Mode: XT Crystal
#pragma config FCKSM = CSDCMD     // Clock Switching Disabled, Fail-Safe Monitor Disabled

// Resulting frequencies:
// Crystal: 8 MHz
// PLL Input: 8 MHz / 2 = 4 MHz
// VCO Output: 4 MHz × 20 = 80 MHz
// System Clock: 80 MHz / 1 = 80 MHz
// Peripheral Bus: 80 MHz / 1 = 80 MHz
```

---

## Pin Assignments and Peripheral Mapping

### UART1 - BACnet MS/TP Interface

**Function**: BACnet MS/TP communication over RS-485
**Baud Rates**: 9600, 19200 (default), 38400, 57600, 76800, 115200
**Data Format**: 8N1 (8 data bits, no parity, 1 stop bit)

```c
// Pin assignments (uart485.h)
#define U1RX_PIN  (1 << 2)  // RF2 - UART1 RX
#define U1TX_PIN  (1 << 8)  // RF8 - UART1 TX
#define U1CTS_PIN (1 << 12) // RD12 - CTS (flow control)
#define U1RTS_PIN (1 << 5)  // RD5 - RTS/DE (Driver Enable)

// RS-485 Driver Enable control
#define ENA_BACNET  (mPORTDSetBits(U1RTS_PIN))    // Enable transmitter
#define DIS_BACNET  (mPORTDClearBits(U1RTS_PIN))  // Enable receiver
```

### UART2 - Modbus RTU Interface

**Function**: Modbus RTU Master communication
**Baud Rate**: 19200 (fixed)
**Data Format**: 8E1 (8 data bits, even parity, 1 stop bit)

```c
// Pin assignments
#define U2RX_PIN  (1 << 4)  // RF4 - UART2 RX
#define U2TX_PIN  (1 << 5)  // RF5 - UART2 TX
#define U2RTS_PIN (1 << 13) // RD13 - RTS/DE (Driver Enable)

// RS-485 Driver Enable control
#define ENA_MOD  (mPORTDSetBits(U2RTS_PIN))    // Enable transmitter
#define DIS_MOD  (mPORTDClearBits(U2RTS_PIN))  // Enable receiver
```

### LED Indicators

```c
// Status LED assignments (user.h)
#define mInitAllLEDs()  LATDCLR = 0x1C0; TRISDCLR = 0x1C0
#define mLED_1          LATDbits.LATD6  // Red LED
#define mLED_2          LATDbits.LATD7  // Yellow LED
#define mLED_3          LATDbits.LATD8  // Green LED

// LED control macros
#define RedLEDOn()      mLED_1 = 1
#define RedLEDOff()     mLED_1 = 0
#define YelLEDOn()      mLED_2 = 1
#define YelLEDOff()     mLED_2 = 0
#define GrnLEDOn()      mLED_3 = 1
#define GrnLEDOff()     mLED_3 = 0
```

### Buttons and Mode Selection

```c
// Button definitions
#define swUser      PORTDbits.RD0   // User button
#define swProgram   PORTDbits.RD1   // Program/Mode button

// Operating mode detection
#define BACnet_FLAG swProgram  // High = BACnet mode, Low = Modbus mode
```

### Debug/Timing Output

```c
// Port F bit 0 used for oscilloscope timing measurements
#define TIMING_PIN  (1 << 0)  // RF0
mPORTFSetBits(TIMING_PIN);    // Set high
mPORTFClearBits(TIMING_PIN);  // Set low
mPORTFToggleBits(TIMING_PIN); // Toggle
```

---

## Timer Configuration

### Timer1 - System Tick (1ms)

```c
// Timer1 configuration for 1ms tick
void Init_Timer1(void)
{
    T1CON = 0x00;                     // Stop timer, prescaler 1:1
    TMR1 = 0x00;                      // Clear timer register
    PR1 = (GetPeripheralClock()/1000) - 1;  // 1ms period
    IPC1SET = _IPC1_T1IP_MASK & (6 << _IPC1_T1IP_POSITION);  // Priority 6
    IFS0CLR = _IFS0_T1IF_MASK;       // Clear interrupt flag
    IEC0SET = _IEC0_T1IE_MASK;       // Enable interrupt
    T1CONSET = _T1CON_ON_MASK;       // Start timer
}

// Timer1 ISR - handles system timing
void __ISR(_TIMER_1_VECTOR, ipl6) Timer1Handler(void)
{
    mT1ClearIntFlag();
    millisecond_counter++;

    // BACnet MS/TP timing
    if(MSTP_Timer) MSTP_Timer--;
    if(MSTP_Silence_Timer) MSTP_Silence_Timer--;

    // Execute BACnet tasks
    dlmstp_task();
    BACnet_task();
}
```

### Timer2 - Modbus RTU Timing

```c
// Timer2 for Modbus character/frame timing
void Init_Timer2(uint32_t timeout_us)
{
    T2CON = 0x00;                     // Stop timer
    TMR2 = 0x00;                      // Clear timer

    // Calculate period for timeout
    uint32_t period = (GetPeripheralClock() / 1000000) * timeout_us;

    if(period <= 65535) {
        T2CONbits.TCKPS = 0;  // 1:1 prescaler
        PR2 = period - 1;
    } else {
        T2CONbits.TCKPS = 7;  // 1:256 prescaler
        PR2 = (period / 256) - 1;
    }

    IPC2SET = _IPC2_T2IP_MASK & (3 << _IPC2_T2IP_POSITION);  // Priority 3
    IFS0CLR = _IFS0_T2IF_MASK;
    IEC0SET = _IEC0_T2IE_MASK;
    T2CONSET = _T2CON_ON_MASK;
}
```

---

## Interrupt Configuration

### Interrupt Priority Levels

```c
// Priority assignments (higher number = higher priority)
#define TIMER1_PRIORITY     6  // System tick (highest)
#define UART1_PRIORITY      5  // BACnet MS/TP
#define UART2_PRIORITY      3  // Modbus RTU
#define TIMER2_PRIORITY     3  // Modbus timing
```

### UART1 Interrupt Handler (BACnet)

```c
void __ISR(_UART1_VECTOR, ipl5) IntUART1Handler(void)
{
    // RX interrupt
    if(INTGetFlag(INT_SOURCE_UART_RX(UART1))) {
        INTClearFlag(INT_SOURCE_UART_RX(UART1));

        while(UARTReceivedDataIsAvailable(UART1)) {
            uint8_t data = UARTGetDataByte(UART1);

            // Store in FIFO with timestamp
            FIFO_Add(&bac_rx_fifo, &millisecond_counter, 4);
            FIFO_Add(&bac_rx_fifo, &data, 1);
        }
    }

    // TX interrupt
    if(INTGetFlag(INT_SOURCE_UART_TX(UART1))) {
        INTClearFlag(INT_SOURCE_UART_TX(UART1));

        if(bac_tx_count > 0) {
            UARTSendDataByte(UART1, bac_tx_buffer[bac_tx_pos++]);
            bac_tx_count--;
        } else {
            // Transmission complete
            INTEnable(INT_SOURCE_UART_TX(UART1), INT_DISABLED);
        }
    }
}
```

---

## RS-485 Physical Layer Configuration

### Hardware Requirements

1. **Transceiver IC**: Isolated RS-485 transceiver (ADM2483, MAX14840, or equivalent)
2. **Bus Biasing Resistors**:
   - 680Ω pull-up to +5V on A/+ line
   - 680Ω pull-down to GND on B/- line
3. **Termination Resistors**: 120Ω between A and B lines at bus ends
4. **Protection**: TVS diodes for surge protection (optional but recommended)

### Driver Enable Timing

```c
// RS-485 turnaround timing for MS/TP
void RS485_Send_Frame(uint8_t *buffer, uint16_t nbytes)
{
    // Pre-transmit delay
    DelayMs(2);  // Minimum 2ms turnaround time

    // Enable transmitter
    ENA_BACNET;
    DelayUs(10); // Driver enable setup time

    // Send data
    for(uint16_t i = 0; i < nbytes; i++) {
        UARTSendDataByte(UART1, buffer[i]);
    }

    // Wait for transmission complete
    while(!UARTTransmissionHasCompleted(UART1));

    // Disable transmitter
    DelayUs(10); // Hold time
    DIS_BACNET;
}
```

---

## Watchdog Timer Configuration

```c
// Watchdog configuration
#pragma config FWDTEN = ON      // Watchdog Timer Enable
#pragma config WDTPS = PS32768  // Watchdog Timer Postscaler (32.768 seconds)

// Watchdog control
#define RESTART_WDT()  (WDTCONSET = _WDTCON_WDTCLR_MASK)
#define ENABLE_WDT()   (WDTCONSET = _WDTCON_ON_MASK)
#define DISABLE_WDT()  (WDTCONCLR = _WDTCON_ON_MASK)
```

---

## Flash Memory Configuration

### Memory Map

```
0x9D000000 - 0x9D07FFFF : Program Flash (512KB)
0x9FC00000 - 0x9FC02FFF : Boot Flash (12KB)
0xBFC00000 - 0xBFC02FFF : Boot Flash (mapped)

// Configuration storage location
0x9D070000 - 0x9D073FFF : User Configuration Area (16KB)
```

### Non-Volatile Storage Implementation

```c
// Flash page operations
#define FLASH_PAGE_SIZE     4096    // 4KB per page
#define CONFIG_FLASH_ADDR   0x9D070000

void Flash_Write_Config(CONFIG_DATA *config)
{
    // Disable interrupts
    uint32_t status = INTDisableInterrupts();

    // Erase page
    NVMErasePage((void*)CONFIG_FLASH_ADDR);

    // Write configuration
    NVMWriteRow((void*)CONFIG_FLASH_ADDR, (void*)config);

    // Restore interrupts
    INTRestoreInterrupts(status);
}
```

---

## Startup Sequence

```c
void InitializeHardware(void)
{
    // 1. Configure system clocks
    SYSTEMConfigPerformance(80000000);

    // 2. Initialize I/O pins
    AD1PCFG = 0xFFFF;  // All pins digital
    mInitAllLEDs();

    // 3. Configure UARTs
    Init_UART1_BACnet();
    Init_UART2_Modbus();

    // 4. Setup timers
    Init_Timer1();  // 1ms system tick

    // 5. Configure interrupts
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableSystemMultiVectoredInt();

    // 6. Initialize protocol stacks
    BACnet_Init();
    Modbus_Init();

    // 7. Enable watchdog
    ENABLE_WDT();
}
```

---

## Power Management

### Operating Conditions
- **Supply Voltage**: 3.3V (regulated from 5V USB or external supply)
- **Current Consumption**:
  - Active: ~95mA @ 80MHz
  - Sleep: ~30μA (if implemented)
- **Temperature Range**: -40°C to +85°C (industrial)

### Power-On Reset
```c
// Check reset cause
uint32_t resetCause = RCON;
if(resetCause & _RCON_POR_MASK) {
    // Power-on reset
}
if(resetCause & _RCON_BOR_MASK) {
    // Brown-out reset
}
if(resetCause & _RCON_WDTO_MASK) {
    // Watchdog timeout
}
RCON = 0;  // Clear reset status
```

---

## Debug Support

### JTAG Interface
```c
// Disable JTAG for production (frees up pins)
DDPCONbits.JTAGEN = 0;
```

### Debug UART (Optional)
- Can use USB CDC class for debug output
- Or dedicated UART3 if pins available

### Timing Analysis Points
```c
// Key timing measurement points for oscilloscope
// 1. Token rotation time
// 2. Frame gap timing
// 3. Interrupt latency
// 4. RS-485 turnaround time
```

---

## Known Hardware Limitations

1. **Single-ended RS-485**: No redundant bus support
2. **Fixed Modbus baud rate**: 19200 baud only for Modbus side
3. **No hardware flow control**: CTS/RTS used for RS-485 DE
4. **Limited buffering**: 256-byte FIFOs in software
5. **No DMA**: All transfers are interrupt-driven

---

## Compiler-Specific Considerations (XC32 v4.60)

1. **Interrupt syntax**: Use `__ISR()` macro with IPL suffix
2. **Configuration bits**: Use `#pragma config` directives
3. **Peripheral libraries**: Use compatibility layer (plib_compat.h)
4. **Optimization**: Start with -O0, validate timing, then optimize
5. **Stack size**: Minimum 16KB for BACnet stack operations

---

## References

- [PIC32MX795F512H Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/60001156K.pdf)
- [Olimex PIC32-RETROBSD Schematic](https://www.olimex.com/Products/Duino/PIC32/PIC32-RETROBSD/resources/PIC32-RETROBSD-Rev-A.pdf)
- [RS-485 Design Guide](https://www.ti.com/lit/an/slla272c/slla272c.pdf)
- [BACnet MS/TP Physical Layer Specification](https://www.ashrae.org/technical-resources/bookstore/bacnet)

---

*End of BSP Documentation*
