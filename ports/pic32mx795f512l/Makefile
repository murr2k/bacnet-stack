# Makefile for BACnet-stack PIC32MX795F512L port
# Author: Murray Kopit <murr2k@gmail.com>
# Date: 2025

# Tools
CC = xc32-gcc
AR = xc32-ar
LD = xc32-ld
OBJCOPY = xc32-objcopy
SIZE = xc32-size

# Target MCU
MCU = 32MX795F512H

# Build directory
BUILD_DIR = build

# BACnet-stack root directory
BACNET_ROOT = ../..

# Compiler flags
CFLAGS = -mprocessor=$(MCU)
CFLAGS += -g -O1
CFLAGS += -Wall -Wextra
CFLAGS += -std=c99
CFLAGS += -ffunction-sections -fdata-sections

# Define symbols
DEFINES = -DBACDL_MSTP
DEFINES += -DMSTP_PDU_DELAY
DEFINES += -DMAX_APDU=480
DEFINES += -DMAX_TSM_TRANSACTIONS=1
DEFINES += -DMAX_CHARACTER_STRING_BYTES=64
DEFINES += -DMAX_OCTET_STRING_BYTES=64
DEFINES += -DBACAPP_MINIMAL
DEFINES += -DPRINT_ENABLED=0
DEFINES += -DBACNET_PROTOCOL_REVISION=14

# Include paths
INCLUDES = -I.
INCLUDES += -I$(BACNET_ROOT)/include
INCLUDES += -I$(BACNET_ROOT)/src

# Linker flags
LDFLAGS = -mprocessor=$(MCU)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/bacnet.map
LDFLAGS += -Wl,--defsym,_min_heap_size=2048
LDFLAGS += -Wl,--defsym,_min_stack_size=2048

# Source files - Port specific
PORT_SRCS = \
	main.c \
	rs485.c \
	dlmstp.c \
	device.c \
	ai.c \
	ao.c \
	bi.c \
	bo.c \
	stubs.c

# Source files - BACnet core
BACNET_CORE_SRCS = \
	$(BACNET_ROOT)/src/bacnet/abort.c \
	$(BACNET_ROOT)/src/bacnet/bacaddr.c \
	$(BACNET_ROOT)/src/bacnet/bacdcode.c \
	$(BACNET_ROOT)/src/bacnet/bacerror.c \
	$(BACNET_ROOT)/src/bacnet/bacint.c \
	$(BACNET_ROOT)/src/bacnet/bacreal.c \
	$(BACNET_ROOT)/src/bacnet/bacstr.c \
	$(BACNET_ROOT)/src/bacnet/bacapp.c \
	$(BACNET_ROOT)/src/bacnet/bacprop.c \
	$(BACNET_ROOT)/src/bacnet/bactext.c \
	$(BACNET_ROOT)/src/bacnet/indtext.c \
	$(BACNET_ROOT)/src/bacnet/datetime.c \
	$(BACNET_ROOT)/src/bacnet/dcc.c \
	$(BACNET_ROOT)/src/bacnet/iam.c \
	$(BACNET_ROOT)/src/bacnet/npdu.c \
	$(BACNET_ROOT)/src/bacnet/reject.c \
	$(BACNET_ROOT)/src/bacnet/rp.c \
	$(BACNET_ROOT)/src/bacnet/wp.c \
	$(BACNET_ROOT)/src/bacnet/whois.c

# Source files - BACnet services
BACNET_SVC_SRCS = \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_apdu.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_dcc.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_iam.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_noserv.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_rd.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_rp.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_rpm.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_whois.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/h_wp.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/s_iam.c \
	$(BACNET_ROOT)/src/bacnet/basic/service/s_whois.c

# Source files - BACnet datalink
BACNET_DL_SRCS = \
	$(BACNET_ROOT)/src/bacnet/datalink/crc.c \
	$(BACNET_ROOT)/src/bacnet/datalink/cobs.c \
	$(BACNET_ROOT)/src/bacnet/datalink/mstp.c \
	$(BACNET_ROOT)/src/bacnet/datalink/mstptext.c

# Source files - BACnet system
BACNET_SYS_SRCS = \
	$(BACNET_ROOT)/src/bacnet/basic/sys/fifo.c \
	$(BACNET_ROOT)/src/bacnet/basic/sys/ringbuf.c

# Source files - BACnet TSM
BACNET_TSM_SRCS = \
	$(BACNET_ROOT)/src/bacnet/basic/tsm/tsm.c

# Source files - BACnet binding
BACNET_BIND_SRCS = \
	$(BACNET_ROOT)/src/bacnet/basic/binding/address.c

# All source files
SRCS = $(PORT_SRCS) $(BACNET_CORE_SRCS) $(BACNET_SVC_SRCS) \
	   $(BACNET_DL_SRCS) $(BACNET_SYS_SRCS) $(BACNET_TSM_SRCS) \
	   $(BACNET_BIND_SRCS)

# Object files
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Target
TARGET = $(BUILD_DIR)/bacnet.elf
TARGET_HEX = $(BUILD_DIR)/bacnet.hex

# Default target
all: $(TARGET_HEX)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(BACNET_ROOT)/src/bacnet
	mkdir -p $(BUILD_DIR)/$(BACNET_ROOT)/src/bacnet/basic/service
	mkdir -p $(BUILD_DIR)/$(BACNET_ROOT)/src/bacnet/basic/sys
	mkdir -p $(BUILD_DIR)/$(BACNET_ROOT)/src/bacnet/basic/tsm
	mkdir -p $(BUILD_DIR)/$(BACNET_ROOT)/src/bacnet/basic/binding
	mkdir -p $(BUILD_DIR)/$(BACNET_ROOT)/src/bacnet/datalink

# Compile source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

# Link
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) $@

# Create hex file
$(TARGET_HEX): $(TARGET)
	$(OBJCOPY) -O ihex $< $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Program target (requires PICkit or ICD)
program: $(TARGET_HEX)
	@echo "Programming with PICkit3/ICD3..."
	@echo "Run: pic32prog -d /dev/ttyUSB0 $(TARGET_HEX)"

.PHONY: all clean program
