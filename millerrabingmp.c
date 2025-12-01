#include <stdio.h>
#include <gmp.h>
#include <time.h>


// Miller–Rabin test using GMP
int is_probable_prime(mpz_t n, int k, gmp_randstate_t st) {
    if (mpz_cmp_ui(n, 2) < 0) return 0;
    if (mpz_even_p(n)) return mpz_cmp_ui(n, 2) == 0;


    mpz_t d, a, x, n1;
    mpz_inits(d, a, x, n1, NULL);
    mpz_sub_ui(n1, n, 1);  // n-1
    mpz_set(d, n1);


    int s = 0;
    while (mpz_even_p(d)) { mpz_divexact_ui(d, d, 2); s++; }


    for (int i = 0; i < k; i++) {
        mpz_urandomm(a, st, n1); // a in [0,n-2]
        mpz_add_ui(a, a, 1);     // a in [1,n-1]
        if (mpz_cmp_ui(a, 2) < 0) mpz_set_ui(a, 2);


        mpz_powm(x, a, d, n);
        if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, n1) == 0) continue;


        int pass = 0;
        for (int r = 1; r < s; r++) {
            mpz_powm_ui(x, x, 2, n);
            if (mpz_cmp(x, n1) == 0) { pass = 1; break; }
        }
        if (!pass) { mpz_clears(d,a,x,n1,NULL); return 0; }
    }
    mpz_clears(d,a,x,n1,NULL);
    return 1;
}
int main() {
    gmp_randstate_t st;
    gmp_randinit_default(st);
    gmp_randseed_ui(st, time(NULL));


    mpz_t n;
    mpz_init(n);


    // random 1024-bit odd number
    mpz_urandomb(n, st, 1024);
    mpz_setbit(n, 1023); // force 1024 bits
    mpz_setbit(n, 0);    // force odd


    gmp_printf("Random 1024-bit number: %Zx\n", n);
    printf("%s\n", is_probable_prime(n, 20, st) ? "Probably prime" : "Composite");


    mpz_clear(n);
    gmp_randclear(st);
    return 0;
}
