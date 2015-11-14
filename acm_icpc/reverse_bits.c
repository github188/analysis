#include <stdio.h>
#include <stdint.h>


uint8_t exchange_bit(uint8_t x)
{
    static const uint8_t MASK = 0x03; // 00000011
    uint8_t rslt;
    int i;

    rslt = 0;
    for (i = 0; i < 4; ++i) {
        uint8_t maski = (MASK << (i * 2));
        uint8_t xi = (x & maski);
        rslt |= (((xi >> 1) | (xi << 1)) & maski);
    }

    return rslt;
}

uint8_t reverse_8bit(uint8_t x)
{
    uint8_t ebn = exchange_bit(x);
    uint8_t ebn_left = (ebn << 4);
    uint8_t ebn_right = (ebn >> 4);

    ebn_left = (((ebn_left << 2) | (ebn_left >> 2)) & 0xF0);
    ebn_right = (((ebn_right << 2) | (ebn_right >> 2)) & 0x0F);

    return ebn_left | ebn_right;
}

uint32_t reverseBits(uint32_t n)
{
    int i;
    uint8_t tmp;
    union {
        uint32_t x32;
        uint8_t x8[4];
    } un;

    un.x32 = n;
    tmp = un.x8[0];
    un.x8[0] = un.x8[3];
    un.x8[3] = tmp;
    tmp = un.x8[1];
    un.x8[1] = un.x8[2];
    un.x8[2] = tmp;

    for (i = 0; i < 4; ++i) {
        un.x8[i] = reverse_8bit(un.x8[i]);
    }

    return un.x32;
}

uint32_t reverseBits2(uint32_t n)
{
    uint32_t res=0;

    for (int i=0; i<32; i++) {
        res|=((n >> i) & 1) << (31 - i);
    }

    return res;
}


int main(int argc, char *argv[])
{
    uint32_t x = 0x12345678;

    fprintf(stderr, "0x%x\n", reverseBits(x));
    fprintf(stderr, "0x%x\n", reverseBits2(x));

    return 0;
}
