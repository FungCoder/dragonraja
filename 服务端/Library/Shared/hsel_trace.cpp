// hsel_trace.cpp - ADD 5B encode/decode step diagnostic
#define private public
#define protected public
#include "HSEL.h"
#undef private
#undef protected
#include <cstdio>
#include <cstring>
#include <vector>

static void dump(const char* tag, const std::vector<char>& b) {
    printf("%-16s ", tag);
    for (size_t i = 0; i < b.size(); i++) printf("%02X", (unsigned char)b[i]);
    printf("\n");
}

int main() {
    HSEL_INITIAL init;
    const __int32 keys[16] = {3, 0x80, 0x0000, 0x1000, 111111111, 222222222, 333333333, 0,
                              5,7,11,13, 17,19,23,29};
    memcpy(&init, keys, 64);

    std::vector<char> plain;
    plain.push_back((char)0x00); plain.push_back((char)0x00);

    CHSEL_STREAM e, d;
    e.Initial(init);
    d.Initial(init);

    std::vector<char> buf = plain;
    e.SwapEncrypt(buf.data(), (int)buf.size());            dump("after-swap(e):", buf);
    e.DESLeftEncode_Type_2(buf.data(), (int)buf.size());   dump("after-L-enc:", buf);
    e.DESRightEncode_Type_2(buf.data(), (int)buf.size());  dump("after-R-enc:", buf);
    e.DESMiddleEncode_Type_2(buf.data(), (int)buf.size()); dump("after-M-enc:", buf);
    e.SetNextKey();
    e.GetCRC(buf.data(), (int)buf.size());
    dump("cipher:", buf);

    d.GetCRC(buf.data(), (int)buf.size());
    d.DESMiddleDecode_Type_2(buf.data(), (int)buf.size()); dump("after-M-dec:", buf);
    d.DESRightDecode_Type_2(buf.data(), (int)buf.size());  dump("after-R-dec:", buf);
    d.DESLeftDecode_Type_2(buf.data(), (int)buf.size());   dump("after-L-dec:", buf);
    d.SwapDecrypt(buf.data(), (int)buf.size());            dump("after-swap(d):", buf);

    printf("plain was:       ");
    for (size_t i = 0; i < plain.size(); i++) printf("%02X", (unsigned char)plain[i]);
    printf("\n");
    return 0;
}
