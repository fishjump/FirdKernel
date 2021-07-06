#include <bootparam.h>

const bootparam_t *bootp;

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

void put_str(int x_pos, int y_pos, const char *str,
             int color = 0xffffffff) {
    for(int i = 0; str[i] != '\0'; i++) {
        put_char(x_pos + i * 8, y_pos, str[i], color);
    }
}

extern "C" void _start(const bootparam_t *bootp) {
    ::bootp = bootp;

    put_str(100, 100, "hello, world!", 0x00ff0000);
    put_str(100, 200, "hello, world!", 0x0000ff00);
    put_str(100, 300, "hello, world!", 0x000000ff);

    while(true) {
        /* code */
    }

    return;
}