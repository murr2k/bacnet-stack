# PIC32MX795F512L BACnet MS/TP Port

![PIC32MX ABI Compatibility](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/murr2k/d9e183fda10aaa641ddb6ec578293727/raw/pic32mx-abi-badge.json)

## Overview

This is a BACnet MS/TP implementation for the PIC32MX795F512L microcontroller, designed for the Olimex PIC32-RETROBSD development board. The port provides a complete BACnet MS/TP stack implementation with RS-485 communication support.

## Features

- **Full BACnet MS/TP Implementation** - Complete datalink layer with master node capability
- **RS-485 Hardware Support** - UART1 with automatic driver enable control
- **Basic Object Types** - AI, AO, BI, BO, and Device objects
- **100% API Compatibility** - Binary compatible with BACnet-stack headers
- **Comprehensive Testing** - Full ABI compatibility test suite included

## Hardware Requirements

- PIC32MX795F512L microcontroller (or compatible)
- Olimex PIC32-RETROBSD board (or equivalent)
- RS-485 transceiver (e.g., MAX485)
- 80MHz system clock configuration

## Pin Configuration

| Function | Pin | Description |
|----------|-----|-------------|
| UART1 RX | RF2 | RS-485 Receive |
| UART1 TX | RF8 | RS-485 Transmit |
| RS-485 DE | RD5 | Driver Enable |
| LED1 | RD6 | Status LED (Red) |
| LED2 | RD7 | Status LED (Yellow) |
| LED3 | RD8 | Status LED (Green) |

## Building

### Prerequisites

- Microchip XC32 compiler (v4.35 or later)
- Make utility
- BACnet-stack source tree

### Build Instructions

```bash
# From the port directory
cd ports/pic32mx795f512l

# Build the port
make

# The output will be in build/bacnet.elf and build/bacnet.hex
```

### Configuration

Edit `hardware.h` to modify:
- System clock frequency
- Pin assignments
- FIFO buffer sizes
- Interrupt priorities

## Testing

The port includes comprehensive ABI compatibility testing:

```bash
# Run ABI compatibility tests
cd test
make -f Makefile.host test

# View the compatibility report
cat ABI_COMPATIBILITY_REPORT.md
```

## MS/TP Configuration

Default MS/TP settings:
- **Baud Rate**: 19200 bps
- **MAC Address**: 1
- **Max Master**: 127
- **Max Info Frames**: 1

These can be modified at runtime using the dlmstp API functions.

## Memory Usage

Typical resource usage:
- **Flash**: ~60KB (including BACnet stack)
- **RAM**: ~8KB (including buffers)
- **Stack**: ~2KB

## Documentation

- [Board Support Package](BOARD_SUPPORT_PACKAGE.md) - Detailed hardware documentation
- [Change Log](CHANGELOG.md) - Version history and changes
- [Test Documentation](test/README.md) - Testing procedures and results

## Example Usage

```c
#include "dlmstp.h"
#include "device.h"

int main(void) {
    // Initialize hardware
    hardware_init();

    // Initialize MS/TP
    dlmstp_init(NULL);
    dlmstp_set_mac_address(1);
    dlmstp_set_baud_rate(19200);

    // Initialize Device
    Device_Init(NULL);

    // Main loop
    while (1) {
        dlmstp_task();
        // Handle other tasks
    }

    return 0;
}
```

## License

This port is released under the MIT License, consistent with the BACnet-stack project.

## Author

Murray Kopit <murr2k@gmail.com>

## Contributing

Contributions are welcome! Please ensure all changes maintain ABI compatibility by running the test suite before submitting pull requests.