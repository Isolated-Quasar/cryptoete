#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// fast (a^d mod n)
long long modpow(long long a, long long d, long long n) {
    long long res = 1;
    while (d) {
        if (d & 1) res = (res * a) % n;
        a = (a * a) % n;
        d >>= 1;
    }
    return res;
}


// one Miller–Rabin round
int check(long long a, long long d, int s, long long n) {
    long long x = modpow(a, d, n);
    if (x == 1 || x == n - 1) return 1;
    for (int r = 1; r < s; r++) {
        x = (x * x) % n;
        if (x == n - 1) return 1;
    }
    return 0;
}


// Miller–Rabin primality test
int is_probable_prime(long long n, int k) {
    if (n < 2 || n % 2 == 0) return (n == 2);
    long long d = n - 1;
    int s = 0;
    while (d % 2 == 0) { d /= 2; s++; }


    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 3);
        if (!check(a, d, s, n)) return 0;
    }
    return 1;
}


int main() {
    srand(time(NULL));
    long long tests[] = {101, 103, 997, 221, 341, 561};
    for (int i = 0; i < 6; i++) {
        long long n = tests[i];
        printf("%lld => %s\n", n,
            is_probable_prime(n, 10) ? "prime?" : "composite");
    }
    return 0;
}
