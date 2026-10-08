/*
 * HSEL (HWOARANG SANGWOO ENCRYPT LIBRARY) - 忠实重建版 (2026-09-28)
 *
 * 依据 Client/Library/HSEL.lib (2004, VC6 x86) 的完整反汇编逐指令重建,
 * 并以 hsel_original_test.exe (原版 lib 基准) 做字节级交叉验证:
 *   - 全尺寸 (4..300) Encrypt 密文与原版逐字节一致
 *   - 配对对象自回环全尺寸通过
 *   - 双向互操作 (原版Enc->本版Dec, 本版Enc->原版Dec) 全尺寸通过
 *
 * 算法结构:
 *   Encrypt: SwapEncrypt -> Left(前向链) -> Right(后向链,末对齐) -> Middle(ECB)
 *            -> SetNextKey(1次) -> GetCRC
 *   Decrypt: GetCRC -> Middle(ECB逆) -> Right(后向链逆) -> Left(前向链逆)
 *            -> SwapDecrypt -> SetNextKey(1次)
 *   三遍的块网格不同:
 *     Left  : 起始对齐 (buffer+0 起, 步进 +4, 共 blocks 个 DWORD)
 *     Right : 末尾对齐 (buffer+size-4 起, 步进 -4, 共 blocks 个 DWORD)
 *     Middle: 起始对齐 ECB (无链, 每块独立与密钥运算)
 *   余数 (size%4) 字节:
 *     Left/Middle: buffer[blocks*4+j] ^= key_bytes[rem-j]  (j=0..rem-1, 永不用 key[0])
 *     Right      : buffer[j]          ^= key_bytes[j]      (缓冲区头部 rem 字节)
 *   密钥旋转: 每次 Encrypt/Decrypt 调用各 1 次, 直接作用于 Init.Keys.
 *   GetCRC: 4字节块 XOR + 余数字节 movsx(符号扩展) XOR.
 *   交换: 位置 = ((key >> (8*i)) & 0xF) % (n-1); Encrypt 8 组, Decrypt 15 组.
 *   类型 0x10/0x20/0x40/0x80 = 块 XOR/ADD/SUB/SUB + 余数 XOR/ADD/SUB/XOR.
 */

#include "HSEL.h"
#include <cstring>
#include <cstdlib>

CHSEL_STREAM::CHSEL_STREAM(void) {
    iVersion = 3;
    iHSELType = 0;
    memset(&Init, 0, sizeof(HSEL_INITIAL));
    iBlockCount = 0;
    iRemainCount = 0;
    iPos = 0;
    iCRCValue = 0;
    iTempLeftKey = 0;
    iTempRightKey = 0;
    iTempMiddleKey = 0;
    iDecryptKey = 0;
    lpDesEncryptType = NULL;
    lpDesDecryptType = NULL;
    lpDesLeftEncrypt = NULL;
    lpDesLeftDecrypt = NULL;
    lpDesRightEncrypt = NULL;
    lpDesRightDecrypt = NULL;
    lpDesMiddleEncrypt = NULL;
    lpDesMiddleDecrypt = NULL;
    lpSwapEncrypt = NULL;
    lpSwapDecrypt = NULL;
}

CHSEL_STREAM::~CHSEL_STREAM(void) {}

__int32 CHSEL_STREAM::Initial(HSEL_INITIAL hselinit) {
    const __int32 des = hselinit.iDesCount & 0xF;
    if (des == 1) {
        lpDesEncryptType = &CHSEL_STREAM::DESSingleEncode;
        lpDesDecryptType = &CHSEL_STREAM::DESSingleDecode;
    } else if (des == 3) {
        lpDesEncryptType = &CHSEL_STREAM::DESTripleEncode;
        lpDesDecryptType = &CHSEL_STREAM::DESTripleDecode;
    } else {
        return 0;
    }

    if (hselinit.iEncryptType == HSEL_ENCRYPTTYPE_RAND)
        hselinit.iEncryptType = HSEL_ENCRYPTTYPE_1 << (rand() % 4);

    switch (hselinit.iEncryptType & 0xF0) {
    case 0x10:
        lpDesLeftEncrypt   = &CHSEL_STREAM::DESLeftEncode_Type_1;
        lpDesLeftDecrypt   = &CHSEL_STREAM::DESLeftDecode_Type_1;
        lpDesRightEncrypt  = &CHSEL_STREAM::DESRightEncode_Type_1;
        lpDesRightDecrypt  = &CHSEL_STREAM::DESRightDecode_Type_1;
        lpDesMiddleEncrypt = &CHSEL_STREAM::DESMiddleEncode_Type_1;
        lpDesMiddleDecrypt = &CHSEL_STREAM::DESMiddleDecode_Type_1;
        break;
    case 0x20:
        lpDesLeftEncrypt   = &CHSEL_STREAM::DESLeftEncode_Type_2;
        lpDesLeftDecrypt   = &CHSEL_STREAM::DESLeftDecode_Type_2;
        lpDesRightEncrypt  = &CHSEL_STREAM::DESRightEncode_Type_2;
        lpDesRightDecrypt  = &CHSEL_STREAM::DESRightDecode_Type_2;
        lpDesMiddleEncrypt = &CHSEL_STREAM::DESMiddleEncode_Type_2;
        lpDesMiddleDecrypt = &CHSEL_STREAM::DESMiddleDecode_Type_2;
        break;
    case 0x40:
        lpDesLeftEncrypt   = &CHSEL_STREAM::DESLeftEncode_Type_3;
        lpDesLeftDecrypt   = &CHSEL_STREAM::DESLeftDecode_Type_3;
        lpDesRightEncrypt  = &CHSEL_STREAM::DESRightEncode_Type_3;
        lpDesRightDecrypt  = &CHSEL_STREAM::DESRightDecode_Type_3;
        lpDesMiddleEncrypt = &CHSEL_STREAM::DESMiddleEncode_Type_3;
        lpDesMiddleDecrypt = &CHSEL_STREAM::DESMiddleDecode_Type_3;
        break;
    case 0x80:
        lpDesLeftEncrypt   = &CHSEL_STREAM::DESLeftEncode_Type_4;
        lpDesLeftDecrypt   = &CHSEL_STREAM::DESLeftDecode_Type_4;
        lpDesRightEncrypt  = &CHSEL_STREAM::DESRightEncode_Type_4;
        lpDesRightDecrypt  = &CHSEL_STREAM::DESRightDecode_Type_4;
        lpDesMiddleEncrypt = &CHSEL_STREAM::DESMiddleEncode_Type_4;
        lpDesMiddleDecrypt = &CHSEL_STREAM::DESMiddleDecode_Type_4;
        break;
    default: return 0;
    }

    if (hselinit.iSwapFlag & HSEL_SWAP_FLAG_OFF) {
        lpSwapEncrypt = &CHSEL_STREAM::NoSwapEncrypt;
        lpSwapDecrypt = &CHSEL_STREAM::NoSwapDecrypt;
    } else {
        lpSwapEncrypt = &CHSEL_STREAM::SwapEncrypt;
        lpSwapDecrypt = &CHSEL_STREAM::SwapDecrypt;
    }

    Init = hselinit;

    if ((hselinit.iCustomize & 0xF000) == 0) {
        GenerateKeys(Init.Keys);
    } else if ((hselinit.iCustomize & 0xF000) == HSEL_KEY_TYPE_CUSTOMIZE) {
        ;
    } else {
        return 0;
    }

    Init.iCustomize = HSEL_KEY_TYPE_CUSTOMIZE;
    iHSELType = hselinit.iEncryptType | hselinit.iDesCount | hselinit.iSwapFlag;
    iCRCValue = 0;
    return 1;
}

bool CHSEL_STREAM::ChackFaultStreamSize(const __int32 iStreamSize) {
    return iStreamSize > 0;
}

void CHSEL_STREAM::GenerateKeys(HselKey &key) {
    int a, b;
    a = rand() % 0x84D0 + 0x2810; b = rand() % 0x7530 + 0x2810;
    key.iLeftKey = a * b + 5;
    a = rand() % 0x84D0 + 0x2810; b = rand() % 0x7530 + 0x2810;
    key.iRightKey = a * b + 5;
    a = rand() % 0x84D0 + 0x2810; b = rand() % 0x7530 + 0x2810;
    key.iMiddleKey = a * b + 5;
    a = rand() % 0x84D0 + 0x2810; b = rand() % 0x7530 + 0x2810;
    key.iTotalKey = a * b + 5;
    key.iLeftMultiGab = rand() % 1000 + 1;
    key.iRightMultiGab = rand() % 1000 + 1;
    key.iMiddleMultiGab = rand() % 1000 + 1;
    key.iTotalMultiGab = rand() % 1000 + 1;
    key.iLeftPlusGab = rand() % 1000 + 1;
    key.iRightPlusGab = rand() % 1000 + 1;
    key.iMiddlePlusGab = rand() % 1000 + 1;
    key.iTotalPlusGab = rand() % 1000 + 1;
}

void CHSEL_STREAM::SetKeyCustom(HselKey key) {
    Init.Keys = key;
    iCRCValue = 0;
}

void CHSEL_STREAM::SetNextKey(void) {
    Init.Keys.iLeftKey   = Init.Keys.iLeftKey   * Init.Keys.iLeftMultiGab   + Init.Keys.iLeftPlusGab;
    Init.Keys.iRightKey  = Init.Keys.iRightKey  * Init.Keys.iRightMultiGab  + Init.Keys.iRightPlusGab;
    Init.Keys.iMiddleKey = Init.Keys.iMiddleKey * Init.Keys.iMiddleMultiGab + Init.Keys.iMiddlePlusGab;
}

void CHSEL_STREAM::GetCRC(char *lpStream, const __int32 iStreamSize) {
    int size = iStreamSize;
    int numBlocks = size >> 2;
    int remainBytes = size & 3;
    iCRCValue = 0;
    iBlockCount = numBlocks;
    iRemainCount = remainBytes;
    iPos = numBlocks * 4;
    __int32 *p = (__int32*)lpStream;
    for (int i = 0; i < numBlocks; i++) { iCRCValue ^= *p; p++; }
    char *pb = (char*)p;
    for (int i = 0; i < remainBytes; i++) { iCRCValue ^= (signed char)pb[i]; }
}

char CHSEL_STREAM::GetCRCConvertChar(void) const {
    char *p = (char*)&iCRCValue;
    return p[3] ^ p[2] ^ p[1] ^ p[0];
}

short CHSEL_STREAM::GetCRCConvertShort(void) const {
    short *p = (short*)&iCRCValue;
    return p[1] ^ p[0];
}

__int32 CHSEL_STREAM::GetCRCConvertInt(void) const { return iCRCValue; }

static void swap_positions(__int32 kl, __int32 kr, __int32 km, int n, int L[4], int R[4], int M[4]) {
    const __int32 keys[3] = { kl, kr, km };
    for (int i = 0; i < 4; i++) {
        L[i] = ((keys[0] >> (8 * i)) & 0xF) % n;
        R[i] = ((keys[1] >> (8 * i)) & 0xF) % n;
        M[i] = ((keys[2] >> (8 * i)) & 0xF) % n;
    }
}

void CHSEL_STREAM::SwapEncrypt(char *lpStream, const __int32 iSize) {
    int n = iSize >> 2;
    if (n < 5) return;
    n -= 1;
    int L[4], R[4], M[4];
    swap_positions(Init.Keys.iLeftKey, Init.Keys.iRightKey, Init.Keys.iMiddleKey, n, L, R, M);
    __int32 *p = (__int32*)lpStream, t;
    for (int i = 0; i < 4; i++) {
        if (L[i] != R[i]) { t = p[L[i]]; p[L[i]] = p[R[i]]; p[R[i]] = t; }
        if (R[i] != M[i]) { t = p[R[i]]; p[R[i]] = p[M[i]]; p[M[i]] = t; }
    }
}

void CHSEL_STREAM::SwapDecrypt(char *lpStream, const __int32 iSize) {
    int n = iSize >> 2;
    if (n < 5) return;
    n -= 1;
    int L[4], R[4], M[4];
    swap_positions(Init.Keys.iLeftKey, Init.Keys.iRightKey, Init.Keys.iMiddleKey, n, L, R, M);
    __int32 *p = (__int32*)lpStream, t;
    // SwapEncrypt 的严格逆序: G8..G1
    for (int i = 3; i >= 0; i--) {
        if (R[i] != M[i]) { t = p[R[i]]; p[R[i]] = p[M[i]]; p[M[i]] = t; }
        if (L[i] != R[i]) { t = p[L[i]]; p[L[i]] = p[R[i]]; p[R[i]] = t; }
    }
}

#define HSEL_OP_XOR 0
#define HSEL_OP_ADD 1
#define HSEL_OP_SUB 2

static inline __int32 blk_op(__int32 v, __int32 key, int op) {
    if (op == HSEL_OP_ADD) return v + key;
    if (op == HSEL_OP_SUB) return v - key;
    return v ^ key;
}
static inline __int32 blk_inv(__int32 v, __int32 key, int op) {
    if (op == HSEL_OP_ADD) return v - key;
    if (op == HSEL_OP_SUB) return v + key;
    return v ^ key;
}
static inline unsigned char byte_op(unsigned char v, unsigned char k, int op) {
    if (op == HSEL_OP_ADD) return (unsigned char)(v + k);
    if (op == HSEL_OP_SUB) return (unsigned char)(v - k);
    return (unsigned char)(v ^ k);
}
static inline unsigned char byte_inv(unsigned char v, unsigned char k, int op) {
    if (op == HSEL_OP_ADD) return (unsigned char)(v - k);
    if (op == HSEL_OP_SUB) return (unsigned char)(v + k);
    return (unsigned char)(v ^ k);
}

// Left: 起始对齐前向链 + 尾余数 (key 字节 [rem-j])
static void left_encode(char *s, int size, __int32 key, int blk_op_id, int rem_op_id) {
    int blocks = size >> 2, rem = size & 3;
    __int32 *p = (__int32*)s;
    __int32 workkey = key;
    for (int i = 0; i < blocks; i++) { p[i] = blk_op(p[i], workkey, blk_op_id); workkey = p[i]; }
    unsigned char *kb = (unsigned char*)&key;
    unsigned char *pb = (unsigned char*)(p + blocks);
    for (int j = 0; j < rem; j++) pb[j] = byte_op(pb[j], kb[rem - j], rem_op_id);
}
static void left_decode(char *s, int size, __int32 key, int blk_op_id, int rem_op_id) {
    int blocks = size >> 2, rem = size & 3;
    __int32 *p = (__int32*)s;
    for (int i = blocks - 1; i >= 1; i--) p[i] = blk_inv(p[i], p[i - 1], blk_op_id);
    if (blocks >= 1) p[0] = blk_inv(p[0], key, blk_op_id);
    unsigned char *kb = (unsigned char*)&key;
    unsigned char *pb = (unsigned char*)(p + blocks);
    for (int j = 0; j < rem; j++) pb[j] = byte_inv(pb[j], kb[rem - j], rem_op_id);
}

// Right: 末尾对齐反向链 (size>=4) + 头余数 (key 字节 [j], size<4 时仅余数)
static void right_encode(char *s, int size, __int32 key, int blk_op_id, int rem_op_id) {
    int blocks = size >> 2, rem = size & 3;
    __int32 workkey = key;
    char *ptr = s + size - 4;
    for (int i = 0; i < blocks; i++) {
        __int32 v = blk_op(*(__int32*)ptr, workkey, blk_op_id);
        *(__int32*)ptr = v;
        workkey = v;
        ptr -= 4;
    }
    unsigned char *kb = (unsigned char*)&key;
    for (int j = 0; j < rem; j++) s[j] = byte_op(s[j], kb[j], rem_op_id);
}
static void right_decode(char *s, int size, __int32 key, int blk_op_id, int rem_op_id) {
    int blocks = size >> 2, rem = size & 3;
    unsigned char *kb = (unsigned char*)&key;
    for (int j = 0; j < rem; j++) s[j] = byte_inv(s[j], kb[j], rem_op_id);
    if (blocks < 1) return;   // size<4: 无末对齐块 (越界写会破坏包头 size/crc 字段!)
    char *ptr = s + (size - 4) - 4 * (blocks - 1);
    for (int i = 0; i < blocks - 1; i++) {
        __int32 v = blk_inv(*(__int32*)ptr, *(__int32*)(ptr + 4), blk_op_id);
        *(__int32*)ptr = v;
        ptr += 4;
    }
    *(__int32*)ptr = blk_inv(*(__int32*)ptr, key, blk_op_id);
}

// Middle: 起始对齐 ECB + 尾余数 (key 字节 [rem-j])
static void middle_encode(char *s, int size, __int32 key, int blk_op_id, int rem_op_id) {
    int blocks = size >> 2, rem = size & 3;
    __int32 *p = (__int32*)s;
    for (int i = 0; i < blocks; i++) p[i] = blk_op(p[i], key, blk_op_id);
    unsigned char *kb = (unsigned char*)&key;
    unsigned char *pb = (unsigned char*)(p + blocks);
    for (int j = 0; j < rem; j++) pb[j] = byte_op(pb[j], kb[rem - j], rem_op_id);
}
static void middle_decode(char *s, int size, __int32 key, int blk_op_id, int rem_op_id) {
    int blocks = size >> 2, rem = size & 3;
    __int32 *p = (__int32*)s;
    for (int i = 0; i < blocks; i++) p[i] = blk_inv(p[i], key, blk_op_id);
    unsigned char *kb = (unsigned char*)&key;
    unsigned char *pb = (unsigned char*)(p + blocks);
    for (int j = 0; j < rem; j++) pb[j] = byte_inv(pb[j], kb[rem - j], rem_op_id);
}

void CHSEL_STREAM::DESLeftEncode_Type_1(char *s, __int32 n) { left_encode(s, n, Init.Keys.iLeftKey, HSEL_OP_XOR, HSEL_OP_XOR); }
void CHSEL_STREAM::DESRightEncode_Type_1(char *s, __int32 n) { right_encode(s, n, Init.Keys.iRightKey, HSEL_OP_XOR, HSEL_OP_XOR); }
void CHSEL_STREAM::DESMiddleEncode_Type_1(char *s, __int32 n) { middle_encode(s, n, Init.Keys.iMiddleKey, HSEL_OP_XOR, HSEL_OP_XOR); }
void CHSEL_STREAM::DESLeftDecode_Type_1(char *s, __int32 n) { left_decode(s, n, Init.Keys.iLeftKey, HSEL_OP_XOR, HSEL_OP_XOR); }
void CHSEL_STREAM::DESRightDecode_Type_1(char *s, __int32 n) { right_decode(s, n, Init.Keys.iRightKey, HSEL_OP_XOR, HSEL_OP_XOR); }
void CHSEL_STREAM::DESMiddleDecode_Type_1(char *s, __int32 n) { middle_decode(s, n, Init.Keys.iMiddleKey, HSEL_OP_XOR, HSEL_OP_XOR); }
void CHSEL_STREAM::DESLeftEncode_Type_2(char *s, __int32 n) { left_encode(s, n, Init.Keys.iLeftKey, HSEL_OP_ADD, HSEL_OP_ADD); }
void CHSEL_STREAM::DESRightEncode_Type_2(char *s, __int32 n) { right_encode(s, n, Init.Keys.iRightKey, HSEL_OP_ADD, HSEL_OP_ADD); }
void CHSEL_STREAM::DESMiddleEncode_Type_2(char *s, __int32 n) { middle_encode(s, n, Init.Keys.iMiddleKey, HSEL_OP_ADD, HSEL_OP_ADD); }
void CHSEL_STREAM::DESLeftDecode_Type_2(char *s, __int32 n) { left_decode(s, n, Init.Keys.iLeftKey, HSEL_OP_ADD, HSEL_OP_ADD); }
void CHSEL_STREAM::DESRightDecode_Type_2(char *s, __int32 n) { right_decode(s, n, Init.Keys.iRightKey, HSEL_OP_ADD, HSEL_OP_ADD); }
void CHSEL_STREAM::DESMiddleDecode_Type_2(char *s, __int32 n) { middle_decode(s, n, Init.Keys.iMiddleKey, HSEL_OP_ADD, HSEL_OP_ADD); }
void CHSEL_STREAM::DESLeftEncode_Type_3(char *s, __int32 n) { left_encode(s, n, Init.Keys.iLeftKey, HSEL_OP_SUB, HSEL_OP_SUB); }
void CHSEL_STREAM::DESRightEncode_Type_3(char *s, __int32 n) { right_encode(s, n, Init.Keys.iRightKey, HSEL_OP_SUB, HSEL_OP_SUB); }
void CHSEL_STREAM::DESMiddleEncode_Type_3(char *s, __int32 n) { middle_encode(s, n, Init.Keys.iMiddleKey, HSEL_OP_SUB, HSEL_OP_SUB); }
void CHSEL_STREAM::DESLeftDecode_Type_3(char *s, __int32 n) { left_decode(s, n, Init.Keys.iLeftKey, HSEL_OP_SUB, HSEL_OP_SUB); }
void CHSEL_STREAM::DESRightDecode_Type_3(char *s, __int32 n) { right_decode(s, n, Init.Keys.iRightKey, HSEL_OP_SUB, HSEL_OP_SUB); }
void CHSEL_STREAM::DESMiddleDecode_Type_3(char *s, __int32 n) { middle_decode(s, n, Init.Keys.iMiddleKey, HSEL_OP_SUB, HSEL_OP_SUB); }
void CHSEL_STREAM::DESLeftEncode_Type_4(char *s, __int32 n) { left_encode(s, n, Init.Keys.iLeftKey, HSEL_OP_SUB, HSEL_OP_XOR); }
void CHSEL_STREAM::DESRightEncode_Type_4(char *s, __int32 n) { right_encode(s, n, Init.Keys.iRightKey, HSEL_OP_ADD, HSEL_OP_XOR); }
void CHSEL_STREAM::DESMiddleEncode_Type_4(char *s, __int32 n) { middle_encode(s, n, Init.Keys.iMiddleKey, HSEL_OP_XOR, HSEL_OP_ADD); }
void CHSEL_STREAM::DESLeftDecode_Type_4(char *s, __int32 n) { left_decode(s, n, Init.Keys.iLeftKey, HSEL_OP_SUB, HSEL_OP_XOR); }
void CHSEL_STREAM::DESRightDecode_Type_4(char *s, __int32 n) { right_decode(s, n, Init.Keys.iRightKey, HSEL_OP_ADD, HSEL_OP_XOR); }
void CHSEL_STREAM::DESMiddleDecode_Type_4(char *s, __int32 n) { middle_decode(s, n, Init.Keys.iMiddleKey, HSEL_OP_XOR, HSEL_OP_ADD); }

void CHSEL_STREAM::NoSwapEncrypt(char *, const __int32) {}
void CHSEL_STREAM::NoSwapDecrypt(char *, const __int32) {}

void CHSEL_STREAM::DESSingleEncode(char *s, __int32 n) { (this->*lpDesLeftEncrypt)(s, n); }
void CHSEL_STREAM::DESSingleDecode(char *s, __int32 n) { (this->*lpDesLeftDecrypt)(s, n); }

void CHSEL_STREAM::DESTripleEncode(char *s, __int32 n) {
    (this->*lpDesLeftEncrypt)(s, n);
    (this->*lpDesRightEncrypt)(s, n);
    (this->*lpDesMiddleEncrypt)(s, n);
}
void CHSEL_STREAM::DESTripleDecode(char *s, __int32 n) {
    (this->*lpDesMiddleDecrypt)(s, n);
    (this->*lpDesRightDecrypt)(s, n);
    (this->*lpDesLeftDecrypt)(s, n);
}

bool CHSEL_STREAM::Encrypt(char *lpStream, const __int32 iStreamSize) {
    if (!lpStream) return false;
    if (!ChackFaultStreamSize(iStreamSize)) return false;
    (this->*lpSwapEncrypt)(lpStream, iStreamSize);
    (this->*lpDesEncryptType)(lpStream, iStreamSize);
    SetNextKey();
    GetCRC(lpStream, iStreamSize);
    return true;
}

bool CHSEL_STREAM::Decrypt(char *lpStream, const __int32 iStreamSize) {
    if (!lpStream) return false;
    if (!ChackFaultStreamSize(iStreamSize)) return false;
    GetCRC(lpStream, iStreamSize);
    (this->*lpDesDecryptType)(lpStream, iStreamSize);
    (this->*lpSwapDecrypt)(lpStream, iStreamSize);
    SetNextKey();
    return true;
}
