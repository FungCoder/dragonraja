// HSEL Detailed Diagnostic Test
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "HSEL.h"

// Dump hex bytes
void hexdump(const char* label, const unsigned char* data, int len) {
    printf("  %s: ", label);
    for (int i = 0; i < len && i < 32; i++) printf("%02X ", data[i]);
    if (len > 32) printf("...");
    printf("\n");
}

int main() {
    printf("=== HSEL Detailed Diagnostic ===\n\n");
    
    // Create test with known simple data
    CHSEL_STREAM enc_hsel, dec_hsel;
    HselInit init;
    memset(&init, 0, sizeof(init));
    init.iDesCount = HSEL_DES_SINGLE;
    init.iEncryptType = HSEL_ENCRYPTTYPE_1;
    init.iSwapFlag = HSEL_SWAP_FLAG_ON;
    init.iCustomize = HSEL_KEY_TYPE_CUSTOMIZE;
    init.Keys.iLeftKey = 100;
    init.Keys.iRightKey = 200;
    init.Keys.iMiddleKey = 300;
    init.Keys.iLeftMultiGab = 3;
    init.Keys.iRightMultiGab = 5;
    init.Keys.iMiddleMultiGab = 7;
    init.Keys.iLeftPlusGab = 11;
    init.Keys.iRightPlusGab = 13;
    init.Keys.iMiddlePlusGab = 17;
    
    enc_hsel.Initial(init);
    dec_hsel.Initial(init);
    
    // Test with 8 bytes (2 DWORD blocks)
    char data[] = {0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88};
    char orig[8];
    memcpy(orig, data, 8);
    
    printf("Original:     "); hexdump("", (unsigned char*)data, 8);
    
    // Encrypt
    enc_hsel.Encrypt(data, 8);
    printf("After Encrypt:"); hexdump("", (unsigned char*)data, 8);
    printf("CRC after enc: %d\n", enc_hsel.GetCRCConvertShort());
    
    // Decrypt with SAME hsel instance (simulating Load behavior)
    dec_hsel.Decrypt(data, 8);
    printf("After Decrypt:"); hexdump("", (unsigned char*)data, 8);
    printf("CRC after dec: %d\n", dec_hsel.GetCRCConvertShort());
    
    if (memcmp(data, orig, 8) == 0)
        printf("\nRESULT: PASS\n");
    else {
        printf("\nRESULT: FAIL\n");
        printf("Expected:    "); hexdump("", (unsigned char*)orig, 8);
        printf("Got:         "); hexdump("", (unsigned char*)data, 8);
    }
    
    // Test with separate enc/dec instances using SAME initial state
    printf("\n--- Test with fresh instances ---\n");
    CHSEL_STREAM enc2, dec2;
    enc2.Initial(init);
    dec2.Initial(init);
    
    char data2[] = {0xAA,0xBB,0xCC,0xDD, 0x11,0x22,0x33,0x44};
    char orig2[8];
    memcpy(orig2, data2, 8);
    
    enc2.Encrypt(data2, 8);
    printf("Encrypted: "); hexdump("", (unsigned char*)data2, 8);
    
    dec2.Decrypt(data2, 8);
    printf("Decrypted: "); hexdump("", (unsigned char*)data2, 8);
    
    if (memcmp(data2, orig2, 8) == 0)
        printf("PASS\n");
    else
        printf("FAIL\n");
    
    // Test with larger data (16 bytes = 4 blocks)
    printf("\n--- Test with 16 bytes ---\n");
    CHSEL_STREAM enc3, dec3;
    enc3.Initial(init);
    dec3.Initial(init);
    
    char data3[] = {0x01,0x02,0x03,0x04, 0x05,0x06,0x07,0x08,
                     0x09,0x0A,0x0B,0x0C, 0x0D,0x0E,0x0F,0x10};
    char orig3[16];
    memcpy(orig3, data3, 16);
    
    enc3.Encrypt(data3, 16);
    printf("Encrypted: "); hexdump("", (unsigned char*)data3, 16);
    
    dec3.Decrypt(data3, 16);
    printf("Decrypted: "); hexdump("", (unsigned char*)data3, 16);
    
    if (memcmp(data3, orig3, 16) == 0)
        printf("PASS\n");
    else
        printf("FAIL\n");
    
    // Test with 12 bytes (3 blocks + no remainder)
    printf("\n--- Test with 12 bytes ---\n");
    CHSEL_STREAM enc4, dec4;
    enc4.Initial(init);
    dec4.Initial(init);
    
    char data4[] = {0xAA,0xBB,0xCC,0xDD, 0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88};
    char orig4[12];
    memcpy(orig4, data4, 12);
    
    enc4.Encrypt(data4, 12);
    dec4.Decrypt(data4, 12);
    
    if (memcmp(data4, orig4, 12) == 0)
        printf("PASS\n");
    else
        printf("FAIL\n");
    
    // Test with 5 bytes (1 block + 1 remainder)
    printf("\n--- Test with 5 bytes (1 block + 1 remainder) ---\n");
    CHSEL_STREAM enc5, dec5;
    enc5.Initial(init);
    dec5.Initial(init);
    
    char data5[] = {0xAA,0xBB,0xCC,0xDD, 0x11};
    char orig5[5];
    memcpy(orig5, data5, 5);
    
    enc5.Encrypt(data5, 5);
    dec5.Decrypt(data5, 5);
    
    if (memcmp(data5, orig5, 5) == 0)
        printf("PASS\n");
    else {
        printf("FAIL\n");
        printf("Expected: "); hexdump("", (unsigned char*)orig5, 5);
        printf("Got:      "); hexdump("", (unsigned char*)data5, 5);
    }
    
    // Test with 4 bytes (1 block, no remainder)
    printf("\n--- Test with 4 bytes (1 block) ---\n");
    CHSEL_STREAM enc6, dec6;
    enc6.Initial(init);
    dec6.Initial(init);
    
    char data6[] = {0xAA,0xBB,0xCC,0xDD};
    char orig6[4];
    memcpy(orig6, data6, 4);
    
    enc6.Encrypt(data6, 4);
    dec6.Decrypt(data6, 4);
    
    if (memcmp(data6, orig6, 4) == 0)
        printf("PASS\n");
    else
        printf("FAIL\n");
    
    // Test with 3 bytes (< 1 block)
    printf("\n--- Test with 3 bytes (< 1 block) ---\n");
    CHSEL_STREAM enc7, dec7;
    enc7.Initial(init);
    dec7.Initial(init);
    
    char data7[] = {0xAA,0xBB,0xCC};
    char orig7[3];
    memcpy(orig7, data7, 3);
    
    enc7.Encrypt(data7, 3);
    dec7.Decrypt(data7, 3);
    
    if (memcmp(data7, orig7, 3) == 0)
        printf("PASS\n");
    else
        printf("FAIL\n");
    
    return 0;
}
