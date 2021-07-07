#include <bootparam.h>

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

void put_char(int x_pos, int y_pos, char ch, int color = 0xffffffff) {
    const char *font = (char *)bootp->psf1_font.glyph_buffer
                       + ch * bootp->psf1_font.psf1_header->charsize;
    for(int y = y_pos; y < y_pos + 16; y++) {
        char row = font[y - y_pos];
        for(int x = x_pos; x < x_pos + 8; x++) {
            if(row & (0b10000000 >> (x - x_pos))) {
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

char *itoa(uint64_t src, char *dest) {
    uint64_t len = 0, tmp = src;
    if(tmp) {
        while(tmp) {
            tmp /= 10;
            len++;
        }
    } else {
        len = 1;
    }

    for(int i = len - 1; i >= 0; i--) {
        dest[i] = src % 10 + '0';
        src /= 10;
    }
    dest[len] = '\0';

    return dest;
}

const char *memory_types[] = {
    "EfiReservedMemoryType     ", "EfiLoaderCode             ",
    "EfiLoaderData             ", "EfiBootServicesCode       ",
    "EfiBootServicesData       ", "EfiRuntimeServicesCode    ",
    "EfiRuntimeServicesData    ", "EfiConventionalMemory     ",
    "EfiUnusableMemory         ", "EfiACPIReclaimMemory      ",
    "EfiACPIMemoryNVS          ", "EfiMemoryMappedIO         ",
    "EfiMemoryMappedIOPortSpace", "EfiPalCode                "};

extern "C" void _start(const bootparam_t *bootp) {
    ::bootp = bootp;
    char buffer[30];
    pos  cursor = {0, 0};
    put_str(cursor, "Hello, FirdOS.\n");
    for(int i = 0; i < ::bootp->memory_map.memory_map_size; i++) {
        auto desc  = bootp->memory_map.base[i];
        int  color = 0xffffffff;
        if(desc.type == 7) {
            color = 0xff00ffff;
        }
        put_str(cursor, memory_types[desc.type], color);
        put_str(cursor, " ");
        // default page size is 4KiB
        put_str(cursor, itoa(desc.num_of_pages * 4, buffer), color);
        put_str(cursor, " KB\n", 0xffff00ff);
    }

    while(true) {
        /* code */
    }

    return;
}