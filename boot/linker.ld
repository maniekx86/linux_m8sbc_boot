ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

MEMORY {
    /* We are loaded at 0x90000, so our code lives here. */
    ROM (rx) : ORIGIN = 0x90000, LENGTH = 48K
}

SECTIONS {
    /* Place the .text section (code) at the beginning of our memory region */
    .text : {
        *(.text)
    } > ROM

    /* Place read-only data after the code */
    .rodata : {
        *(.rodata)
    } > ROM

    /* Place initialized data after read-only data */
    .data : {
        *(.data)
    } > ROM

    /* Uninitialized data section. The bootloader should zero this for us,
       or it can be handled in startup code if needed. */
    .bss : {
        *(.bss)
    } > ROM

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }

    gdt_descriptor_offset = gdt_descriptor - 0x90000;
}