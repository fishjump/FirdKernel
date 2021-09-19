#include <bootparam.h>

#include <core.hpp>
#include <gdt.hpp>
// #include "gdt.hpp"
#include "pageframe_allocator.hpp"
#include "paging.hpp"

extern core::uint8_t _kernel_start, _kernel_end;

struct pos_t {
    int  x;
    int  y;
    void next();
    void new_line();
};

struct global_t {
    PML4E_t PML4E __attribute__((aligned(0x1000)));
    PDPTE_t PDPTE __attribute__((aligned(0x1000)));
    PDE_t   PDE __attribute__((aligned(0x1000)));
    PTE_t   PTE __attribute__((aligned(0x1000)));

    GDT_t       GDT;
    GDT_entry_t GDT_entry;

    bootparam_t bootp;
    pos_t       cursor;

    pageframe_bitmap_allocator_t allocator;
} global __attribute__((aligned(0x1000)));

void pos_t::next() {
    x += 8;
    if(x >= global.bootp.framebuffer.width) {
        new_line();
    }
}

void pos_t::new_line() {
    x = 0;
    y += 16;
}

void put_char(int x_pos, int y_pos, char ch, int color = 0xffff'ffff) {
    const char *font = (char *)global.bootp.psf1_font.glyph_buffer
                       + ch * global.bootp.psf1_font.psf1_header.charsize;
    for(int y = y_pos; y < y_pos + 16; y++) {
        char row = font[y - y_pos];
        for(int x = x_pos; x < x_pos + 8; x++) {
            if(row & (0b1000'0000 >> (x - x_pos))) {
                (global.bootp.framebuffer.base
                 + y * global.bootp.framebuffer.pixel_per_scan_line)[x] = color;
            }
        }
    }
}

void put_str(pos_t &cursor, const char *str, const int &color = 0xffffffff) {
    for(int i = 0; str[i] != '\0'; i++) {
        if(str[i] == '\n') {
            cursor.new_line();
            continue;
        }

        put_char(cursor.x, cursor.y, str[i], color);
        cursor.next();
    }
}

const char *memory_types[] = {
    "EfiReservedMemoryType     ", "EfiLoaderCode             ", "EfiLoaderData             ",
    "EfiBootServicesCode       ", "EfiBootServicesData       ", "EfiRuntimeServicesCode    ",
    "EfiRuntimeServicesData    ", "EfiConventionalMemory     ", "EfiUnusableMemory         ",
    "EfiACPIReclaimMemory      ", "EfiACPIMemoryNVS          ", "EfiMemoryMappedIO         ",
    "EfiMemoryMappedIOPortSpace", "EfiPalCode                "};

template <class T>
void printk(const T &t) {
    put_str(global.cursor, t);
}

template <class T, class... Args>
void printk(const T &t, const Args &...args) {
    put_str(global.cursor, t);
    printk(args...);
}

extern "C" void _start(bootparam_t *bootp) {
    // 0x0000'0000'0000'0000 ~ 0x0000'7fff'ffff'ffff
    // 0xffff'8000'0000'0000 ~ 0xffff'ffff'ffff'ffff
    for(int i = 0; i < 512; i++) {
        global.PML4E[i] = make_PDE((core::uint64_t)global.PDPTE, PDE_FLAG_PR | PDE_FLAG_RW);
        global.PDPTE[i] = make_PDE((core::uint64_t)global.PDE, PDE_FLAG_PR | PDE_FLAG_RW);
        global.PDE[i]   = make_PDE((core::uint64_t)global.PTE, PDE_FLAG_PR | PDE_FLAG_RW);
    }
    global.PDPTE[2] = make_PDE((core::uint64_t)&global.PDE[512], PDE_FLAG_PR | PDE_FLAG_RW);
    for(int i = 0; i < 10; i++) {
        global.PDE[i] = make_PDE((core::uint64_t)&global.PTE[i * 512], PDE_FLAG_PR | PDE_FLAG_RW);
    }
    for(int i = 0; i < 9; i++)
        global.PDE[512 + i] =
            make_PDE((core::uint64_t)&global.PTE[(i + 1) * 512], PDE_FLAG_PR | PDE_FLAG_RW);
    for(int i = 0; i < 512; i++) {
        global.PTE[i] = make_PTE(i << 12, PTE_FALG_PR | PTE_FALG_RW);
    }
    for(int i = 512; i < 5120; i++) {
        global.PTE[i] = make_PTE(0x8000'0000 + (i - 512) * 0x1000, PTE_FALG_PR | PTE_FALG_RW);
    }

    global.bootp.framebuffer                    = bootp->framebuffer;
    global.bootp.memory_map                     = bootp->memory_map;
    global.bootp.psf1_font.psf1_header.mode     = bootp->psf1_font.psf1_header.mode;
    global.bootp.psf1_font.psf1_header.charsize = bootp->psf1_font.psf1_header.charsize;
    for(int i = 0; i < 2; i++) {
        global.bootp.psf1_font.psf1_header.magic[i] = bootp->psf1_font.psf1_header.magic[i];
    }
    for(int i = 0; i < sizeof(global.bootp.psf1_font.glyph_buffer); i++) {
        global.bootp.psf1_font.glyph_buffer[i] = bootp->psf1_font.glyph_buffer[i];
    }
    // global.allocator = {
    //     _memmap : bootp.memory_map,
    // };

    global.GDT       = make_default_GDT();
    global.GDT_entry = make_GDT_entry(&(global.GDT), sizeof(global.GDT) * 8 - 1);
    char buffer[60];
    asm("mov %0, %%cr3" : : "r"(global.PML4E));
    load_GDT(&global.GDT_entry);

    printk("Hello, FirdOS.\n");
    printk("GDT_entry: 0x", core::to_hex((core::uint64_t)&global.GDT_entry, buffer, sizeof(buffer)),
           "\n");
    printk("GDT_entry.addr: 0x",
           core::to_hex((core::uint64_t)global.GDT_entry.addr, buffer, sizeof(buffer)), "\n");
    printk("GDT_entry.size: ", core::itoa(global.GDT_entry.size, buffer, sizeof(buffer)), "\n");
    printk("h: ", core::itoa(global.bootp.framebuffer.height, buffer, sizeof(buffer)), "\n");
    printk("w: ", core::itoa(global.bootp.framebuffer.pixel_per_scan_line, buffer, sizeof(buffer)),
           "\n");
    printk("kernal_start: 0x", core::to_hex((core::uint64_t)&_kernel_start, buffer, sizeof(buffer)),
           "\n");
    printk("kernal_end: 0x", core::to_hex((core::uint64_t)&_kernel_end, buffer, sizeof(buffer)),
           "\n");
    printk("screen: 0x",
           core::to_hex((core::uint64_t)global.bootp.framebuffer.base, buffer, sizeof(buffer)),
           "\n");
    printk("PML4E: 0x", core::to_hex((core::uint64_t)global.PML4E, buffer, sizeof(buffer)), "\n");
    printk("PDPTE: 0x", core::to_hex((core::uint64_t)global.PDPTE, buffer, sizeof(buffer)), "\n");
    printk("PDE: 0x", core::to_hex((core::uint64_t)global.PDE, buffer, sizeof(buffer)), "\n");
    printk("PTE: 0x", core::to_hex((core::uint64_t)global.PTE, buffer, sizeof(buffer)), "\n");

    printk("GDT: 0x", core::to_hex((core::uint64_t)&global.GDT, buffer, sizeof(buffer)), "\n");
    printk("GDT_e: 0x", core::to_hex((core::uint64_t)&global.GDT_entry, buffer, sizeof(buffer)),
           "\n");

    // printk(
    //     "font addr: 0x",
    //     core::to_hex((core::uint64_t)global.bootp.psf1_font.glyph_buffer, buffer, sizeof(buffer)),
    //     "\n");
    // printk("bootp addr: 0x", core::to_hex((core::uint64_t)&bootp, buffer, sizeof(buffer)), "\n");
    // printk("bootp: 0x", core::to_hex((core::uint64_t)bootp, buffer, sizeof(buffer)), "\n");
    // printk("buffer addr: 0x", core::to_hex((core::uint64_t)buffer, buffer, sizeof(buffer)), "\n");
    // for(int i = 0; i < global.bootp.memory_map.memory_map_size; i++) {
    //     auto desc = bootp->memory_map.base[i];

    //     printk(memory_types[desc.type], " ");
    //     printk("0x", core::to_hex((uint64_t)desc.paddr, buffer, sizeof(buffer)), " - ");
    //     printk("0x",
    //            core::to_hex((uint64_t)desc.paddr + desc.num_of_pages * 4096 - 1, buffer,
    //                         sizeof(buffer)),
    //            " ");
    //     printk("size: ", core::itoa(desc.num_of_pages * 4, buffer, sizeof(buffer)), " KB\n");
    // }

    while(true) {
        /* code */
        asm("hlt");
    }

    return;
}
