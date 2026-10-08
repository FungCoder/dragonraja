// hsel_interop_test.cpp - HSEL 交叉验证工具(与 tests/mock_login_client.py 配合)
//
// 用法:
//   hsel_interop_test.exe encrypt <init.bin> <in.bin> <out.bin>   服务端 Encrypt
//   hsel_interop_test.exe decrypt <init.bin> <in.bin> <out.bin>   服务端 Decrypt
//   hsel_interop_test.exe selftest <init.bin> <in.bin>            自回环(Enc+Dec)
//
// init.bin = 64 字节 HSEL_INITIAL(与 Python 端共用同一组密钥)

#include "HSEL.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static bool read_file(const char* path, std::vector<char>& out) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    out.resize(n);
    fread(out.data(), 1, n, f);
    fclose(f);
    return true;
}

static bool write_file(const char* path, const void* data, size_t n) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    fwrite(data, 1, n, f);
    fclose(f);
    return true;
}

int main(int argc, char** argv) {
    std::string mode = argc >= 2 ? argv[1] : "";
    if ((mode == "encrypt" || mode == "decrypt") && argc != 5) { fprintf(stderr, "usage: %s encrypt|decrypt <init> <in> <out>\n", argv[0]); return 2; }
    if (mode == "selftest" && argc != 4) { fprintf(stderr, "usage: %s selftest <init> <in>\n", argv[0]); return 2; }
    if (argc < 4) { fprintf(stderr, "bad args\n"); return 2; }

    std::vector<char> initbuf, input;
    if (!read_file(argv[2], initbuf) || initbuf.size() != 64) { fprintf(stderr, "init.bin must be 64 bytes\n"); return 2; }
    if (!read_file(argv[3], input)) { fprintf(stderr, "cannot read input\n"); return 2; }

    HSEL_INITIAL init;
    memcpy(&init, initbuf.data(), 64);

    CHSEL_STREAM s;
    if (!s.Initial(init)) { fprintf(stderr, "Initial failed\n"); return 2; }

    if (mode == "encrypt" || mode == "decrypt") {
        std::vector<char> buf = input;
        bool ok;
        if (mode == "encrypt") ok = s.Encrypt(buf.data(), (int)buf.size());
        else                   ok = s.Decrypt(buf.data(), (int)buf.size());
        if (!ok) { fprintf(stderr, "op failed\n"); return 2; }
        if (!write_file(argv[4], buf.data(), buf.size())) return 2;
        printf("crc_byte=0x%02X\n", (unsigned char)s.GetCRCConvertChar());
        return 0;
    }

    if (mode == "selftest") {
        // 加密/解密用两个各自 Initial 的对象(与真实用法一致: 每次调用各旋转 1 次)
        std::vector<char> plain = input;
        std::vector<char> work = plain;
        CHSEL_STREAM e, d;
        e.Initial(init);
        d.Initial(init);
        if (!e.Encrypt(work.data(), (int)work.size())) return 2;
        unsigned char crc = (unsigned char)e.GetCRCConvertChar();
        if (!d.Decrypt(work.data(), (int)work.size())) return 2;
        bool pass = memcmp(work.data(), plain.data(), plain.size()) == 0;
        printf("selftest size=%d -> %s (crc=0x%02X)\n", (int)plain.size(), pass ? "PASS" : "FAIL", crc);
        return pass ? 0 : 1;
    }

    if (mode == "sweep") {
        int fails = 0;
        for (int size = 5; size <= 300; size++) {
            std::vector<char> plain(size);
            for (int i = 0; i < size; i++) plain[i] = (char)(i * 7 + 3);
            std::vector<char> work = plain;
            CHSEL_STREAM e, d;
            e.Initial(init); d.Initial(init);
            e.Encrypt(work.data(), size);
            unsigned char crc = (unsigned char)e.GetCRCConvertChar();
            d.Decrypt(work.data(), size);
            bool pass = memcmp(work.data(), plain.data(), size) == 0;
            if (!pass) fails++;
            printf("size=%3d %s crc=0x%02X\n", size, pass ? "PASS" : "FAIL", crc);
            fflush(stdout);
        }
        printf("sweep done, fails=%d\n", fails);
        return 0;
    }

    fprintf(stderr, "unknown mode\n");
    return 2;
}
