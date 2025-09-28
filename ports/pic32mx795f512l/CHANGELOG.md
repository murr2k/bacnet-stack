# Changelog

All notable changes to the PIC32MX795F512L BACnet MS/TP Port will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive ABI (Application Binary Interface) compatibility testing framework
- Automated CI/CD pipeline for GitHub Actions
- Binary compatibility verification with BACnet-stack API
- Host compiler build support for testing (TEST_MODE)

### Changed
- Updated hardware.h with TEST_MODE guards for cross-platform testing
- Enhanced RS-485 driver with TEST_MODE stubs for CI/CD compatibility

### Fixed
- Resolved linking issues with missing symbols in test builds
- Fixed millisecond_counter and timer dependencies for test mode

## [1.0.0] - 2025-01-27

### Added
- Initial release of PIC32MX795F512L BACnet MS/TP port
- Full BACnet MS/TP (Master-Slave/Token-Passing) protocol implementation
- RS-485 half-duplex communication driver with hardware flow control
- Support for four BACnet object types:
  - Analog Input (AI) - 4 instances with COV support
  - Analog Output (AO) - 2 instances with priority array
  - Binary Input (BI) - 4 instances with change detection
  - Binary Output (BO) - 4 instances with feedback and priority array
- Device object with full property support
- Hardware abstraction layer for Olimex PIC32-RETROBSD board
- UART1 interrupt-driven communication at configurable baud rates
- FIFO buffering for reliable data transmission
- Comprehensive error handling and recovery mechanisms
- System tick timer at 1ms resolution
- Watchdog timer support for system reliability
- LED indicators for system status and debugging
- Flash-based non-volatile configuration storage

### Implemented BACnet Services
- Who-Is / I-Am device discovery
- Read Property / Read Property Multiple
- Write Property / Write Property Multiple
- Change of Value (COV) notifications
- Device Communication Control
- Reinitialize Device
- Time Synchronization

### Hardware Features
- **Microcontroller**: PIC32MX795F512L (80MHz, 512KB Flash, 128KB RAM)
- **Communication**: RS-485 via UART1 with automatic direction control
- **Pins Configuration**:
  - RF2: UART1 RX (RS-485 receive)
  - RF8: UART1 TX (RS-485 transmit)
  - RD5: RS-485 driver enable (DE/~RE)
  - RD12: CTS (optional flow control)
- **Status LEDs**:
  - RD6: Red LED (error/fault indication)
  - RD7: Yellow LED (communication activity)
  - RD8: Green LED (system running)
- **User Interface**:
  - RD0: User button
  - RD1: Program/mode button

### Build System
- MPLAB X IDE project configuration
- XC32 compiler support (v2.50 or later)
- Makefile for command-line building
- Configurable optimization levels
- Debug and release build configurations
- Automated dependency management

### Testing Infrastructure
- Test framework with assertion macros
- Unit tests for all object types
- MS/TP protocol verification
- ABI compatibility validation
- Automated regression testing
- Code coverage reporting (via test_framework.h)

### Documentation
- Comprehensive API documentation
- Hardware integration guide
- Build and deployment instructions
- Troubleshooting guide
- Example applications

### Performance Characteristics
- MS/TP token rotation: <10ms typical
- Object property access: <1ms
- COV notification latency: <50ms
- Maximum throughput: 76.8 kbps at 76800 baud
- RAM usage: ~32KB typical
- Flash usage: ~128KB including BACnet stack

### Compliance
- BACnet Protocol Revision: 14
- Conformance Class: B-ASC (BACnet Application Specific Controller)
- BIBBS Supported:
  - DS-RP-B (Data Sharing - Read Property - B)
  - DS-WP-B (Data Sharing - Write Property - B)
  - DM-DDB-B (Device Management - Dynamic Device Binding - B)
  - DM-DOB-B (Device Management - Dynamic Object Binding - B)

### Known Limitations
- Maximum 4 instances per object type (configurable)
- MS/TP addresses limited to 0-127
- Maximum APDU size: 480 bytes
- Single MS/TP port support
- No routing or gateway functionality

### Development Tools Required
- MPLAB X IDE v5.50 or later
- XC32 Compiler v2.50 or later
- PICkit 3/4 or ICD3/4 programmer
- RS-485 to USB converter for testing
- Terminal emulator for debugging output

### License
- MIT License
- Copyright (c) 2025 Murray Kopit

---

## Migration Guide

### From Other PIC32 Ports
1. Update pin assignments in hardware.h
2. Adjust timer configurations for your clock speed
3. Modify UART settings for your RS-485 transceiver
4. Update LED and button pin definitions

### From Other Microcontroller Families
1. Replace hardware-specific includes
2. Implement timer and UART abstraction functions
3. Adapt interrupt handlers to your architecture
4. Update memory configuration in linker script

---

## Roadmap

### Version 1.1.0 (Planned)
- [ ] Add support for Trend Log objects
- [ ] Implement Schedule objects
- [ ] Add BACnet/IP support alongside MS/TP
- [ ] Enhance error diagnostics and logging

### Version 1.2.0 (Future)
- [ ] Multi-state Input/Output objects
- [ ] Notification Class objects
- [ ] Event Enrollment objects
- [ ] Alarm and event handling

### Version 2.0.0 (Long-term)
- [ ] BACnet Secure Connect (BACnet/SC)
- [ ] Cloud connectivity options
- [ ] Web-based configuration interface
- [ ] OTA firmware updates

---

## Support and Contribution

### Reporting Issues
Please report bugs and feature requests at:
https://github.com/murr2k/bacnet-stack/issues

### Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

### Contact
Murray Kopit <murr2k@gmail.com>

---

[Unreleased]: https://github.com/murr2k/bacnet-stack/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/murr2k/bacnet-stack/releases/tag/pic32mx-v1.0.0
