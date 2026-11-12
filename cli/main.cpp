// mytool: command-line frontend for libmytool.
#include "mytool/mytool.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace {

void usage(FILE *o) {
    std::fputs(
        "Usage: mytool [--json] analyze FILE...\n"
        "       mytool sum FILE...\n"
        "       mytool verify MANIFEST\n"
        "       mytool --version | --help\n", o);
}

std::string json_escape(const std::string &s) {
    std::string o;
    for (char c : s) {
        if (c == '"' || c == '\\') { o += '\\'; o += c; }
        else if (static_cast<unsigned char>(c) < 0x20) o += ' ';
        else o += c;
    }
    return o;
}

int cmd_analyze(const std::vector<std::string> &files, bool json) {
    int rc = 0;
    if (json) std::puts("[");
    for (size_t i = 0; i < files.size(); i++) {
        mytool_report r; r.struct_size = sizeof r;
        mytool_status st = mytool_analyze_file(files[i].c_str(), &r);
        if (st != MYTOOL_OK) {
            std::fprintf(stderr, "mytool: %s: %s\n", files[i].c_str(), mytool_strerror(st));
            rc = 2; continue;
        }
        if (json)
            std::printf("  {\"file\":\"%s\",\"size\":%llu,\"crc32\":\"%08x\","
                        "\"entropy\":%.4f,\"sha256\":\"%s\"}%s\n",
                        json_escape(files[i]).c_str(), (unsigned long long)r.size_bytes,
                        r.crc32, r.entropy_bits_per_byte, r.sha256_hex,
                        i + 1 < files.size() ? "," : "");
        else
            std::printf("%s\n  size:    %llu bytes\n  crc32:   %08x\n"
                        "  entropy: %.4f bits/byte\n  sha256:  %s\n",
                        files[i].c_str(), (unsigned long long)r.size_bytes,
                        r.crc32, r.entropy_bits_per_byte, r.sha256_hex);
    }
    if (json) std::puts("]");
    return rc;
}

// Output is byte-compatible with sha256sum(1): "<hex>  <path>".
int cmd_sum(const std::vector<std::string> &files) {
    int rc = 0;
    for (const auto &f : files) {
        mytool_report r; r.struct_size = sizeof r;
        mytool_status st = mytool_analyze_file(f.c_str(), &r);
        if (st != MYTOOL_OK) {
            std::fprintf(stderr, "mytool: %s: %s\n", f.c_str(), mytool_strerror(st));
            rc = 2; continue;
        }
        std::printf("%s  %s\n", r.sha256_hex, f.c_str());
    }
    return rc;
}

// Manifest format = sha256sum output; blank lines and '#' comments ignored.
int cmd_verify(const std::string &manifest) {
    std::ifstream in(manifest);
    if (!in) {
        std::fprintf(stderr, "mytool: %s: cannot open manifest\n", manifest.c_str());
        return 2;
    }
    std::string line;
    int bad = 0, checked = 0;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.size() < 67 || line[64] != ' ') {
            std::fprintf(stderr, "mytool: malformed manifest line: %s\n", line.c_str());
            bad++; continue;
        }
        std::string hash = line.substr(0, 64);
        std::string path = line.substr(line.find_first_not_of(' ', 64));
        mytool_status st = mytool_verify_file(path.c_str(), hash.c_str());
        checked++;
        if (st == MYTOOL_OK) std::printf("%s: OK\n", path.c_str());
        else { std::printf("%s: FAILED (%s)\n", path.c_str(), mytool_strerror(st)); bad++; }
    }
    if (bad) std::fprintf(stderr, "mytool: %d of %d checks failed\n", bad, checked);
    return bad ? 1 : 0;
}

} // namespace

int main(int argc, char **argv) {
    bool json = false;
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--json")) json = true;
        else if (!std::strcmp(argv[i], "--version")) {
            std::printf("mytool %s\n", mytool_version()); return 0;
        } else if (!std::strcmp(argv[i], "--help") || !std::strcmp(argv[i], "-h")) {
            usage(stdout); return 0;
        } else args.emplace_back(argv[i]);
    }
    if (args.size() < 2) { usage(stderr); return 64; }
    std::string cmd = args[0];
    std::vector<std::string> rest(args.begin() + 1, args.end());
    if (cmd == "analyze") return cmd_analyze(rest, json);
    if (cmd == "sum")     return cmd_sum(rest);
    if (cmd == "verify")  return cmd_verify(rest[0]);
    usage(stderr);
    return 64;
}
