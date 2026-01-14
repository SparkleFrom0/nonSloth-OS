#define VGA_ADDRESS 0xB8000
#define WHITE_ON_BLACK 0x0F
#define BRIGHT_GREEN_ON_BLACK 0x0A
#define KEYBOARD_PORT 0x60
#define PIC1_COMMAND 0x20
#define PIC_EOI 0x20
#define INPUT_BUFFER_SIZE 256
#define IDT_SIZE 256
#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gp;

static void gdt_set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].base_low  = base & 0xFFFF;
    gdt[i].base_mid  = (base >> 16) & 0xFF;
    gdt[i].base_high = (base >> 24) & 0xFF;

    gdt[i].limit_low = limit & 0xFFFF;
    gdt[i].gran      = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt[i].access    = access;
}

extern void gdt_flush(struct gdt_ptr* gp);

void gdt_install() {
    gp.limit = sizeof(gdt) - 1;
    gp.base  = (uintptr_t)&gdt;

    // null
    gdt_set(0, 0, 0, 0, 0);
    // code (0x08)
    gdt_set(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // data (0x10)
    gdt_set(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush(&gp);
}

struct idt_entry {
    uint16_t offset_low;	//handler low 16 bits
    uint16_t selector;		//GDT selector(0x08)
    uint8_t  zero;			// not using, must be 0
    uint8_t  type_attr;		// 0x8E: present, ring0, 32-bit interrupt gate
    uint16_t offset_high;	//handler high 16 bits
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
struct idt_entry idt[IDT_SIZE];
struct idt_ptr   idtp;
extern void keyboard_isr_stub(); // ASM stub

static void set_idt_gate(int n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;        // code segment (kernel code in GDT)
    idt[n].zero         = 0;
    idt[n].type_attr   = 0x8E;        // present + ring0 + interrupt gate
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

static void load_idt() {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uintptr_t)&idt[0];
    asm volatile("lidt %0" : : "m"(idtp));
}

void idt_init() {
	int i;
    // reset all input
    for (i = 0; i < IDT_SIZE; i++) {
        set_idt_gate(i, 0);
    }

    // IRQ1 (keyboard) vector: after PIC remap 0x21 (33)
    set_idt_gate(0x21, (uintptr_t)keyboard_isr_stub);

    load_idt();
}


void print_string(const char* str,int row,int col,unsigned char color){
	volatile char* vga = (volatile char*) VGA_ADDRESS;
	int offset = (row * 80 + col) *2 , i;
	for(i =0 ; str[i] != '\0' ; i++){
		vga[offset] = str[i];
		vga[offset + 1] = color;
		offset += 2;
	}
}
//reading byte from keyboard port
unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0"
                  : "=a"(result)
                  : "dN"(port));
    return result;
}

//writing byte to keyboard port
void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1"
                  :
                  : "a"(data), "dN"(port));
}

// scanning table(0s for non output chars exp. shift , control,alt)
char scancode_table[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ','Q','W',
    'E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K',
    'L','Z','X','C','V','B','N','M'
};

char input_buffer[INPUT_BUFFER_SIZE];
int buffer_index = 0;
int input_ready = 0;

static void input_init() {
    input_buffer[0] = '\0';
    buffer_index = 0;
    input_ready = 0;
}

int cur_row =7,cur_col=10;

void keyboard_interrupt(){
	unsigned char scancode = inb(KEYBOARD_PORT);
	
	//break code -> ignore for now
	if(scancode & 0x80){
		outb(PIC1_COMMAND, PIC_EOI);
		return;
	}
	
    // scan code to character
    if (scancode < 128) {
        char c = scancode_table[scancode];
        if (c) {
            if(c == '\n'){
            	//end input when read Enter key
            	input_buffer[buffer_index] = '\0';//end string
            	input_ready=1;
				buffer_index = 0;//reset for new input
			}else if(buffer_index < INPUT_BUFFER_SIZE - 1){
				//add buffer when key is not Enter key
				input_buffer[buffer_index++] = c;
				print_string(&c,cur_row,cur_col++,WHITE_ON_BLACK);//live echo
			}
        }
    }
    // send EOI to PIC (interrupt ended)
    outb(PIC1_COMMAND, PIC_EOI);

}

char* keyboard_input(){
	//wait until Enter key
	while(!input_ready){
		//busy wait (will be a interrupt-driven)
	}
	input_ready = 0; // consume ready flag
	return input_buffer;
}

void pic_remap() {
    // start PIC1 and PIC2(ICW1)
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // Vector offsets (ICW2): master=0x20, slave=0x28
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3: master to slave connection and slave’s IRQ line
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4: 8086/88 (MCS-80/85) mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    //Only IRQ1 (keyboard) enabled on master:others masked
    outb(0x21, 0xFD);	//IRQ1 open (11111101b)
    outb(0xA1, 0xFF);	//all slave closed
}

void kernel_main() {
	gdt_install(); 			// 2) gdt
	pic_remap();   			// 2) remap the PIC
	idt_init();    			// 3) load IDT
	input_init();  			// 4) input buffer init
	asm volatile("sti");	// 5) open interrupts
	int i;
    const char *msg = "Hello to nonSloth-OS!";
    char *vidmem = (char*)0xb8000; // VGA text buffer address
    print_string(msg , 5, 10, BRIGHT_GREEN_ON_BLACK);
	char* user_input = keyboard_input();
	print_string(user_input, 7, 10, WHITE_ON_BLACK);
    while(1); // infinite loop
}
