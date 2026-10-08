// Pure C XOR chain test - no class overhead
#include <stdio.h>
#include <string.h>

void xor_encode(char *data, int size, int *key) {
    int n = size / 4, r = size % 4;
    int *p = (int*)data;
    for (int i = 0; i < n; i++) {
        int v = *p ^ *key;
        *p = v;
        *key = v;
        p++;
    }
    char *pb = (char*)p;
    char *kb = (char*)key;
    for (int i = 0; i < r; i++) pb[i] ^= kb[i];
}

void xor_decode(char *data, int size, int key) {
    int n = size / 4, r = size % 4;
    int *p = (int*)data;
    if (n > 1) {
        int prev = *(p - 1);
        for (int i = n - 1; i >= 1; i--) {
            int cur = p[i];
            p[i] = cur ^ prev;
            prev = cur;
        }
        *p ^= key;
    } else if (n == 1) {
        *p ^= key;
    }
    if (r > 0) {
        char *pb = (char*)(p + n);
        char *kb = (char*)&key;
        for (int i = 0; i < r; i++) pb[i] ^= kb[i];
    }
}

void hexdump(const char* label, unsigned char* data, int len) {
    printf("  %s: ", label);
    for (int i = 0; i < len; i++) printf("%02X ", data[i]);
    printf("\n");
}

int main() {
    int key = 100;
    
    // Test 8 bytes
    printf("=== Test 8 bytes ===\n");
    char data[] = {0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88};
    char orig[8]; memcpy(orig, data, 8);
    int key_enc = key;
    xor_encode(data, 8, &key_enc);
    printf("Encrypted: "); hexdump("", (unsigned char*)data, 8);
    printf("Key after enc: %d\n", key_enc);
    
    int key_dec = key;
    xor_decode(data, 8, key_dec);
    printf("Decrypted: "); hexdump("", (unsigned char*)data, 8);
    printf("Match: %s\n", memcmp(data, orig, 8) == 0 ? "PASS" : "FAIL");
    
    // Test 5 bytes
    printf("\n=== Test 5 bytes ===\n");
    char data2[] = {0xAA,0xBB,0xCC,0xDD, 0x11};
    char orig2[5]; memcpy(orig2, data2, 5);
    int key_enc2 = key;
    xor_encode(data2, 5, &key_enc2);
    printf("Encrypted: "); hexdump("", (unsigned char*)data2, 5);
    
    int key_dec2 = key;
    xor_decode(data2, 5, key_dec2);
    printf("Decrypted: "); hexdump("", (unsigned char*)data2, 5);
    printf("Match: %s\n", memcmp(data2, orig2, 5) == 0 ? "PASS" : "FAIL");
    
    return 0;
}
