[org 0x7c00]          ; BIOS boot adress (0x7c00)
mov ah, 0x0E          ; BIOS TTY mode character write
mov al, 'B'           ; write 'B' to screen
int 0x10              ; BIOS screen service

jmp $                 ; infinite loop

times 510 - ($ - $$) db 0  ; fill to 512 byte
dw 0xAA55             ; Boot sign (BIOS check this)