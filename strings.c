unsigned long hex2ascii(unsigned int n) {
        uint8_t t1,t2,t3;
        t1 = '0';
        t2 = '0';
        t3 = '0';
        while (n>=100) {
                t1++;
                n-=100;
        }
        while (n>=10) {
                t2++;
                n-=10;
        }
        t3 += (n & 0xFF);
        return (((unsigned long)t1)<<16) | (((unsigned long)t2)<<8) | (unsigned long)t3;
}

uint16_t hex2bcd(uint8_t n) {
        uint8_t bcd[2];
        bcd[0] = n & 0x0F;
        bcd[1] = (n & 0xF0)>>4;
        if (bcd[0]<10) {
                bcd[0]+='0';
        } else {
                bcd[0] = 'A' + (bcd[0]-10);
        }
        if (bcd[1]<10) {
                bcd[1]+='0';
        } else {
                bcd[1] = 'A' + (bcd[1]-10);
        }

        return (bcd[0]<<8) | bcd[1];

}

