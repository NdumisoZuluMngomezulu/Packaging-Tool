#include "mytool/mytool.h"
#include "internal.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <strings.h>

extern "C" {

const char *mytool_version(void) { return "1.0.0"; }

const char *mytool_strerror(mytool_status st) {
    switch (st) {
    case MYTOOL_OK:           return "success";
    case MYTOOL_ERR_ARG:      return "invalid argument";
    case MYTOOL_ERR_IO:       return "I/O error";
    case MYTOOL_ERR_MISMATCH: return "checksum mismatch";
    }
    return "unknown error";
}

mytool_status mytool_sha256_buf(const void *data, size_t len, char *out_hex) {
    if (!out_hex || (!data && len)) return MYTOOL_ERR_ARG;
    Sha256 s;
    s.update(data, len);
    s.finish(out_hex);
    return MYTOOL_OK;
}

mytool_status mytool_analyze_file(const char *path, mytool_report *out) {
    if (!path || !out || out->struct_size < offsetof(mytool_report, sha256_hex))
        return MYTOOL_ERR_ARG;
    FILE *f = std::fopen(path, "rb");
    if (!f) return MYTOOL_ERR_IO;

    Sha256 sha;
    uint32_t crc = 0;
    uint64_t counts[256] = {0}, total = 0;
    unsigned char buf[1 << 16];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) {
        sha.update(buf, n);
        crc = crc32_update(crc, buf, n);
        for (size_t i = 0; i < n; i++) counts[buf[i]]++;
        total += n;
    }
    bool err = std::ferror(f);
    std::fclose(f);
    if (err) return MYTOOL_ERR_IO;

    double H = 0.0;
    for (uint64_t c : counts)
        if (c) { double p = double(c) / double(total); H -= p * std::log2(p); }

    char hex[65];
    sha.finish(hex);
    out->size_bytes = total;
    out->crc32 = crc;
    out->entropy_bits_per_byte = H;
    // Only write trailing fields the caller's struct actually has room for.
    if (out->struct_size >= sizeof(mytool_report)) std::memcpy(out->sha256_hex, hex, 65);
    return MYTOOL_OK;
}

mytool_status mytool_verify_file(const char *path, const char *expected_hex) {
    if (!expected_hex || std::strlen(expected_hex) != MYTOOL_SHA256_HEX_LEN)
        return MYTOOL_ERR_ARG;
    mytool_report r;
    r.struct_size = sizeof r;
    mytool_status st = mytool_analyze_file(path, &r);
    if (st != MYTOOL_OK) return st;
    return strcasecmp(r.sha256_hex, expected_hex) == 0 ? MYTOOL_OK : MYTOOL_ERR_MISMATCH;
}

} // extern "C"
