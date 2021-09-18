#pragma once

#include <core/stdint.hpp>

/*
 *
 * +-------------------------------------------------------------+
 * |                       GDT Access Byte                       |
 * +--------+-----------+-------+--------+-------+-------+-------+
 * |    7   |    6..5   |   4   |    3   |   2   |   1   |   0   |
 * +--------+-----------+-------+--------+-------+-------+-------+
 * |   Pr   |   Privl   |   S   |   Ex   |   DC  |   RW  |   Ac  |
 * +--------+-----------+-------+--------+-------+-------+-------+
 * | View detail: https://wiki.osdev.org/Global_Descriptor_Table |
 * +-------------------------------------------------------------+
 * 
 */

constexpr core::uint8_t GDT_AC_PR = 0b1000'0000;

constexpr core::uint8_t GDT_AC_PRIVL_HIGHEST = 0b0000'0000;
constexpr core::uint8_t GDT_AC_PRIVL_LOWEST  = 0b0110'0000;

constexpr core::uint8_t GDT_AC_S = 0b0001'0000;

constexpr core::uint8_t GDT_AC_EX_CODE = 0b0000'0000;
constexpr core::uint8_t GDT_AC_EX_DATA = 0b0000'1000;

constexpr core::uint8_t GDT_AC_DC = 0b0000'0100;

constexpr core::uint8_t GDT_AC_RW = 0b0000'0010;

constexpr core::uint8_t GDT_AC_CODE_SEG = GDT_AC_PR | GDT_AC_S | GDT_AC_EX_CODE;
constexpr core::uint8_t GDT_AC_DATA_SEG = GDT_AC_PR | GDT_AC_S | GDT_AC_EX_DATA | GDT_AC_RW;

constexpr core::uint8_t GDT_AC_NULL            = 0b0000'0000;
constexpr core::uint8_t GDT_AC_KERNEL_CODE_SEG = GDT_AC_CODE_SEG | GDT_AC_PRIVL_HIGHEST;
constexpr core::uint8_t GDT_AC_KERNEL_DATA_SEG = GDT_AC_DATA_SEG | GDT_AC_PRIVL_HIGHEST;
constexpr core::uint8_t GDT_AC_USER_CODE_SEG   = GDT_AC_CODE_SEG | GDT_AC_PRIVL_LOWEST;
constexpr core::uint8_t GDT_AC_USER_DATA_SEG   = GDT_AC_DATA_SEG | GDT_AC_PRIVL_LOWEST;

constexpr core::uint8_t GDT_FLAG_GR_1B = 0b0000;
constexpr core::uint8_t GDT_FLAG_GR_4K = 0b1000;

constexpr core::uint8_t GDT_FLAG_SZ_16 = 0b0000;
constexpr core::uint8_t GDT_FLAG_SZ_32 = 0b0100;
constexpr core::uint8_t GDT_FLAG_SZ_64 = 0b0010;

struct GDT_descriptor_t {
    core::uint16_t limit1;
    core::uint16_t base1;
    core::uint8_t  base2;
    core::uint8_t  access;
    core::uint8_t  limit2 : 4;
    core::uint8_t  flags : 4;
    core::uint8_t  base3;
} __attribute__((packed));

struct GDT_t {
    GDT_descriptor_t null;
    GDT_descriptor_t kernel_code_64;
    GDT_descriptor_t kernel_data_64;
    GDT_descriptor_t user_code_64;
    GDT_descriptor_t user_data_64;
} __attribute__((packed));

struct GDT_entry_t {
    core::uint16_t size;
    core::uint64_t addr;
} __attribute__((packed));

constexpr GDT_descriptor_t make_GDT_descriptor(const core::uint32_t &base,
                                               const core::uint32_t &limit,
                                               const core::uint8_t & access,
                                               const core::uint8_t & flags) {
    return {
        limit1 : (core::uint16_t)limit,
        base1 : (core::uint16_t)base,
        base2 : (core::uint8_t)((base) >> 16),
        access : access,
        limit2 : (core::uint8_t)((limit) >> 16),
        flags : (core::uint8_t)flags,
        base3 : (core::uint8_t)((base) >> 24),
    };
}

constexpr GDT_entry_t make_GDT_entry(const GDT_t *gdt_addr, const core::uint16_t &size) {
    return {size : size, addr : (core::uint64_t)gdt_addr};
}

constexpr GDT_t make_default_GDT() {
    return {
        null : make_GDT_descriptor(0, 0, GDT_AC_NULL, 0),
        kernel_code_64 : make_GDT_descriptor(0, 0, GDT_AC_KERNEL_CODE_SEG, GDT_FLAG_SZ_64),
        kernel_data_64 : make_GDT_descriptor(0, 0, GDT_AC_KERNEL_DATA_SEG, GDT_FLAG_SZ_64),
        user_code_64 : make_GDT_descriptor(0, 0, GDT_AC_USER_CODE_SEG, GDT_FLAG_SZ_64),
        user_data_64 : make_GDT_descriptor(0, 0, GDT_AC_USER_DATA_SEG, GDT_FLAG_SZ_64),
    };
}

extern "C" void load_GDT(const GDT_entry_t *addr);