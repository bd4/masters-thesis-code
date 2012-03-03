/*
 * =====================================================================================
 *
 *       Filename:  randomhelpers.cc
 *
 *    Description:  Helper functions for generating random numbers.
 *
 *        Version:  1.0
 *        Created:  07/09/2008 04:24:51 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Bryce Allen
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <gmp.h>

#include "../include/types.h"
#include "../include/randomhelpers.h"

#define MAX_SEQ_LEN 1000

bool randomULong (unsigned long *out) {
    FILE *f = fopen ("/dev/urandom", "r");
    if (f == NULL) {
        perror ("Failed to open urandom");
        return false;
    }

    if (fread (out, sizeof(*out), 1, f) != 1) {
        if (ferror (f)) {
            fputs ("urandom read error", stderr);
        } else {
            fputs ("urandom eof", stderr);
        }
        fclose (f);
        return false;
    }
    fclose (f);

    return true;
}

bool seedRandState (gmp_randstate_t state) {
    unsigned long seed;
    if (randomULong (&seed)) {
        gmp_randseed_ui (state, seed);
        return true;
    }
    return false;
}

/*
 * Generate a non-increasing random sequence of mpz_t's using
 * Victor Shoup's algorithm RS (see p 295). Modified to store only
 * the primes, since this is all we need for the RFN algorithm below.
 */
/*
int randomNonIncreasingPrimeSequence (mpz_t *seq, const mpz_t max, gmp_randstate_t rstate) {
    mpz_t currentMax, n;

    mpz_init(n);
    mpz_init_set (currentMax, max);
    int i = 0;

    do {
        mpz_urandomm (n, rstate, currentMax); // between 0 and currentMax - 1
        mpz_add_ui (n, n, 1); // shift up
        if (mpz_probab_prime_p (n, 10)) {
            mpz_init_set (seq[i], n);
            //seq->push_back (n);
            i++;
        }
        mpz_set (currentMax, n);
        // n should leave the stack, but it's now stored in the array, assuming
        // mpz_t really behaves like a pointer (or struct of pointers) here
    } while (mpz_cmp_ui (currentMax, 1) > 0 && i < MAX_SEQ_LEN);

    mpz_clear (currentMax);
    mpz_clear (n);

    return i;
}
*/
/*
 * Generate a random factored integer in {0 ... 2^maxBits -1} with a uniform
 * distribution using Victor Shoup's algorithm RFN (see p 298).
 */
/*
FactoredInteger* randomFactoredInteger (mpz_t max, gmp_randstate_t rstate) {
    mpz_t x, y, currentFactor;
    mpz_t currentMax, n;

    mpz_init (x);
    mpz_init (y);
    mpz_init (currentFactor);
    mpz_init (currentMax);
    mpz_init (n);

    PrimePower *factors = (PrimePower *) malloc (sizeof(PrimePower) * MAX_SEQ_LEN);
    //mpz_t *seq = (mpz_t *) malloc (sizeof(mpz_t) * MAX_SEQ_LEN);
    
    int i;
    bool done = false;
    while (!done) {
        mpz_set_ui (y, 1);
        mpz_set_ui (currentFactor, 0);

        // generate random sequence using RN and save the primes
        i = -1;
        mpz_set (currentMax, max);
        mpz_set_ui (currentFactor, 0);
        do {
            mpz_urandomm (n, rstate, currentMax); // between 0 and currentMax - 1
            mpz_add_ui (n, n, 1); // now between 1 and currentMax
            if (mpz_probab_prime_p (n, 10)) {

                mpz_mul (y, y, n);

                if (mpz_cmp (y, max) > 0) {
                    //printf ("y too big, breaking\n");
                    break;
                }

                if (mpz_cmp (n, currentFactor) != 0) {

                    i++;
                    mpz_set (currentFactor, n);
                    //gmp_printf ("currentFactor = %Zd\n", currentFactor);
                    //gmp_printf ("seq[i] = %Zd\n", seq[i]);
                    //gmp_printf ("factorIndex = %d\n", factorIndex);
                    mpz_init_set (factors[i].prime, n);
                    factors[i].power = 1;

                } else {
                    factors[i].power++;
                    //gmp_printf ("Inc power for index %d, value %Zd\n", i, currentFactor);
                }

            }

            mpz_set (currentMax, n);
            
        } while (mpz_cmp_ui (currentMax, 1) > 0 && i < MAX_SEQ_LEN);

        // now test if we generated a suitible factored integer
        if (mpz_cmp_ui (y, 0) > 0 && mpz_cmp (y, max) <= 0) {
            mpz_urandomm (x, rstate, max);
            mpz_add_ui (x, x, 1);
            //gmp_printf ("    x = %Zd\n", x);
            if (mpz_cmp (y, x) >= 0) {
                    done = true;
            } else {
                //printf ("y smaller than x\n");
            }
        } else {
            //printf ("y too big or zero \n");
        }

    }
    
    FactoredInteger *factorList = (FactoredInteger *) malloc (sizeof (FactoredInteger));
    factorList->factors = factors;
    factorList->nFactors = i+1;
    mpz_init_set (factorList->value, y);

    mpz_clear (x);
    mpz_clear (y);
    mpz_clear (currentFactor);
    mpz_clear (currentMax);
    mpz_clear (n);
    
    return factorList;

}


FactoredInteger* randomFactoredInteger (int maxBits, gmp_randstate_t rstate) {
 
    mpz_t max;

    mpz_init (max);

    mpz_ui_pow_ui (max, 2, maxBits);
    mpz_sub_ui (max, max, 1);

    FactoredInteger* list = randomFactoredInteger (max, rstate);

    mpz_clear (max);

    return list;

}
*/
