BITS 16
;org 0x700

section .text
global _start
extern main
extern setup_paging

global gdt_descriptor ; Linker calculated
extern gdt_descriptor_offset

_start:
    cli ; Disable interrupts during setup

    mov bx, es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'A'
    mov byte [es:1], 0x07
    mov es, bx


    ;mov ax, 0x0000
    mov ax, 0x9000
    mov ds, ax
    mov ss, ax
    mov es, ax
    
    ; Load GDT
    lgdt [gdt_descriptor_offset]


    ; Set the PE bit to enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax


    ; Go to protected mode
    jmp dword 0x10:pmode

BITS 32

pmode:

    ; Now in 32-bit protected mode
    ; Set up data segment registers to point to our 32-bit data segment (selector 0x10)
    mov ax, 0x18
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; set bit NE in CR0 to enable x87 FPU exceptions
    mov eax, cr0
    or eax, 0x20
    mov cr0, eax

    ; Set up the stack pointer
    mov esp, 0x3F0000

    ; Call our C main function
    call main

    ; If main ever returns, halt the CPU
    hlt

; --- LINUX COMPATIBLE GDT ---
gdt_start:
    ; 0x00: Null Descriptor
    dd 0
    dd 0

    ; 0x08: UNUSED (or BIOS, Linux 2.2 usually ignores this or uses it for APM)
    ; We can just duplicate code here if we want or leave null.
    dd 0
    dd 0

    ; 0x10: KERNEL CODE (Ring 0) 
    ; Must be Base 0, Limit 4GB, Executable
    dw 0xFFFF    ; Limit (low)
    dw 0         ; Base (low)
    db 0         ; Base (mid)
    db 0x9A      ; Access: Present, Ring 0, Code, Exec/Read
    db 0xCF      ; Flags: 4KB, 32-bit
    db 0         ; Base (high)

    ; 0x18: KERNEL DATA (Ring 0)
    ; Must be Base 0, Limit 4GB, Writable
    dw 0xFFFF    
    dw 0         
    db 0         
    db 0x92      ; Access: Present, Ring 0, Data, Read/Write
    db 0xCF      
    db 0         
gdt_end:

; GDT Descriptor (pointer to the GDT)
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size of the GDT
    dd gdt_start                ; Address of the GDT

; GDT pointer if we are running at 0xC0000000
gdt_descriptor_kernel:
    dw gdt_end - gdt_start - 1 ; Size of the GDT
    dd gdt_start + 0xC0000000  ; Address of the GDT
