void kernel_main() {
    int i;
    const char *msg = "Hello to nonSloth-OS!";
    char *vidmem = (char*)0xb8000; // VGA text buffer address
    for (i = 0; msg[i] != '\0'; i++) {
        vidmem[i*2] = msg[i];      // character
        vidmem[i*2+1] = 0x07;      // color (white on black)
    }
    while(1); // infinite loop
}