#!/bin/bash
date
echo "========================================================="

# assemble boot.s file
as --32 src/boot.s -o boot.o

# compile C files
gcc -m32 -c src/kernel.c -o kernel.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c src/modules/utils.c -o utils.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c src/modules/char.c -o char.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c src/modules/random.c -o random.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

# linking the kernel with *.o files
ld -m elf_i386 -T src/linker.ld kernel.o utils.o char.o random.o boot.o -o mihkelanzhelo.bin -nostdlib

# check mihkelanzhelo.bin file is x86 multiboot file or not
grub-file --is-x86-multiboot mihkelanzhelo.bin

# building the iso file
mkdir -p isodir/boot/grub
cp mihkelanzhelo.bin isodir/boot/mihkelanzhelo.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o mihkelanzhelo-ingel.iso isodir/

# cleanup
rm *.o *.bin
rm -rf isodir
