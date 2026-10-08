// Detailed XOR chain debug
#include <stdio.h>
#include <string.h>

void xor_encode_debug(char *data, int size, int *key) {
    int n = size / 4, r = size % 4;
    int *p = (int*)data;
    printf("  ENCODE: n=%d r=%d key=%d\n", n, r, *key);
    for (int i = 0; i < n; i++) {
        int orig = *p;
        int v = *p ^ *key;
        printf("    block[%d]: data=%08X key=%08X -> enc=%08X\n", i, orig, *key, v);
        *p = v;
        *key = v;
        p++;
    }
    char *pb = (char*)p;
    char *kb = (char*)key;
    for (int i = 0; i < r; i++) {
        printf("    remain[%d]: data=%02X key=%02X -> enc=%02X\n", i, (unsigned char)pb[i], (unsigned char)kb[i], (unsigned char)(pb[i]^kb[i]));
        pb[i] ^= kb[i];
    }
}

void xor_decode_debug(char *data, int size, int key) {
    int n = size / 4, r = size % 4;
    int *p = (int*)data;
    printf("  DECODE: n=%d r=%d key=%08X\n", n, r, key);
    
    if (n > 1) {
        int prev = *(p - 1);  // Load from BEFORE the data
        printf("    initial prev (from before data): %08X\n", prev);
        for (int i = n - 1; i >= 1; i--) {
            int cur = p[i];
            int dec = cur ^ prev;
            printf("    block[%d]: enc=%08X prev=%08X -> dec=%08X\n", i, cur, prev, dec);
            p[i] = dec;
            prev = cur;
        }
        // First block
        int first = *p;
        int dec = first ^ key;
        printf("    block[0]: enc=%08X key=%08X -> dec=%08X\n", first, key, dec);
        *p = dec;
    } else if (n == 1) {
        int first = *p;
        int dec = first ^ key;
        printf("    block[0]: enc=%08X key=%08X -> dec=%08X\n", first, key, dec);
        *p = dec;
    }
    
    if (r > 0) {
        char *pb = (char*)(p + n);
        char *kb = (char*)&key;
        for (int i = 0; i < r; i++) {
            printf("    remain[%d]: data=%02X key=%02X -> dec=%02X\n", i, (unsigned char)pb[i], (unsigned char)kb[i], (unsigned char)(pb[i]^kb[i]));
            pb[i] ^= kb[i];
        }
    }
}

int main() {
    int key = 100;
    
    printf("=== Encode 8 bytes ===\n");
    char data[] = {0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88};
    int key_save = key;
    xor_encode_debug(data, 8, &key);
    printf("  Key after enc: %d (%08X)\n", key, key);
    
    printf("\n=== Decode 8 bytes ===\n");
    xor_decode_debug(data, 8, key_save);
    
    printf("\n=== Expected: 11 22 33 44 55 66 77 88 ===\n");
    printf("=== Got:      ");
    for (int i = 0; i < 8; i++) printf("%02X ", (unsigned char)data[i]);
    printf(" ===\n");
    
    return 0;
}
