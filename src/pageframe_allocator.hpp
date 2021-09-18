#pragma once

#include <bootparam.h>

#include <core.hpp>

struct pageframe_bitmap_allocator_t {
    core::bitmap_t _bmp;
    memory_map_t   _memmap;
    core::size_t   _reserved    = 0;
    core::size_t   _used        = 0;
    core::size_t   _free        = 0;
    core::size_t   _total       = 0;
    bool           _initialized = false;

    inline bool  initialize_bitmap();
    inline void *malloc(core::size_t size);
    inline void  free(void *ptr);
};

bool pageframe_bitmap_allocator_t::initialize_bitmap() {
    if(_initialized) {
        return _initialized;
    }

    core::memset(_bmp._buffer, 0, _bmp.capacity());
    for(int i = 0; i < _memmap.memory_map_size; i++) {
        memory_descriptor_t desc      = _memmap.base[i];
        core::size_t        bit_index = (core::uint64_t)desc.paddr / 4096;
        for(int j = 0; j < desc.num_of_pages; j++, bit_index++) {
            if(desc.type == memory_type_t::efi_conventional_memory) {
                _bmp.set(bit_index, true);
            } else {
            }
        }
    }

    _initialized = true;
    return _initialized;
}

void *pageframe_bitmap_allocator_t::malloc(core::size_t size) {
    if(!_initialized) {
        initialize_bitmap();
    }

    return nullptr;
}

void pageframe_bitmap_allocator_t::free(void *ptr) {
    core::size_t index = (core::uint64_t)ptr / 4096;
    if(!_bmp[index]) {
        return;
    }
    _bmp.reset(index);
}
