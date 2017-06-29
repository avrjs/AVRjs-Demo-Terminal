# Copyright 2017 Julian Ingram
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

MCU ?= attiny1634

DEFINES :=

CC := avr-gcc
OBJCOPY := avr-objcopy
CFLAGS += -O0 -Werror -Wall -Wextra $(DEFINES:%=-D%) -mmcu=$(MCU) -std=c11
# expanded below
DEPFLAGS = -MMD -MP -MF $(@:$(BUILD_DIR)/%.o=$(DEP_DIR)/%.d)
LDFLAGS := -O0 -mmcu=$(MCU)
SRCS := mcu_term.c avrjs_uart.c avrjs_term.c
BIN_DIR ?= bin
TARGET ?= $(BIN_DIR)/avrjs_term_$(MCU).elf
TARGET_HEX ?= $(BIN_DIR)/avrjs_term_$(MCU).hex
RM := rm -rf
MKDIR := mkdir -p
# BUILD_DIR and DEP_DIR should both have non-empty values
BUILD_DIR ?= build
DEP_DIR ?= $(BUILD_DIR)/deps
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(SRCS:%.c=$(DEP_DIR)/%.d)

.PHONY: all
all: $(TARGET)

# link
$(TARGET): $(OBJS)
	$(if $(BIN_DIR),$(MKDIR) $(BIN_DIR),)
	$(CC) -o $@ $^ $(LDFLAGS)
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET) $(TARGET_HEX)

# compile and/or generate dep files
$(BUILD_DIR)/%.o: %.c
	$(MKDIR) $(BUILD_DIR)/$(dir $<)
	$(MKDIR) $(DEP_DIR)/$(dir $<)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(BIN_DIR) $(DEP_DIR) $(BUILD_DIR)

-include $(DEPS)
