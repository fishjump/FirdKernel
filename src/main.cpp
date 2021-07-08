#include <bootparam.h>

#include <core.hpp>

const bootparam_t *bootp;

struct pos {
    int  x;
    int  y;
    void next();
    void new_line();
};

void pos::next() {
    x += 8;
    if(x >= bootp->framebuffer.width) {
        new_line();
    }
}

void pos::new_line() {
    x = 0;
    y += 16;
}

void put_char(int x_pos, int y_pos, char ch, int color = 0xffff'ffff) {
    const char *font = (char *)bootp->psf1_font.glyph_buffer
                       + ch * bootp->psf1_font.psf1_header->charsize;
    for(int y = y_pos; y < y_pos + 16; y++) {
        char row = font[y - y_pos];
        for(int x = x_pos; x < x_pos + 8; x++) {
            if(row & (0b1000'0000 >> (x - x_pos))) {
                (bootp->framebuffer.base
                 + y * bootp->framebuffer.pixel_per_scan_line)[x] = color;
            }
        }
    }
}

void put_str(pos &cursor, const char *str, const int &color = 0xffffffff) {
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
    "EfiReservedMemoryType     ", "EfiLoaderCode             ",
    "EfiLoaderData             ", "EfiBootServicesCode       ",
    "EfiBootServicesData       ", "EfiRuntimeServicesCode    ",
    "EfiRuntimeServicesData    ", "EfiConventionalMemory     ",
    "EfiUnusableMemory         ", "EfiACPIReclaimMemory      ",
    "EfiACPIMemoryNVS          ", "EfiMemoryMappedIO         ",
    "EfiMemoryMappedIOPortSpace", "EfiPalCode                "};

pos cursor = {x : 0, y : 0};

template <class T>
void printk(const T &t) {
    put_str(cursor, t);
}

template <class T, class... Args>
void printk(const T &t, const Args &...args) {
    put_str(cursor, t);
    printk(args...);
}

extern "C" void _start(const bootparam_t *bootp) {
    ::bootp = bootp;
    char buffer[30];

    printk("Hello, FirdOS.\n");
    for(int i = 0; i < ::bootp->memory_map.memory_map_size; i++) {
        auto desc  = bootp->memory_map.base[i];
        int  color = 0xffffffff;
        if(desc.type == 7) {
            color = 0xff00ffff;
        }
        put_str(cursor, memory_types[desc.type], color);
        put_str(cursor, " ");
        // default page size is 4KiB
        put_str(cursor,
                core::itoa(desc.num_of_pages * 4, buffer, sizeof(buffer)),
                color);
        put_str(cursor, " KB\n", 0xffff00ff);
    }

    uint8_t        test   = 0b11111111;
    core::bitmap_t bitmap = {_buffer : &test, _size : sizeof(test) * 8};

    // test all ones
    printk("test all ones\n0b", bitmap.to_string(buffer, sizeof(buffer)),
           "\n");

    // test all zeros
    test = 0;
    printk("test all zeros\n0b", bitmap.to_string(buffer, sizeof(buffer)),
           "\n");

    // test set
    printk("test set\n");
    for(int i = 0; i < 8; i++) {
        bitmap.set(i);
        printk("0b", bitmap.to_string(buffer, sizeof(buffer)), "\n");
        bitmap.reset(i);
    }

    // test reset
    printk("test reset\n");
    test = 0b11111111;
    for(int i = 0; i < 8; i++) {
        bitmap.reset(i);
        printk("0b", bitmap.to_string(buffer, sizeof(buffer)), "\n");
        bitmap.set(i);
    }

    // test filp
    printk("test filp\n");

    test = 0b01010101;
    printk("0b", bitmap.to_string(buffer, sizeof(buffer)), "\n");
    for(int i = 0; i < 8; i++) {
        bitmap.flip(i);
    }
    printk("0b", bitmap.to_string(buffer, sizeof(buffer)), "\n");

    test = 0b10101010;
    printk("0b", bitmap.to_string(buffer, sizeof(buffer)), "\n");
    for(int i = 0; i < 8; i++) {
        bitmap.flip(i);
    }
    printk("0b", bitmap.to_string(buffer, sizeof(buffer)), "\n");

    while(true) {
        /* code */
    }

    return;
}
