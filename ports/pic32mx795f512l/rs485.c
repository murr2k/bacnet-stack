/**
 * @file
 * @brief RS-485 Interface for PIC32MX795F512L BACnet MS/TP
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#ifndef TEST_MODE
#include <xc.h>
#include <sys/attribs.h>
#endif
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "hardware.h"
#include "rs485.h"
#include "bacnet/datalink/dlmstp.h"
#include "bacnet/basic/sys/fifo.h"

/* FIFO buffers for receive and transmit */
static volatile FIFO_BUFFER rx_fifo;
static volatile uint8_t rx_fifo_buffer[BACNET_RX_FIFO_SIZE];
static volatile uint32_t rx_timestamp[BACNET_RX_FIFO_SIZE];  /* Store timestamps separately */
static volatile uint16_t rx_timestamp_head = 0;
static volatile uint16_t rx_timestamp_tail = 0;

static volatile uint8_t tx_buffer[BACNET_TX_FIFO_SIZE];
static volatile uint16_t tx_count = 0;
static volatile uint16_t tx_pos = 0;

/* Statistics */
static volatile uint32_t rx_bytes = 0;
static volatile uint32_t tx_bytes = 0;

/* RS-485 state */
static volatile bool transmitting = false;
extern volatile uint32_t millisecond_counter;

#ifdef TEST_MODE
/* Test mode stubs - no hardware access */

void RS485_Initialize(void)
{
    /* Initialize FIFO */
    FIFO_Init((FIFO_BUFFER *)&rx_fifo, (volatile uint8_t *)rx_fifo_buffer, sizeof(rx_fifo_buffer));
    transmitting = false;
}

void RS485_Set_Baud_Rate(uint32_t baud)
{
    /* Stub - no hardware to configure */
    (void)baud;
}

bool RS485_Transmit_Ready(void)
{
    return !transmitting;
}

void RS485_Send_Frame(uint8_t *buffer, uint16_t nbytes)
{
    uint16_t i;

    if (nbytes == 0 || nbytes > BACNET_TX_FIFO_SIZE) {
        return;
    }

    /* Copy data to transmit buffer */
    for (i = 0; i < nbytes; i++) {
        tx_buffer[i] = buffer[i];
    }
    tx_count = nbytes;
    tx_pos = 0;
    tx_bytes += nbytes;

    /* Simulate immediate transmission */
    transmitting = false;
}

bool RS485_DataAvailable(void)
{
    return !FIFO_Empty((FIFO_BUFFER *)&rx_fifo);
}

bool RS485_Receive(uint8_t *data_byte, uint32_t *timestamp)
{
    bool status = false;

    if (!FIFO_Empty((FIFO_BUFFER *)&rx_fifo)) {
        /* Get data byte from FIFO */
        *data_byte = FIFO_Get((FIFO_BUFFER *)&rx_fifo);

        /* Get corresponding timestamp */
        if (timestamp) {
            *timestamp = rx_timestamp[rx_timestamp_tail];
        }
        rx_timestamp_tail = (rx_timestamp_tail + 1) % BACNET_RX_FIFO_SIZE;

        status = true;
    }

    return status;
}

bool RS485_ReceiveError(void)
{
    /* No errors in test mode */
    return false;
}

void RS485_Get_Statistics(uint32_t *p_rx_bytes, uint32_t *p_tx_bytes)
{
    if (p_rx_bytes) {
        *p_rx_bytes = rx_bytes;
    }
    if (p_tx_bytes) {
        *p_tx_bytes = tx_bytes;
    }
}

void RS485_Clear_Statistics(void)
{
    rx_bytes = 0;
    tx_bytes = 0;
}

#else  /* !TEST_MODE - Real hardware implementation */

/**
 * @brief Initialize RS-485 interface
 */
void RS485_Initialize(void)
{
    /* Initialize FIFO */
    FIFO_Init((FIFO_BUFFER *)&rx_fifo, (volatile uint8_t *)rx_fifo_buffer, sizeof(rx_fifo_buffer));

    /* Configure RS-485 driver enable pin (RD5) */
    LATDCLR = U1RTS_PIN;   // Disable transmitter initially
    TRISDCLR = U1RTS_PIN;  // Set as output

    /* Configure UART1 pins */
    /* RF2 = U1RX (input) */
    TRISFSET = U1RX_PIN;
    /* RF8 = U1TX (output) */
    TRISFCLR = U1TX_PIN;

    /* Map UART1 pins using PPS (Peripheral Pin Select) */
    /* Note: PIC32MX795F512L has fixed UART1 pins, not remappable */
    /* U1RX is on RF2, U1TX is on RF8 - these are fixed */

    /* Configure UART1 */
    U1MODE = 0;             // Clear mode register
    U1STA = 0;              // Clear status register

    /* Configure for 8N1 at 19200 baud (default) */
    RS485_Set_Baud_Rate(19200);

    U1MODEbits.PDSEL = 0;  // 8-bit data, no parity
    U1MODEbits.STSEL = 0;  // 1 stop bit
    U1MODEbits.BRGH = 1;   // High-speed mode

    /* Configure interrupts */
    IPC6SET = (_IPC6_U1IP_MASK & (UART1_PRIORITY << _IPC6_U1IP_POSITION)) |
              (_IPC6_U1IS_MASK & (0 << _IPC6_U1IS_POSITION));

    IFS0CLR = _IFS0_U1RXIF_MASK | _IFS0_U1TXIF_MASK;  // Clear flags

    /* Enable RX interrupt, TX interrupt enabled when transmitting */
    IEC0SET = _IEC0_U1RXIE_MASK;

    /* Enable UART */
    U1STAbits.URXEN = 1;   // Enable receiver
    U1STAbits.UTXEN = 1;   // Enable transmitter
    U1MODEbits.ON = 1;     // Enable UART
}

/**
 * @brief Set RS-485 baud rate
 */
void RS485_Set_Baud_Rate(uint32_t baud)
{
    uint32_t brg_value;

    /* Calculate BRG value for high-speed mode */
    brg_value = ((GetPeripheralClock() / (4 * baud)) - 1);

    U1BRG = brg_value;
}

/**
 * @brief Check if RS-485 is ready to transmit
 */
bool RS485_Transmit_Ready(void)
{
    return !transmitting;
}

/**
 * @brief Send frame via RS-485
 */
void RS485_Send_Frame(uint8_t *buffer, uint16_t nbytes)
{
    uint16_t i;

    if (nbytes == 0 || nbytes > BACNET_TX_FIFO_SIZE) {
        return;
    }

    /* Wait for any previous transmission to complete */
    while (transmitting) {
        /* Timeout after 100ms */
        /* In production, add proper timeout handling */
    }

    /* Copy data to transmit buffer */
    for (i = 0; i < nbytes; i++) {
        tx_buffer[i] = buffer[i];
    }
    tx_count = nbytes;
    tx_pos = 0;
    tx_bytes += nbytes;

    /* Enable transmitter */
    transmitting = true;
    ENA_BACNET;

    /* Small delay for driver enable setup */
    DelayUs(10);

    /* Enable TX interrupt to start transmission */
    IEC0SET = _IEC0_U1TXIE_MASK;

    /* Send first byte to trigger TX interrupt */
    if (tx_count > 0) {
        U1TXREG = tx_buffer[tx_pos++];
        tx_count--;
    }
}

/**
 * @brief Check if byte received
 */
bool RS485_DataAvailable(void)
{
    return !FIFO_Empty((FIFO_BUFFER *)&rx_fifo);
}

/**
 * @brief Get received byte
 */
bool RS485_Receive(uint8_t *data_byte, uint32_t *timestamp)
{
    bool status = false;

    if (!FIFO_Empty((FIFO_BUFFER *)&rx_fifo)) {
        /* Get data byte from FIFO */
        *data_byte = FIFO_Get((FIFO_BUFFER *)&rx_fifo);

        /* Get corresponding timestamp */
        if (timestamp) {
            *timestamp = rx_timestamp[rx_timestamp_tail];
        }
        rx_timestamp_tail = (rx_timestamp_tail + 1) % BACNET_RX_FIFO_SIZE;

        status = true;
    }

    return status;
}

/**
 * @brief Check receive errors
 */
bool RS485_ReceiveError(void)
{
    bool error = false;

    /* Check for overrun error */
    if (U1STAbits.OERR) {
        U1STAbits.OERR = 0;  // Clear error
        error = true;
    }

    /* Check for framing error */
    if (U1STAbits.FERR) {
        /* Read and discard byte to clear error */
        volatile uint8_t dummy = U1RXREG;
        (void)dummy;
        error = true;
    }

    /* Check for parity error (not used in 8N1) */
    if (U1STAbits.PERR) {
        /* Read and discard byte to clear error */
        volatile uint8_t dummy = U1RXREG;
        (void)dummy;
        error = true;
    }

    return error;
}

/**
 * @brief UART1 interrupt handler
 */
void __ISR(_UART1_VECTOR, IPL5AUTO) UART1Handler(void)
{
    uint8_t data;

    /* RX interrupt */
    if (IFS0bits.U1RXIF) {
        IFS0CLR = _IFS0_U1RXIF_MASK;

        while (U1STAbits.URXDA) {
            data = U1RXREG;
            rx_bytes++;

            /* Store data in FIFO */
            FIFO_Put((FIFO_BUFFER *)&rx_fifo, data);

            /* Store timestamp */
            rx_timestamp[rx_timestamp_head] = millisecond_counter;
            rx_timestamp_head = (rx_timestamp_head + 1) % BACNET_RX_FIFO_SIZE;

            /* Signal data received for MS/TP */
            dlmstp_rs485_rx_event();
        }
    }

    /* TX interrupt */
    if (IFS0bits.U1TXIF) {
        IFS0CLR = _IFS0_U1TXIF_MASK;

        /* Send next byte if available */
        if (tx_count > 0) {
            U1TXREG = tx_buffer[tx_pos++];
            tx_count--;
        } else {
            /* Transmission complete */
            IEC0CLR = _IEC0_U1TXIE_MASK;  // Disable TX interrupt

            /* Wait for transmit shift register to empty */
            while (!U1STAbits.TRMT);

            /* Small hold time before disabling driver */
            DelayUs(10);

            /* Disable transmitter */
            DIS_BACNET;
            transmitting = false;
        }
    }

    /* Error interrupt */
    if (IFS0bits.U1EIF) {
        IFS0CLR = _IFS0_U1EIF_MASK;
        RS485_ReceiveError();
    }
}

/**
 * @brief Get RS-485 statistics
 */
void RS485_Get_Statistics(uint32_t *p_rx_bytes, uint32_t *p_tx_bytes)
{
    if (p_rx_bytes) {
        *p_rx_bytes = rx_bytes;
    }
    if (p_tx_bytes) {
        *p_tx_bytes = tx_bytes;
    }
}

/**
 * @brief Clear RS-485 statistics
 */
void RS485_Clear_Statistics(void)
{
    rx_bytes = 0;
    tx_bytes = 0;
}

#endif /* TEST_MODE */