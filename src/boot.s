.set MAGIC, 0x1BADB002 # magic number
.set FLAGS, 0
.set CHECKSUM, -(MAGIC + FLAGS)

# enable multiboot
.section .multiboot

.long MAGIC
.long FLAGS
.long CHECKSUM

stackBottom:
    .skip 4096 # max size of stack - 512 bytes

stackTop:
    .section .text
    .global _start
    .type _start, @function

_start:
    mov $stackTop, %esp
    call kernel_entry
    cli

# system in infinite loop
hltloop:
    hlt
    jmp hltloop

.size _start, . - _start
