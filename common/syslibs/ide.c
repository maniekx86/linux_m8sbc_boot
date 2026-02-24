#include <stdint.h>
#include <stdbool.h>
#include <ide.h>
#include <x86_io.h>

// To do: timeouts ?

static void ide_nop_loop() { 
    for(int i=0;i<4;i++) {
        __asm__ volatile ( "nop;" );    
    }
}

static bool ide_wait_drq() {
    ide_nop_loop();
    
    while(1) {
    
        if(inb(IDE_STATUS) & 0b00001000) break;
        ide_nop_loop();
        
    }

    return true;
}

static bool ide_wait_ready() {
    ide_nop_loop();
    
    while(1) {
    
        if((inb(IDE_STATUS) & 0b11000000) == 0b01000000) break;
        ide_nop_loop();
        
    }
    return true;
}

void ide_reset() {
    ide_nop_loop();
    outb(IDE_STATUS, 0x08);
    ide_nop_loop();
}


bool ide_identify(uint16_t *buffer) {
    if(!ide_wait_ready()) return false;

    ide_nop_loop();
    outb(IDE_STATUS, 0xEC);

    if(!ide_wait_drq()) return false;

    for(int i = 0; i < 256; i++) {
        buffer[i] = inw(IDE_DATA);
    }

    return true;
}


bool ide_get_drive_name(char *str) { // doesnt require 512 byte buffer
    if(!ide_wait_ready()) return false;

    ide_nop_loop();
    outb(IDE_STATUS, 0xEC);

    if(!ide_wait_drq()) return false;

    uint8_t strpos = 0;
    uint16_t tmpbuf;
    for(int i = 0; i < 256; i++) { // we need to read all 256 words anyway
        tmpbuf = inw(IDE_DATA);
        if(i>=27 && i<27+20) {
            str[strpos] = (tmpbuf >> 8) & 0xFF;
            strpos++;
            str[strpos] = tmpbuf & 0xFF;
            strpos++;
        }
    }
    str[strpos] = '\0';

    return true;
}

bool ide_read_sectors(uint32_t lba, uint16_t *buffer, uint8_t sectors_amount) {
    if(!ide_wait_ready()) return false;

    outb(IDE_SECNUM, sectors_amount);
    ide_nop_loop();

    outb(IDE_LBA0, lba & 0xFF);
    ide_nop_loop();

    outb(IDE_LBA1, (lba >> 8) & 0xFF);
    ide_nop_loop();

    outb(IDE_LBA2, (lba >> 16) & 0xFF);
    ide_nop_loop();

    outb(IDE_LBA3, (((lba >> 24) & 0xFF) | 0b11100000) & 0b11101111); // LBA, Master
    ide_nop_loop();

    outb(IDE_STATUS, 0x20);

    if(!ide_wait_drq()) return false;

    uint32_t c = 0;

    for(int i = 0; i < sectors_amount; i++) {
        for(int j = 0; j < 256; j++) {
            buffer[c] = inw(IDE_DATA);
            c++;
        }
    }
    return true;
}
