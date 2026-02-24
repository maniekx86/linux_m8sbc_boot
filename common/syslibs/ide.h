#ifndef _IDE_H
#define _IDE_H

#include <stdint.h>
#include <stdbool.h>
#include <x86_io.h>

#define IDE_BASE     0x1F0

#define IDE_DATA    IDE_BASE + 0x00
#define IDE_ERR     IDE_BASE + 0x01
#define IDE_SECNUM  IDE_BASE + 0x02
#define IDE_LBA0    IDE_BASE + 0x03
#define IDE_LBA1    IDE_BASE + 0x04
#define IDE_LBA2    IDE_BASE + 0x05
#define IDE_LBA3    IDE_BASE + 0x06
#define IDE_STATUS  IDE_BASE + 0x07

#define IDE_DRQ_TIMEOUT 5
#define IDE_RDY_TIMEOUT 5

/// @brief IDE reset (command 0x08)
void ide_reset();

/// @brief Returns drive identification block (command 0xEC)
/// @param buffer 512 byte / 256 word buffer for IDE ident block
/// @return True on success, false on timeout
bool ide_identify(uint16_t *buffer);

/// @brief Gets hard drive model from IDE identification block
/// @param str IDE name, null terminated, recommended char[48]
/// @return True on success, false on timeout
bool ide_get_drive_name(char *str);


/// @brief Read IDE sectors (command 0x20)
/// @param lba LBA address 
/// @param buffer 512 byte / 256 word * sectors_amount buffer for data
/// @param sectors_amount Amount of continous sectors to read
/// @return True on success, false on timeout
bool ide_read_sectors(uint32_t lba, uint16_t *buffer, uint8_t sectors_amount);


#endif // _IDE_H