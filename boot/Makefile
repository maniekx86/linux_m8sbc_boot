TOOLCHAIN_PATH ?= /home/piotrek/Pulpit/dev/486/asm/clang/i686-elf-gcc/bin
CC := $(TOOLCHAIN_PATH)/i686-elf-gcc
OBJDUMP := $(TOOLCHAIN_PATH)/i686-elf-objdump
OBJCOPY := $(TOOLCHAIN_PATH)/i686-elf-objcopy
LD := $(TOOLCHAIN_PATH)/i686-elf-ld
NASM := nasm

TMP_DIR := tmp
OUT_DIR := out

CFLAGS := -ffreestanding -march=i486 -mtune=i486 -nostdlib -nodefaultlibs -m32 -g -Os -I../common/stdlibs -I../common/syslibs -I../common/fat32
LDFLAGS := -T linker.ld -m elf_i386 -nostdlib

SOURCES_C := main.c ../common/fat32/fat.c ../common/stdlibs/string.c ../common/syslibs/ide.c ../common/stdlibs/stdlib.c
SOURCES_ASM := crt0.s

# hack to generate object file paths in TMP_DIR even if they include ../ in their path
# (using VPATH would compile everything in the source dirs, which we don't need)
define obj_path
$(TMP_DIR)/$(patsubst ../%,%,$(basename $(1))).o
endef

define c_rule
$(call obj_path,$(1)): $(1)
	@mkdir -p $$(@D)
	$(CC) $(CFLAGS) -c $$< -o $$@
endef

define asm_rule
$(call obj_path,$(1)): $(1)
	@mkdir -p $$(@D)
	$(NASM) -f elf32 $$< -o $$@
endef

OBJS_C := $(foreach src,$(SOURCES_C),$(call obj_path,$(src)))
OBJS_ASM := $(foreach src,$(SOURCES_ASM),$(call obj_path,$(src)))
OBJS := $(OBJS_ASM) $(OBJS_C)

.PHONY: all clean

all: $(OUT_DIR)/boot.sys

$(OUT_DIR)/boot.sys: $(OUT_DIR)/kernel.elf
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $< $@

$(OUT_DIR)/kernel.elf: $(OBJS)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -o $@ $^

$(foreach src,$(SOURCES_C),$(eval $(call c_rule,$(src))))
$(foreach src,$(SOURCES_ASM),$(eval $(call asm_rule,$(src))))

clean:
	rm -rf $(TMP_DIR) $(OUT_DIR)