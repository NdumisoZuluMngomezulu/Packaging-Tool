#include "mytool/mytool.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define CHECK(c) do { if (!(c)) { std::fprintf(stderr, "FAIL %s:%d %s\n", __FILE__, __LINE__, #c); return 1; } } while (0)

int main() {
    char hex[65];
    CHECK(mytool_sha256_buf("abc", 3, hex) == MYTOOL_OK);
    CHECK(!std::strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
    CHECK(mytool_sha256_buf("", 0, hex) == MYTOOL_OK);
    CHECK(!std::strcmp(hex, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

    char tmpl[] = "/tmp/mytool-testXXXXXX";
    int fd = mkstemp(tmpl);
    CHECK(fd >= 0);
    FILE *f = fdopen(fd, "wb");
    std::fputs("123456789", f);
    std::fclose(f);
    mytool_report r; r.struct_size = sizeof r;
    CHECK(mytool_analyze_file(tmpl, &r) == MYTOOL_OK);
    CHECK(r.size_bytes == 9);
    CHECK(r.crc32 == 0xCBF43926u);                 /* canonical CRC-32 check value */
    CHECK(mytool_verify_file(tmpl, r.sha256_hex) == MYTOOL_OK);
    CHECK(mytool_verify_file(tmpl, "00000000000000000000000000000000000000000000000000000000000000ff") == MYTOOL_ERR_MISMATCH);
    std::remove(tmpl);
    CHECK(mytool_analyze_file("/nonexistent/file", &r) == MYTOOL_ERR_IO);
    std::puts("all tests passed");
    return 0;
}
