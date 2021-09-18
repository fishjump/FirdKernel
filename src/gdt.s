.section   .text
.global    load_GDT

load_GDT:
    // push %rax
    // mov $57, %rax
    // mov %rax, (%rdi)
    // pop %rax
    // ret

    lgdt (%rdi)

    // 2 << 3 = 0x10, 64 bit kernel data segment
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    // mov %ax, %ss
    // mov %ax, %gs


    ret

    // pop %rdi
    // push $0x08
    // push %rdi
    // lretq
