# Top-level Makefile for building the OS and disk image

.PHONY: all clean build_vbr

# =========== Configuration ===========
# Tools
MFORMAT := mformat
MCOPY := mcopy
MMD := mmd
DD := dd
MKFS_EXT2 := /usr/sbin/mkfs.ext2
SFDISK  := /usr/sbin/sfdisk

# Output image file
IMAGE_NAME := os.img
IMAGE_SIZE_MB := 450

# Partition config
BOOT_PART_MB := 48
ALIGNMENT_MB := 1
SECTOR_SIZE  := 512

# Paths to sub-projects and their outputs
VBR_DIR := bootloader
VBR_BIN := $(VBR_DIR)/boot.bin

BOOT_DIR := boot
BOOT_SYS := $(BOOT_DIR)/out/boot.sys

# Intermediate partition files
PART_BOOT_IMG := boot_part.img
PART_ROOT_IMG := root_part.img

# Try to find a generic MBR binary (Syslinux is common)
# If not found, the disk will have a partition table but no code to jump to the partition.
MBR_BIN := /usr/lib/syslinux/mbr/mbr.bin
ifeq ($(wildcard $(MBR_BIN)),)
	MBR_BIN := /usr/lib/syslinux/mbr.bin
endif


# =========== Main Targets ===========

# Default target: build everything and create the disk image
all: $(IMAGE_NAME)

# 3. Create the Final Disk Image
$(IMAGE_NAME): $(VBR_BIN) $(BOOT_SYS)
	@echo "Creating disk image $(IMAGE_NAME) [$(IMAGE_SIZE_MB)MB]..."

# --- Step 1: Create the FAT32 Boot Partition Image ---
	@echo "Creating FAT32 Boot Partition ($(BOOT_PART_MB)MB)..."
	$(DD) if=/dev/zero of=$(PART_BOOT_IMG) bs=1M count=$(BOOT_PART_MB) status=none
	
# Format as FAT32. 
# -H 2048: IMPORTANT! Tells the VBR it is located at sector 2048 (1MB offset).
# -F: Force FAT32 (since 48MB is small, mformat might default to FAT16 otherwise).
	$(MFORMAT) -i $(PART_BOOT_IMG) -F -H 2048 -B $(VBR_BIN) -v "LINUX_BOOT" ::

# Copy files
	$(MCOPY) -i $(PART_BOOT_IMG) $(BOOT_SYS) ::/BOOT.SYS
	$(MCOPY) -i $(PART_BOOT_IMG) zImage ::/zimage

# --- Step 2: Create the EXT2 Root Partition Image ---
	@echo "Creating EXT2 root partition..."
# Calculate size: Total - Boot - Alignment (approx)
# We just create a file for the remainder.
	$(DD) if=/dev/zero of=$(PART_ROOT_IMG) bs=1M count=$$(($(IMAGE_SIZE_MB) - $(BOOT_PART_MB) - $(ALIGNMENT_MB))) status=none
	
# Format as ext2 (force file target with -F)
	$(MKFS_EXT2) -F -q -L "LINUX_ROOT" $(PART_ROOT_IMG) -I 128

# --- Step 3: Create the physical disk image ---
	@echo "   -> Assembling physical disk..."
	$(DD) if=/dev/zero of=$(IMAGE_NAME) bs=1M count=$(IMAGE_SIZE_MB) status=none

# Create partition table (MBR)
# Partition 1: FAT32 (Type 0x0c), bootable, starts at 1MB (sector 2048)
# Partition 2: Linux (Type 0x83), starts after part 1
	@echo "label: dos" > partition.script
	@echo "unit: sectors" >> partition.script
	@echo "start=2048, size=$$(($(BOOT_PART_MB) * 1024 * 1024 / 512)), type=0c, bootable" >> partition.script
	@echo "start=$$(($(BOOT_PART_MB) * 1024 * 1024 / 512 + 2048)), type=83" >> partition.script
	$(SFDISK) $(IMAGE_NAME) < partition.script
	rm partition.script

# --- Step D: Install MBR Code ---
	@if [ -f "$(MBR_BIN)" ]; then \
		echo "Installing MBR code from $(MBR_BIN)..."; \
		$(DD) if=$(MBR_BIN) of=$(IMAGE_NAME) bs=440 count=1 conv=notrunc status=none; \
	else \
		echo "[WARNING] No MBR binary found at /usr/lib/syslinux/mbr/mbr.bin."; \
		echo "The image has a partition table but no code to boot it."; \
		echo "Install 'syslinux-common' or 'mbr' package, or write your own MBR to the first 440 bytes."; \
	fi

# --- Step E: Write Partitions to Disk Image ---
# Write boot partition at 1MB (2048 * 512 = 1048576 bytes)
	$(DD) if=$(PART_BOOT_IMG) of=$(IMAGE_NAME) bs=1M seek=$(ALIGNMENT_MB) conv=notrunc status=none
	
# Write root partition after boot partition
	$(DD) if=$(PART_ROOT_IMG) of=$(IMAGE_NAME) bs=1M seek=$$(($(ALIGNMENT_MB) + $(BOOT_PART_MB))) conv=notrunc status=none
	
# Clean
	rm -f $(PART_BOOT_IMG) $(PART_ROOT_IMG)

	@echo "Done! Created $(IMAGE_NAME)"
	@echo "  Partition 1 (FAT32): Offset 1MB (Sector 2048)"
	@echo "  Partition 2 (EXT2):  Offset $$(($(ALIGNMENT_MB) + $(BOOT_PART_MB)))MB"

# =========== Sub-Project Build Rules ===========

$(VBR_BIN): build_vbr ;
$(BOOT_SYS): build_boot ;

build_vbr:
	@echo "Building VBR in $(VBR_DIR)..."
	$(MAKE) -C $(VBR_DIR)

build_boot:
	@echo "Building BOOT in $(BOOT_DIR)..."
	$(MAKE) -C $(BOOT_DIR)



# =========== Clean Target ===========

clean:
	$(MAKE) -C $(VBR_DIR) clean
	$(MAKE) -C $(BOOT_DIR) clean
	rm -f $(IMAGE_NAME) $(PART_BOOT_IMG) $(PART_ROOT_IMG) $(VBR_BIN)
	@echo "All clean."
