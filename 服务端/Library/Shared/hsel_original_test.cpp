// hsel_original_test.cpp - 原版 2004 HSEL.lib 基准测试(真相来源)
//
// 链接原版 Client/Library/HSEL.lib, 使用与 dragon.vcxproj 相同的
// /ALTERNATENAME 符号桥接. 用于导出原版算法的基准行为:
//
//   hsel_original_test.exe encrypt <init.bin> <in.bin> <out.bin>
//   hsel_original_test.exe decrypt <init.bin> <in.bin> <out.bin>
//   hsel_original_test.exe selftest <init.bin> <in.bin>
//   hsel_original_test.exe sweep <init.bin>            # 5..260 全尺寸自回环扫描
//
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
    out.resize((size_t)n);
    if (n) fread(out.data(), 1, n, f);
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
    if (argc < 3) { fprintf(stderr, "bad args\n"); return 2; }

    std::vector<char> initbuf, input;
    if (!read_file(argv[2], initbuf) || initbuf.size() != 64) { fprintf(stderr, "init.bin must be 64 bytes\n"); return 2; }
    if (!read_file(argv[3], input)) { fprintf(stderr, "cannot read input\n"); return 2; }

    HSEL_INITIAL init;
    memcpy(&init, initbuf.data(), 64);

    if (mode == "sweep") {
        // 对 5..260 每个尺寸做 Enc+Dec 自回环, 输出 PASS/FAIL
        int fails = 0;
        for (int size = 5; size <= 260; size++) {
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
        // 关键: 加密/解密必须用两个各自 Initial 的对象(与客户端 en/de、
        // 服务端 pUser->en/pUser->de 的真实用法一致), 因为密钥旋转在
        // 加密侧(每次 4 步)与解密侧(每次 2 步)是非对称的.
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

    fprintf(stderr, "unknown mode\n");
    return 2;
}
