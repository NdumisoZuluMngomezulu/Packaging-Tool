#pragma once
#include <cstddef>
#include <cstdint>

struct Sha256 {
    uint32_t h[8];
    uint8_t  buf[64];
    uint64_t total = 0;
    size_t   fill = 0;
    Sha256();
    void update(const void *data, size_t len);
    void finish(char *out_hex);   // 65 bytes, NUL-terminated
private:
    void block(const uint8_t *p);
};

uint32_t crc32_update(uint32_t crc, const void *data, size_t len);
