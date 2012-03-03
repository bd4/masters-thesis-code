/* 
 * Functional interface for ElGamal encryption and decryption. Used
 * by the class.
 *
 * Copyright (C) 2008 by Bryce Allen
 *
 * Distributed under the MIT license: see COPYING.MIT
 */
#include <stdio.h>
#include <gmp.h>
#include "../include/types.h"
#include "../include/randomhelpers.h"
#include "../include/elgamal.h"

//#define DEBUG 1

void elgamalEncrypt (ElgamalCipherText *ct, const mpz_t m,
                     const mpz_t p, const mpz_t g, const mpz_t nMinus1,
                     const mpz_t y, gmp_randstate_t rstate) {
    mpz_t k;
    mpz_init (k);
    mpz_urandomm (k, rstate, nMinus1); // 0 - n-2
    mpz_add_ui (k, k, 1); // 1 - n-1

    //gmp_printf ("rand k = %Zd\n", k);

    mpz_powm (ct->gk, g, k, p);

    mpz_powm (ct->myk, y, k, p);
    mpz_mul (ct->myk, ct->myk, m);
    mpz_mod (ct->myk, ct->myk, p);

    mpz_clear (k);
}

void elgamalDecrypt (mpz_t m, const ElgamalCipherText ct,
                              const mpz_t p, const mpz_t x) {
    mpz_invert (m, ct.gk, p);
    mpz_powm (m, m, x, p);
    mpz_mul (m, m, ct.myk);
    mpz_mod (m, m, p);
}

int elgamalWriteCipherText (FILE *f, const ElgamalCipherText ct) {
    int bytes = 0;
    bytes += mpz_out_raw (f, ct.gk); 
    bytes += mpz_out_raw (f, ct.myk); 
    return bytes;
}

int  elgamalReadCipherText (ElgamalCipherText ct, FILE *f) {
    int bytes = 0;
    bytes += mpz_inp_raw (ct.gk, f); 
    bytes += mpz_inp_raw (ct.myk, f); 
    return bytes;
}
