/* libmytool public API (C ABI, so the .symbols file stays simple and stable). */
#ifndef MYTOOL_MYTOOL_H
#define MYTOOL_MYTOOL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__)
#  define MYTOOL_API __attribute__((visibility("default")))
#else
#  define MYTOOL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MYTOOL_SHA256_HEX_LEN 64

typedef enum {
    MYTOOL_OK           = 0,
    MYTOOL_ERR_ARG      = -1,
    MYTOOL_ERR_IO       = -2,
    MYTOOL_ERR_MISMATCH = -3
} mytool_status;

/* Result of mytool_analyze_file().
 * ABI rule: set struct_size = sizeof(mytool_report) before calling; the
 * library only writes fields that fit in the caller's struct, so fields can
 * be appended in later minor releases without bumping the SONAME. */
typedef struct {
    size_t   struct_size;
    uint64_t size_bytes;
    uint32_t crc32;
    double   entropy_bits_per_byte;                      /* 0.0 .. 8.0 */
    char     sha256_hex[MYTOOL_SHA256_HEX_LEN + 1];
} mytool_report;

MYTOOL_API const char *mytool_version(void);
MYTOOL_API const char *mytool_strerror(mytool_status st);

/* Hash an in-memory buffer; out_hex must hold 65 bytes. */
MYTOOL_API mytool_status mytool_sha256_buf(const void *data, size_t len,
                                           char *out_hex);
/* Single pass over a file: size, CRC-32, SHA-256 and Shannon entropy. */
MYTOOL_API mytool_status mytool_analyze_file(const char *path,
                                             mytool_report *out);
/* MYTOOL_OK if the file's SHA-256 equals expected_hex (case-insensitive). */
MYTOOL_API mytool_status mytool_verify_file(const char *path,
                                            const char *expected_hex);

#ifdef __cplusplus
}
#endif
#endif
