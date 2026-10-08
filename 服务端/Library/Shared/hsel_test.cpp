// HSEL Roundtrip Test - Verify encrypt/decrypt compatibility
// Compile: cl /EHsc /I..\Client\Library\Shared hsel_test.cpp ..\Client\Library\Shared\HSEL.cpp /link

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "HSEL.h"

int main() {
    printf("=== HSEL Roundtrip Test ===\n\n");
    
    // Test 1: Basic roundtrip with known keys
    {
        printf("Test 1: Single DES, Type 1, NoSwap\n");
        CHSEL_STREAM hsel;
        HselInit init;
        memset(&init, 0, sizeof(init));
        init.iDesCount = HSEL_DES_SINGLE;
        init.iEncryptType = HSEL_ENCRYPTTYPE_1;
        init.iSwapFlag = HSEL_SWAP_FLAG_ON;
        init.iCustomize = HSEL_KEY_TYPE_DEFAULT;
        init.Keys.iLeftKey = 12345;
        init.Keys.iRightKey = 67890;
        init.Keys.iMiddleKey = 11111;
        init.Keys.iLeftMultiGab = 100;
        init.Keys.iRightMultiGab = 200;
        init.Keys.iMiddleMultiGab = 300;
        init.Keys.iLeftPlusGab = 10;
        init.Keys.iRightPlusGab = 20;
        init.Keys.iMiddlePlusGab = 30;
        
        hsel.Initial(init);
        
        char data[] = "Hello World! This is a test of HSEL encryption roundtrip.";
        int len = strlen(data);
        char original[256];
        memcpy(original, data, len);
        
        printf("  Original:  %s\n", data);
        
        hsel.Encrypt(data, len);
        short crc_after_enc = hsel.GetCRCConvertShort();
        printf("  Encrypted: (CRC=%d)\n", crc_after_enc);
        
        // Print first few bytes
        printf("  Encrypted bytes: ");
        for (int i = 0; i < 16 && i < len; i++) printf("%02X ", (unsigned char)data[i]);
        printf("...\n");
        
        hsel.Decrypt(data, len);
        short crc_after_dec = hsel.GetCRCConvertShort();
        printf("  CRC after decrypt: %d\n", crc_after_dec);
        
        if (memcmp(data, original, len) == 0) {
            printf("  PASS: Roundtrip successful!\n\n");
        } else {
            printf("  FAIL: Data mismatch after decrypt!\n");
            printf("  Original:  ");
            for (int i = 0; i < len; i++) printf("%02X ", (unsigned char)original[i]);
            printf("\n  Decrypted: ");
            for (int i = 0; i < len; i++) printf("%02X ", (unsigned char)data[i]);
            printf("\n\n");
        }
    }
    
    // Test 2: Triple DES, Type 4, NoSwap
    {
        printf("Test 2: Triple DES, Type 4, NoSwap\n");
        CHSEL_STREAM hsel;
        HselInit init;
        memset(&init, 0, sizeof(init));
        init.iDesCount = HSEL_DES_TRIPLE;
        init.iEncryptType = HSEL_ENCRYPTTYPE_4;
        init.iSwapFlag = HSEL_SWAP_FLAG_ON;
        init.iCustomize = HSEL_KEY_TYPE_CUSTOMIZE;
        init.Keys.iLeftKey = 799107505;
        init.Keys.iRightKey = 910992065;
        init.Keys.iMiddleKey = 496336527;
        init.Keys.iLeftMultiGab = 500;
        init.Keys.iRightMultiGab = 600;
        init.Keys.iMiddleMultiGab = 700;
        init.Keys.iLeftPlusGab = 50;
        init.Keys.iRightPlusGab = 60;
        init.Keys.iMiddlePlusGab = 70;
        
        hsel.Initial(init);
        
        char data[] = "Test data for triple DES verification with type 4 algorithm.";
        int len = strlen(data);
        char original[256];
        memcpy(original, data, len);
        
        printf("  Original:  %s\n", data);
        
        hsel.Encrypt(data, len);
        short crc_enc = hsel.GetCRCConvertShort();
        printf("  Encrypted (CRC=%d): ", crc_enc);
        for (int i = 0; i < 16 && i < len; i++) printf("%02X ", (unsigned char)data[i]);
        printf("...\n");
        
        hsel.Decrypt(data, len);
        short crc_dec = hsel.GetCRCConvertShort();
        printf("  CRC after decrypt: %d\n", crc_dec);
        
        if (memcmp(data, original, len) == 0) {
            printf("  PASS: Roundtrip successful!\n\n");
        } else {
            printf("  FAIL: Data mismatch!\n\n");
        }
    }
    
    // Test 3: With Swap enabled
    {
        printf("Test 3: Triple DES, Type 4, WithSwap\n");
        CHSEL_STREAM hsel;
        HselInit init;
        memset(&init, 0, sizeof(init));
        init.iDesCount = HSEL_DES_TRIPLE;
        init.iEncryptType = HSEL_ENCRYPTTYPE_4;
        init.iSwapFlag = HSEL_SWAP_FLAG_ON;  // Swap ON
        init.iCustomize = HSEL_KEY_TYPE_CUSTOMIZE;
        init.Keys.iLeftKey = 117446496;
        init.Keys.iRightKey = 770092224;
        init.Keys.iMiddleKey = 702976613;
        init.Keys.iLeftMultiGab = 300;
        init.Keys.iRightMultiGab = 400;
        init.Keys.iMiddleMultiGab = 500;
        init.Keys.iLeftPlusGab = 30;
        init.Keys.iRightPlusGab = 40;
        init.Keys.iMiddlePlusGab = 50;
        
        hsel.Initial(init);
        
        char data[] = "Another test with swap enabled for verification.";
        int len = strlen(data);
        char original[256];
        memcpy(original, data, len);
        
        printf("  Original:  %s\n", data);
        
        hsel.Encrypt(data, len);
        printf("  Encrypted: ");
        for (int i = 0; i < 16 && i < len; i++) printf("%02X ", (unsigned char)data[i]);
        printf("...\n");
        
        hsel.Decrypt(data, len);
        
        if (memcmp(data, original, len) == 0) {
            printf("  PASS: Roundtrip successful!\n\n");
        } else {
            printf("  FAIL: Data mismatch!\n\n");
        }
    }
    
    // Test 4: Read actual file and test
    {
        printf("Test 4: Read Thunder.dat header\n");
        FILE *fp = fopen("../../客户端/dragonraja-online/effect/Thunder.dat", "rb");
        if (!fp) {
            printf("  Cannot open Thunder.dat\n\n");
        } else {
            int version;
            fread(&version, sizeof(int), 1, fp);
            printf("  File version: %d\n", version);
            
            HselInit init;
            fread(&init, sizeof(HselInit), 1, fp);
            printf("  Init: des=%d enc=%d swap=%d customize=%d\n", 
                   init.iDesCount, init.iEncryptType, init.iSwapFlag, init.iCustomize);
            printf("  Keys: L=%d R=%d M=%d\n", init.Keys.iLeftKey, init.Keys.iRightKey, init.Keys.iMiddleKey);
            printf("  MultiGab: L=%d R=%d M=%d\n", init.Keys.iLeftMultiGab, init.Keys.iRightMultiGab, init.Keys.iMiddleMultiGab);
            printf("  PlusGab: L=%d R=%d M=%d\n", init.Keys.iLeftPlusGab, init.Keys.iRightPlusGab, init.Keys.iMiddlePlusGab);
            
            fclose(fp);
            printf("\n");
        }
    }
    
    printf("=== Tests Complete ===\n");
    return 0;
}
