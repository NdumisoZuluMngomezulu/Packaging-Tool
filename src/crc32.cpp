#include "internal.h"

namespace {
struct Table {
    uint32_t t[256];
    Table() {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            t[i] = c;
        }
    }
};
const Table table;
}

uint32_t crc32_update(uint32_t crc, const void *data, size_t len) {
    const uint8_t *p = static_cast<const uint8_t *>(data);
    crc = ~crc;
    while (len--) crc = table.t[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    return ~crc;
}
