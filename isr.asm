[bits 32]
global keyboard_isr_stub
extern keyboard_interrupt

keyboard_isr_stub:
    pusha               ; save all general registers
    call keyboard_interrupt
    popa
    iretd               ; 32-bit return from interrupt
