// moddes.c -- Modified DES (single-block demo)
// Compile: gcc -O2 -std=c11 -o moddes moddes.c

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Standard DES tables
static const int IP[64] = {
58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7
};
static const int FP[64] = {
40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,
38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,
36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25
};
static const int E[48] = {
32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,
12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,
24,25,26,27,28,29,28,29,30,31,32,1
};
static const int P[32] = {
16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,
2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25
};
static const int PC1[56] = {
57,49,41,33,25,17,9,1,58,50,42,34,26,18,
10,2,59,51,43,35,27,19,11,3,60,52,44,36,
63,55,47,39,31,23,15,7,62,54,46,38,30,22,
14,6,61,53,45,37,29,21,13,5,28,20,12,4
};
static const int PC2[48] = {
14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,
26,8,16,7,27,20,13,2,41,52,31,37,47,55,30,40,
51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32
};
static const int SHIFTS[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
static const int SBOX[8][64] = {
{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8}
};

// bit helpers (pos_from1 = 1..64, MSB at 1)
static inline int getbit64(uint64_t v, int pos_from1) {
return (int)((v >> (64 - pos_from1)) & 1ULL);
}
static inline void setbit64(uint64_t *v, int pos_from1, int b) {
if (b) *v |= (1ULL << (64 - pos_from1));
else *v &= ~(1ULL << (64 - pos_from1));
}

// permute: input left-aligned; returns left-aligned output (bits occupy MSB side)
static uint64_t permute64(uint64_t in_left, const int *table, int n) {
uint64_t out = 0;
for (int i = 0; i < n; ++i) {
int b = getbit64(in_left, table[i]);
out = (out << 1) | (uint64_t)b;
}
// left-align: shift to MSB
if (n < 64) out <<= (64 - n);
return out;
}

// expand 32->48: R is low-aligned 32-bit, produce low-aligned 48-bit
static uint64_t expand32_to48(uint32_t R) {
uint64_t left = ((uint64_t)R) << 32; // left-align 32-bit in 64-bit
uint64_t out = 0;
for (int i = 0; i < 48; ++i) {
int b = getbit64(left, E[i]);
out = (out << 1) | (uint64_t)b;
}
out &= ((1ULL<<48)-1);
return out;
}

// S-box substitution: x48 low-aligned -> returns low-aligned 32-bit
static uint32_t sbox48to32(uint64_t x48) {
uint32_t out = 0;
for (int i = 0; i < 8; ++i) {
int shift = (7 - i) * 6;
uint8_t six = (x48 >> shift) & 0x3F;
int row = ((six & 0x20) >> 4) | (six & 0x01);
int col = (six >> 1) & 0x0F;
uint8_t val = (uint8_t)SBOX[i][row*16 + col];
out = (out << 4) | (uint32_t)(val & 0x0F);
}
return out;
}

// P permutation: input low-aligned 32-bit -> output low-aligned 32-bit
static uint32_t permute32(uint32_t in32) {
uint64_t left = ((uint64_t)in32) << 32;
uint32_t out = 0;
for (int i = 0; i < 32; ++i) {
int b = getbit64(left, P[i]);
out = (out << 1) | (uint32_t)b;
}
return out;
}

// Key schedule: produce 16 low-aligned 48-bit subkeys
static void des_key_schedule(uint64_t key64, uint64_t subkeys[16]) {
uint64_t perm56 = 0;
for (int i = 0; i < 56; ++i) {
int b = getbit64(key64, PC1[i]);
perm56 = (perm56 << 1) | (uint64_t)b;
}
uint32_t C = (uint32_t)((perm56 >> 28) & ((1u<<28)-1));
uint32_t D = (uint32_t)(perm56 & ((1u<<28)-1));
for (int r = 0; r < 16; ++r) {
int s = SHIFTS[r];
C = ((C << s) | (C >> (28 - s))) & ((1u<<28)-1);
D = ((D << s) | (D >> (28 - s))) & ((1u<<28)-1);
uint64_t CD = (((uint64_t)C) << 28) | D;
uint64_t k48 = 0;
for (int i = 0; i < 48; ++i) {
int idx = PC2[i];
int b = (int)((CD >> (56 - idx)) & 1ULL);
k48 = (k48 << 1) | (uint64_t)b;
}
subkeys[r] = k48 & ((1ULL<<48)-1); // low-aligned 48-bit
}
}

// Xo on 48-bit low-aligned values: split into s1,s2,s3 (sum=48), XOR each segment, reassemble low-aligned
static uint64_t Xo48(uint64_t A, uint64_t B, int s1, int s2, int s3) {
uint64_t mask1 = (s1==64? ~0ULL : ((1ULL<<s1)-1));
uint64_t mask2 = (s2==64? ~0ULL : ((1ULL<<s2)-1));
uint64_t mask3 = (s3==64? ~0ULL : ((1ULL<<s3)-1));
uint64_t a1 = (A >> (s2 + s3)) & mask1;
uint64_t a2 = (A >> s3) & mask2;
uint64_t a3 = A & mask3;
uint64_t b1 = (B >> (s2 + s3)) & mask1;
uint64_t b2 = (B >> s3) & mask2;
uint64_t b3 = B & mask3;
uint64_t r1 = a1 ^ b1;
uint64_t r2 = a2 ^ b2;
uint64_t r3 = a3 ^ b3;
uint64_t res = (r1 << (s2 + s3)) | (r2 << s3) | r3;
return res & ((1ULL<<48)-1);
}

// Xo on 32-bit: scale s1,s2,s3 proportions to 32 bits (simple proportional scaling)
static uint32_t Xo32(uint32_t A, uint32_t B, int s1_48, int s2_48, int s3_48) {
int s1 = (s1_48 * 32) / 48; if (s1<1) s1=1;
int s2 = (s2_48 * 32) / 48; if (s2<1) s2=1;
int s3 = 32 - s1 - s2; if (s3<1) { s3=1; if (s1+s2+s3>32) s2=32-s1-s3; }
uint32_t mask1 = (s1==32? 0xFFFFFFFFu : ((1u<<s1)-1));
uint32_t mask2 = (s2==32? 0xFFFFFFFFu : ((1u<<s2)-1));
uint32_t mask3 = (s3==32? 0xFFFFFFFFu : ((1u<<s3)-1));
uint32_t a1 = (A >> (s2 + s3)) & mask1;
uint32_t a2 = (A >> s3) & mask2;
uint32_t a3 = A & mask3;
uint32_t b1 = (B >> (s2 + s3)) & mask1;
uint32_t b2 = (B >> s3) & mask2;
uint32_t b3 = B & mask3;
uint32_t r1 = a1 ^ b1;
uint32_t r2 = a2 ^ b2;
uint32_t r3 = a3 ^ b3;
uint32_t res = (r1 << (s2 + s3)) | (r2 << s3) | r3;
return res;
}

// Modified DES encrypt single block P (64-bit), K (64-bit), segment sizes for Xo on 48-bit: s1,s2,s3
uint64_t moddes_encrypt(uint64_t P, uint64_t K,
int s1, int s2, int s3,
int s1p, int s2p, int s3p)
{
// Initial Permutation (input left-aligned)
uint64_t ip = permute64(P, IP, 64); // left-aligned 64-bit
uint32_t L = (uint32_t)(ip >> 32);
uint32_t R = (uint32_t)(ip & 0xFFFFFFFFu);

// key schedule
uint64_t subkeys[16];
des_key_schedule(K, subkeys);

for (int r = 0; r < 16; ++r) {
uint32_t Lprev = L;
L = R;
// F-function
uint64_t T48 = expand32_to48(R); // low-aligned 48
uint64_t K48 = subkeys[r]; // low-aligned 48
uint64_t Tx = Xo48(T48, K48, s1, s2, s3); // Xo instead of XOR in mixing
uint32_t S32 = sbox48to32(Tx);
uint32_t P32 = permute32(S32);
// combine Li-1 Xo P_out (32-bit)
uint32_t Ri = Xo32(Lprev, P32, s1, s2, s3);
R = Ri;
}

// preoutput swap (R || L)
uint64_t pre = (((uint64_t)R) << 32) | (uint64_t)L; // left-aligned representation
uint64_t cipher = permute64(pre, FP, 64); // left-aligned result
return cipher;
}

// print hex 64
static void print_hex64(uint64_t v) { printf("%016llX\n", (unsigned long long)v); }

int main(int argc, char **argv) {
// sample plaintext and key
uint64_t P = 0x0123456789ABCDEFULL;
uint64_t K = 0x133457799BBCDFF1ULL;
// default segments (must sum to 48)
int s1=16,s2=16,s3=16,s1p=16,s2p=16,s3p=16;
if (argc == 7) {
s1 = atoi(argv[1]); s2 = atoi(argv[2]); s3 = atoi(argv[3]);
s1p = atoi(argv[4]); s2p = atoi(argv[5]); s3p = atoi(argv[6]);
}
uint64_t C = moddes_encrypt(P, K, s1,s2,s3, s1p,s2p,s3p);
printf("Plain : "); print_hex64(P);
printf("Key : "); print_hex64(K);
printf("Cipher: "); print_hex64(C);
return 0;
}
