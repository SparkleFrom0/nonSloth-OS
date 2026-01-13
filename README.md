# nonSloth-OS



## Description

nonSloth-OS is a minimal operating system project created for learning and experimentation purposes.  

The goal is to understand how computers boot, how kernels work, and how low-level programming interacts with hardware.



## Features

* GRUB-based bootloader that prints a startup message
* Simple kernel written in C and Assembly
* VGA text output (Hello World on screen)
* Planned: memory management, file system support, multitasking



## Requirements

To build and run this project you need:

* GCC (cross-compiler or system compiler with freestanding mode)
* NASM (Netwide Assembler)
* QEMU (emulator for testing)
* Make (for build automation)



## Installation / Build Instructions

Clone the repository:

```bash

git clone https://github.com/SparkleFrom0/nonSloth-OS.git

cd nonSloth-OS
```

## Build The Project

```bash

make
qemu-system-i386 -cdrom nonSlothOS.iso
```

## Usage

When you run the OS, you should see a boot message followed by kernel output on the screen.
Currently, kernel prints:
```
Hello to nonSloth-OS!
```

This confirms that the bootloader and kernel are working together.



## Contributing


This project is primarily for educational purposes, but contributions are welcome.

Feel free to open issues or submit pull requests.



## License

This project is licensed under the GPL v3 License.

See the LICENSE file for details.



