# nonSloth-OS Makefile

# Compilers
CC = gcc
LD = ld
AS = nasm

# Flags
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T link.ld -nostdlib

# Source files
C_SOURCES = kernel.c
S_SOURCES = multiboot_header.S start.S
ASM_SOURCES = gdt_flush.asm isr.asm

C_OBJECTS = $(C_SOURCES:.c=.o)
S_OBJECTS = $(S_SOURCES:.S=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

OBJECTS = $(C_OBJECTS) $(S_OBJECTS) $(ASM_OBJECTS)

# Output binary
KERNEL_BIN = kernel.bin

# Default target
all: $(KERNEL_BIN)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile .S files (GAS syntax)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Compile .asm files (NASM syntax)
%.o: %.asm
	$(AS) -f elf32 $< -o $@

# Link everything into the kernel binary
$(KERNEL_BIN): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Build bootable ISO (optional)
iso: $(KERNEL_BIN)
	mkdir -p iso/boot/grub
	cp $(KERNEL_BIN) iso/boot/kernel.bin
	echo 'set timeout=0\nset default=0\nmenuentry "nonSloth-OS" {\n multiboot /boot/kernel.bin\n}' > iso/boot/grub/grub.cfg
	grub-mkrescue -o nonSlothOS.iso iso

# Clean build artifacts
clean:
	rm -f *.o $(KERNEL_BIN) nonSlothOS.iso
	rm -rf iso