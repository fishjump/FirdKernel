#pragma once

#include <core/stdint.hpp>

// more flags are going to be supported.

constexpr core::uint8_t PDE_FLAG_PR = 0b0000'0001;
constexpr core::uint8_t PDE_FLAG_RW = 0b0000'0010;

constexpr core::uint8_t PTE_FALG_PR = 0b0000'0001;
constexpr core::uint8_t PTE_FALG_RW = 0b0000'0010;

struct page_directory_entry_t {
    core::uint8_t  flags;
    core::uint8_t  ignored : 1;
    core::uint8_t  reserved : 3;
    core::uint8_t  addr1 : 4;
    core::uint16_t addr2;
    core::uint32_t addr3;
} __attribute__((packed));

struct page_table_entry_t {
    core::uint8_t  flags;
    core::uint8_t  global : 1;
    core::uint8_t  reserved : 3;
    core::uint8_t  addr1 : 4;
    core::uint16_t addr2;
    core::uint32_t addr3;
} __attribute__((packed));

using PML4E_t = page_directory_entry_t[512];
using PDPTE_t = page_directory_entry_t[512];
using PDE_t   = page_directory_entry_t[1024];
using PTE_t   = page_table_entry_t[5120];

constexpr page_directory_entry_t make_PDE(core::uint64_t addr, core::uint8_t flags) {
    return {
        flags : flags,
        ignored : 0,
        reserved : 0,
        addr1 : (core::uint8_t)(addr >> 12),
        addr2 : (core::uint16_t)(addr >> 16),
        addr3 : (core::uint32_t)(addr >> 32)
    };
}

constexpr page_table_entry_t make_PTE(core::uint64_t addr, core::uint8_t flags) {
    return {
        flags : flags,
        global : 0,
        reserved : 0,
        addr1 : (core::uint8_t)(addr >> 12),
        addr2 : (core::uint16_t)(addr >> 16),
        addr3 : (core::uint32_t)(addr >> 32)
    };
}