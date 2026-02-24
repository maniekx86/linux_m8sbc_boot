#include <stdint.h>
#include <fat.h>
#include <ide.h>
#include <x86_io.h>
#include <stdlib.h>
#include <string.h>


uint8_t* video_mem = (uint8_t*)0xB8000;

static Fat g_fat;

void die(const char *msg) {
    
    for(int i=0; msg[i]!='\0'; i++) {
        video_mem[i*2] = msg[i];
        video_mem[i*2+1] = 0x4F; // white on red
    }
    // hlt
    asm volatile ("cli");
    asm volatile ("hlt");
    while(1);

}

static bool disk_read(uint8_t* buf, uint32_t sect) {
    if(!ide_read_sectors(sect, (uint16_t*)buf, 1)) {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
static bool disk_write(const uint8_t* buf, uint32_t sect) {
    // not supported yet
    return true;
}

// Fat32 init
static DiskOps g_ops = {
  .read  = disk_read,
  .write = disk_write,
};

uint8_t screen_x = 0;
uint8_t screen_y = 0;

void print(const char *str) {
    for(int i = 0; str[i] != '\0'; i++) {
        if(str[i] == '\n') {
            screen_x = 0;
            screen_y++;
            continue;
        } else if(str[i] == '\r') {
            screen_x = 0;
            continue;
        }
        video_mem[(screen_y * 80 + screen_x) * 2] = str[i];
        video_mem[(screen_y * 80 + screen_x) * 2 + 1] = 0x07; // light grey on black
        screen_x++;
        if(screen_x >= 80) {
            screen_x = 0;
            screen_y++;
        }
    }
    // If screen_y >= 25, scroll up
    if(screen_y >= 25) {
        for(int y = 1; y < 25; y++) {
            for(int x = 0; x < 80; x++) {
                video_mem[((y - 1) * 80 + x) * 2] = video_mem[(y * 80 + x) * 2];
                video_mem[((y - 1) * 80 + x) * 2 + 1] = video_mem[(y * 80 + x) * 2 + 1];
            }
        }
        // clear last line
        for(int x = 0; x < 80; x++) {
            video_mem[((24) * 80 + x) * 2] = ' ';
            video_mem[((24) * 80 + x) * 2 + 1] = 0x07; // light gray on black
        }
        screen_y = 24;
    }
    // Update hardware cursor
    uint16_t cursor_pos = screen_y * 80 + screen_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}

void print_itoa_debugcon(const char *prefix, int32_t val) {
#if false
    char buf[12];
    itoa(val, buf, 10);
    // outb to debug port
    for(int i = 0; prefix[i] != '\0'; i++) {
        outb(0xE9, prefix[i]);
    }
    for(int i = 0; buf[i] != '\0'; i++) {
        outb(0xE9, buf[i]);
    }
    outb(0xE9, '\n');
#else 
    print(prefix);
    char buf[12];
    itoa(val, buf, 10);
    print(buf);
    print("\n");
#endif
}


uint8_t *buffer = (uint8_t *)0x100000;

#define ZERO_PAGE   0x98000
#define LOAD_ADDR   0x1000

// Standard Linux boot protocol offsets (relative to Zero Page start)
#define CMD_LINE_ADDR     0x9F000  // Safe location for string
#define KERNEL_LOAD_ADDR  0x001000 // Fixed entry point for zImage

// Memory map structure
struct e820_entry {
    uint64_t addr;
    uint64_t size;
    uint32_t type;
} __attribute__((packed));

#define E820_RAM  1
#define E820_RESERVED 2

uint8_t *zero_page = (uint8_t *)ZERO_PAGE;

void setup_zero_page(uint32_t initrd_addr, uint32_t initrd_size) {
    // Assume zero_page is already zeroed out

    // 2. Magic signatures
    // Offset 0x1FE: Boot Flag
    *(uint16_t *)(zero_page + 0x1FE) = 0xAA55;
    // Offset 0x202: Header Magic "HdrS"
    *(uint32_t *)(zero_page + 0x202) = 0x53726448;
    // Offset 0x210: Bootloader ID (0xFF = undefined/custom)
    *(uint8_t *)(zero_page + 0x210) = 0xFF;
    // Offset 0x211: Load Flags (Bit 0 = loaded high) -> 0 for zImage (loaded low)
    *(uint8_t *)(zero_page + 0x211) = 0x00; 

    // Linux 2.2 decompressor looks here (Offset 0x02), NOT in E820.
    // Value is extended memory in KB. 
    // 4MB - 1MB (low) = 3072 KB Extended.
    *(uint16_t *)(zero_page + 0x0002) = 3072;

    // 3. Screen info
    // Offset 0x00: orig_x, orig_y, mode, etc.
    *(uint8_t *)(zero_page + 0x0000) = 0;    // orig_x
    *(uint8_t *)(zero_page + 0x0001) = 0;    // orig_y
    *(uint16_t *)(zero_page + 0x0004) = 0;   // orig_video_page
    *(uint8_t *)(zero_page + 0x0006) = 3;    // orig_video_mode (3 = 80x25 color)
    *(uint8_t *)(zero_page + 0x0007) = 80;   // orig_video_cols (Offset 0x07)
    *(uint8_t *)(zero_page + 0x000E) = 25;   // orig_video_lines
    *(uint8_t *)(zero_page + 0x000F) = 1;    // orig_video_isVGA (1 = VGA)
    *(uint16_t *)(zero_page + 0x0010) = 16;  // orig_video_points (font height)

    // 4. Command line pointer
    *(uint32_t *)(zero_page + 0x228) = CMD_LINE_ADDR;

    // 5. Initrd (ramdisk) info
    *(uint32_t *)(zero_page + 0x218) = initrd_addr; // initrd_start
    *(uint32_t *)(zero_page + 0x21C) = initrd_size; // initrd_size
}
void setup_memory_map() {
    // Offset 0x2D0 is where the E820 map starts in the zero page
    struct e820_entry *map = (struct e820_entry *)(zero_page + 0x2D0);
    uint8_t *count = (uint8_t *)(zero_page + 0x1E8);


    map[0].addr = 0x00000000;
    map[0].size = 0x000A0000; // 640KB
    map[0].type = E820_RAM;
    
    map[1].addr = 0x00100000;
    map[1].size = 0x00300000; // 3 MB
    map[1].type = E820_RAM;
  
    map[2].addr = 0x004A0000;
    map[2].size = 0x00060000; // 384KB
    map[2].type = E820_RAM;

    *count = 3; // 3 entries
}

void enter_kernel(void *zero_page, void *entry_point) {
    __asm__ __volatile__ (
        "cli \n\t"                 // No interrupts
        "movl $0, %%ebx \n\t"      // EBX must be 0
        "movl %0, %%esi \n\t"      // ESI = 0x90000 (Zero Page)
        "jmp *%1 \n\t"             // Jump to 0x1000
        : 
        : "r" (zero_page), "r" (entry_point) 
        : "ebx", "esi", "memory"
    );
    
    // We should never reach there
    while(1);
}

bool init_on_realx86 = false;

void main() {
    // Clear video memory
    for(int i=0; i<80*25*2; i=i+2) {
        video_mem[i] = 0;
        video_mem[i+1] = 0x07; // light gray on black
    }

    print("Hello World! Linux for the M8SBC-486\n");
    
    // detect real x86 / M8SBC-486
    // TO DO  

    int err = fat_probe(&g_ops, 0);
    if(err != 0) {
        die("Crash! (fat_probe)");
    }
    err = fat_mount(&g_ops, 0, &g_fat, "root");
    if(err != 0) {
        die("Crash! (fat_mount)");
    }

    // test: list directory root
    /*
    Dir dir_entry;
    DirInfo dir_info;

    fat_dir_open(&dir_entry, "/root");
    while(1) {
        err = fat_dir_read(&dir_entry, &dir_info);
        if(err == FAT_ERR_EOF) break;

        if(!(dir_info.attr & FAT_ATTR_LABEL)) {
            for(int i=0; i<dir_info.name_len; i++) {
                outb(0xE9, dir_info.name[i]);
                if(dir_info.attr & FAT_ATTR_DIR) {
                    outb(0xE9, '/');
                }
            }
            outb(0xE9, '\n');
        }

        err = fat_dir_next(&dir_entry);
    }*/

    File file;

    err = fat_file_open(&file, "/root/zimage", FAT_READ);
    if(err != 0) {
        die("Crash! (open /zimage)");
    }

    uint32_t bytes_read = 0;
    err = fat_file_read(&file, buffer, 576*1024, &bytes_read); // max 576KB
    
    if(err != 0) {
        die("Crash! (fat_file_read)");
    }

    uint32_t file_size = bytes_read;

    print_itoa_debugcon("Bytes read: ", bytes_read);

    uint8_t setup_sects = buffer[0x1F1];
    if (setup_sects == 0) setup_sects = 4; // Fallback for ancient versions

    uint32_t header_size = (setup_sects + 1) * 512;
    uint32_t payload_size = file_size - header_size;

    // copy to LOAD_ADDR
    uint8_t* dest = (uint8_t*)LOAD_ADDR;
    uint8_t* src = buffer + header_size;
    for(uint32_t i = 0; i < payload_size; i++) {
        dest[i] = src[i];
    }

    // clear zero page
    uint8_t* zero_page = (uint8_t*)ZERO_PAGE;
    for(uint32_t i = 0; i < 4096; i++) {
        zero_page[i] = 0;
    }
    
    // setup zero page
    setup_zero_page(0, 0); // no initrd
    setup_memory_map();

    // init master PIC
    outb(0x20, 0x11);
    outb(0x21, 0x20);
    if(init_on_realx86) {
        outb(0x21, 0x04);
    } else {
        outb(0x21, 0x00);
    }
    outb(0x21, 0x01);
    outb(0x21, 0xFE);

    // not present on target machine - init slave PIC and keyboard controller
    if(init_on_realx86) {
        outb(0xA0, 0x11);
        outb(0xA1, 0x28);
        outb(0xA1, 0x02);
        outb(0xA1, 0x01);
        outb(0xA1, 0xFF);

        // init keyboard controller to scan code set 1
        outb(0x60, 0xF0);
        outb(0x60, 0x01);
    }

    // Set the boot parameters
    char cmdline[256];
    if(init_on_realx86) {
        strcpy(cmdline, "ide0=0x1f0,0x3f6,14 root=/dev/hda2 rw init=/sbin/init console=tty0"); // standard PC command line
        
        print("Running on real x86 hardware\n");
    } else {
        strcpy(cmdline, "ide0=0x1f0,0x3f6,5 root=/dev/hda2 rw init=/sbin/init console=tty0"); // M8SBC-486 command line
        
        print("Running on M8SBC-486\n");
    }
    
    uint8_t *cmdline_dest = (uint8_t *)CMD_LINE_ADDR;
    for (size_t i = 0; cmdline[i] != '\0'; i++) {
        cmdline_dest[i] = cmdline[i];
    }
    
    cmdline_dest[strlen(cmdline)] = '\0'; // null-terminate
    print("cmdline = ");
    print((const char *)cmdline_dest);
    print("\n");
    print("Kernel entry!\n");

    // Update X and Y position in zero page for framebuffer info
    // so kernel can use it for earlyprintk
    *(uint8_t *)(zero_page + 0x0000) = 0;    // orig_x
    *(uint8_t *)(zero_page + 0x0001) = screen_y;    // orig_y

    

    // Enter kernel
    enter_kernel((void*)ZERO_PAGE, (void*)KERNEL_LOAD_ADDR);



    return;
}
