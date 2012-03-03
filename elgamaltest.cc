/* 
 * Program to test ElGamal encryption and decryption routines,
 * and the ElgamalCryptosystem class.
 *
 * Copyright (c) 2008 Bryce Allen
 *
 * You may use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and permit persons to whom the
 * Software is furnished to do so, under the terms of the MIT license,
 * provided in the LICENSE file. A copy is also available at
 * http://www.opensource.org/licenses/mit-license.php.
 */
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <unistd.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"

void usage () {
    printf ("elgamaltest [-c count] [-v] [-t] [-f file ] [-m msgBits] [-p primeBits] [-b maxBaseOrderBits] [-x]\n");
}

int main(int argc, char **argv) {

    mpz_t m, m2;

    gmp_randstate_t rstate;
    gmp_randinit_default (rstate);
    seedRandState (rstate);

    bool verbose = false;
    bool extendedTests = false;
    bool testFuncs = false;
    char *filePath = NULL;
    
    char *endptr;
    int opt;
    int count = 0;
    unsigned int primeBits = 1024;
    unsigned int maxBaseOrderBits = 512;
    unsigned int messageBits = 0;
    while ((opt = getopt (argc, argv, "p:b:c:f:m:tvx")) != -1) {
        switch (opt) {
        case 'v':
            verbose = true;
            break;
        case 'c':
            count = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case 'p':
            primeBits = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case 'b':
            maxBaseOrderBits = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case 'm':
            messageBits = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case 'f':
            filePath = optarg;
            break;
        case 't':
            testFuncs = true;
            break;
        case 'x':
            extendedTests = true;
            break;
        case ':':
        case '?':
            usage ();
            exit (1);
        }
    }

    if (maxBaseOrderBits > primeBits) { maxBaseOrderBits = primeBits; }

    ElgamalCipherText ct;
    mpz_init (ct.gk);
    mpz_init (ct.myk);
    mpz_init (m2);
    mpz_init (m);

    int returnValue = EXIT_SUCCESS;

    ElgamalCryptosystem *e;

    if (filePath != NULL) {
        e = new ElgamalCryptosystem ();
        FILE *f = fopen (filePath, "r");
        if (f == NULL) {
            perror ("Failed to open cryptosystem file");
            exit (EXIT_FAILURE);
        }
        e->read (f);
        fclose (f);
    } else {
        e = new ElgamalCryptosystem (primeBits, maxBaseOrderBits, rstate);
    }

    if (verbose) { e->print (); }

    mpz_powm (m, e->base, e->baseOrder, e->prime);
    if (mpz_cmp_ui (m, 1) != 0) {
        printf ("ERR: base does not have the correct order\n");
    } else if (verbose) {
        printf ("base looks to have the correct order\n");
    }
    if (mpz_cmp_ui (e->sGenerator, 1) > 0) {
        if (extendedTests) { // this takes way too long unless s is very small
            mpz_set (m, e->sGenerator);
            mpz_set_ui (m2, 2);
            do {
                mpz_mul (m, m, e->sGenerator);
                mpz_mod (m, m, e->prime);
                if (mpz_cmp_ui (m, 1) == 0) {
                    gmp_printf ("ERR: sGenerator has order %Zd, not s\n", m2);
                }
                mpz_add_ui (m2, m2, 1);
            } while (mpz_cmp (m2, e->s->value) < 0);
        }
        // this is incomplete, since it only tests for prime power orders
        mpz_set (m, e->sGenerator);
        for (unsigned int i=0; i < e->s->nFactors; i++) {
            for (unsigned int j=1; j <= e->s->factors[i].power; j++) {
                mpz_pow_ui (m2, e->s->factors[i].prime, j);
                mpz_powm (m, e->sGenerator, m2, e->prime);
                if (mpz_cmp_ui (m, 1) == 0 && (e->s->nFactors > 1 || j < e->s->factors[i].power)) {
                    printf ("ERR: sGenerator does not have the correct order\n");
                }
            }
        }
        mpz_powm (m, e->sGenerator, e->s->value, e->prime);
        if (mpz_cmp_ui (m, 1) != 0) {
            printf ("ERR: sGenerator does not have the correct order\n");
        } else if (verbose) {
            printf ("sGenerator looks to have the correct order\n");
        }
    }
    unsigned int passCount = 0;
    for (int i=0; i < count; i++) {
        if (messageBits > 0) {
            mpz_urandomb (m, rstate, messageBits);
        } else {
            mpz_urandomm (m, rstate, e->prime);
        }

        if (verbose)
            gmp_printf ("%d: %Zd\n", i, m);

        e->encrypt (&ct, m, rstate);
        e->decrypt (m2, ct);
        if (mpz_cmp (m, m2) == 0) {
            if (verbose) { printf ("\tPASS\n"); }
            passCount++;
        } else {
            if (verbose) { printf ("\tFAIL\n"); }
            returnValue = EXIT_FAILURE;
        }
    }
    delete (e);

    if (verbose) {
        printf ("PASS COUNT: %u / %d\n", passCount, count);
    }

    if (testFuncs) {
        // Test the elgamal(En|De)crypt functions
        mpz_t p, g, y, x, pMinus2;

        mpz_init_set_ui (p, 2579);
        mpz_init_set_ui (g, 2);
        mpz_init_set_ui (x, 765);
        mpz_init (y);
        mpz_init (pMinus2);

        mpz_sub_ui (pMinus2, p, 2);

        if (verbose) {
            gmp_printf ("p = %Zd\n", p);
            gmp_printf ("g = %Zd\n", g);
            gmp_printf ("x = %Zd\n", x);
        }

        mpz_powm (y, g, x, p);

        if (verbose)
            gmp_printf ("y = %Zd\n", y);

        mpz_init_set_ui (m, 1299);

        elgamalEncrypt (&ct, m, p, g, pMinus2, y, rstate);

        if (verbose) {
            gmp_printf ("g^k = %Zd\n", ct.gk);
            gmp_printf ("m*y^k = %Zd\n", ct.myk);
        }
        
        elgamalDecrypt (m2, ct, p, x);

        if (verbose) {
            gmp_printf ("m = %Zd\n", m);
            gmp_printf ("m2 = %Zd\n", m2);
        }

        if (mpz_cmp (m, m2) != 0) {
            returnValue = EXIT_FAILURE;
        }

        mpz_clear (p);
        mpz_clear (g);
        mpz_clear (y);
        mpz_clear (x);
        mpz_clear (pMinus2);
    }

    mpz_clear (ct.gk);
    mpz_clear (ct.myk);
    mpz_clear (m);
    mpz_clear (m2);
    gmp_randclear (rstate);

    return returnValue;
}
