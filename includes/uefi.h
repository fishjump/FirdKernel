#pragma once

#include <stdint.h>

typedef uint64_t efi_physical_address_t;
typedef uint64_t efi_virtual_address_t;

typedef struct {
    uint32_t               Type;
    uint32_t               Pad;
    efi_physical_address_t PhysicalStart;
    efi_virtual_address_t  VirtualStart;
    uint64_t               NumberOfPages;
    uint64_t               Attribute;
} efi_memory_descriptor_t;