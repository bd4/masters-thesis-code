/* 
 * Class for creating factored integers by factoring or random generation.
 *
 * Copyright (C) 2008 by Bryce Allen
 *
 * Distributed under the MIT license: see COPYING.MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "../include/types.h"

// WARNING: there is no proper copy operator

//*********************** PRIVATE METHODS *******************************

void CFactoredInteger::computePrimePowerValues () {

    for (unsigned int i = 0; i < nFactors; i++) {
        mpz_pow_ui (factors[i].value, factors[i].prime, factors[i].power);
    }

}

bool CFactoredInteger::init (size_t initialSize) {
    nFactors = 0;
    mallocError = false;
    
    mpz_init_set_ui (value, 1);
    factorsMpzInitCount = 0;

    if (initialSize > 0) {
        factorsSize = initialSize;
        factors = (PrimePower *) malloc (factorsSize * sizeof (*factors));

        if (factors == NULL) {
            // If init fails, all methods should return failure.
            // This can be checked by testing (factorsSize == 0 && mallocError).
            factorsSize = 0;
            mallocError = true;
            return false;
        }
    } else {
        factors = NULL;
        factorsSize = 0;
    }
    return true;
}

bool CFactoredInteger::ensureMallocInitTo (const size_t i, bool exactSize) {

    if (i >= factorsSize) {
        if (exactSize)
            factorsSize = i + 1;
        else
            factorsSize = i + FACTORS_SIZE_INCREMENT;
        // realloc does the right thing if factors is NULL
        PrimePower *newFactors = (PrimePower *) realloc (factors, factorsSize * sizeof (*factors));

        if (newFactors == NULL) {
            mallocError = true;
            return false;
        } else {
            factors = newFactors;
        }
    }

    for (size_t j = factorsMpzInitCount; j <= i; j++) {
        mpz_init (factors[j].prime);
        mpz_init (factors[j].value);
        factorsMpzInitCount++;
    }

    return true;

}

//*********************** PUBLIC METHODS & INSTRUCTORS ************************

CFactoredInteger::CFactoredInteger (size_t initialSize) {
    init (initialSize);
}

/*
CFactoredInteger::CFactoredInteger (unsigned int n, unsigned int factorBitLimit) {
    if (init ())
        factorValue (n, factorBitLimit);
}

CFactoredInteger::CFactoredInteger (mpz_t n, unsigned int factorBitLimit) {
    if (init ())
        factorValue (n, factorBitLimit);
}

CFactoredInteger::CFactoredInteger (unsigned int maxBits, gmp_randstate_t rstate,
                                    unsigned int factorBitLimit) { 
    if (init ())
        randomSmooth (maxBits, rstate, factorBitLimit);
}

CFactoredInteger::CFactoredInteger (mpz_t max, gmp_randstate_t rstate) { 
    if (init ())
        random (max, rstate);
}
*/

CFactoredInteger::~CFactoredInteger () {
    
    if (factors != NULL) {
        for (unsigned int i = 0; i < factorsMpzInitCount; i++) {
            mpz_clear (factors[i].prime);
            mpz_clear (factors[i].value);
        }
        free (factors);
    }
    mpz_clear (value);

}

bool CFactoredInteger::factorValue (unsigned int n, unsigned int factorBitLimit) { 
    if (factorsSize == 0 && mallocError)
        return false;
    mpz_t tmp;
    mpz_init_set_ui (tmp, n);
    bool isok = factorValue (tmp, factorBitLimit);
    mpz_clear (tmp);
    return isok;
}

// Factor using trial division. Not very efficient.
bool CFactoredInteger::factorValue (mpz_t n, unsigned int factorBitLimit) { 

    if (factorsSize == 0 && mallocError)
        return false;

    // if factorBitLimit is set, value may be set < n later
    mpz_set (value, n);

    if (mpz_probab_prime_p (n, 10)) {

        if (!ensureMallocInitTo (0)) { return false; }
        mpz_set (factors[0].prime, value);
        factors[0].power = 1;
        nFactors = 1;

    } else {

        mpz_t y, q, r, p;
        mpz_init_set (y, n);
        mpz_init (q); mpz_init (r);
        mpz_init_set_ui (p, 3);

        nFactors = 0;
        unsigned int power;

        // check for powers of two
        power = 0;
        while (mpz_tstbit (y, power) == 0) {
            power++;
        }

        int i = -1; // FIXME: might be to small
        if (power > 0) {
            mpz_fdiv_q_2exp (y, y, power);

            if (!ensureMallocInitTo (0)) { return false; }
            mpz_set_ui (factors[0].prime, 2);
            factors[0].power = power;

            i++;
            nFactors++;
        }

        do {
            mpz_fdiv_qr (q, r, y, p);
            if (mpz_cmp_ui (r, 0) == 0) {
                mpz_set (y, q);

                if (nFactors > 0 && mpz_cmp (factors[i].prime, p) == 0) {
                    factors[i].power++;
                } else {

                    i++;
                    nFactors++;
                    
                    // new factor found
                    if (!ensureMallocInitTo (i)) { return false; }

                    mpz_set (factors[i].prime, p);
                    factors[i].power = 1;

                }

            } else {
                mpz_add_ui (p, p, 2);
                if (factorBitLimit > 0 && mpz_sizeinbase (p, 2) > factorBitLimit) {
                    mpz_div (value, n, y);
                    break;
                }
            }
        } while (mpz_cmp_ui (y, 1) > 0);

        mpz_clear (y); mpz_clear (q); mpz_clear (r); mpz_clear (p);
    }

    computePrimePowerValues ();

    return true;

}

/*
 * Generate a random factored integer in {1 ... max} with a uniform
 * distribution using Victor Shoup's algorithm RFN (see p 298).
 */
bool CFactoredInteger::random (mpz_t max, gmp_randstate_t rstate) {

    if (factorsSize == 0 && mallocError)
        return false;
    mpz_t x, currentFactor, currentMax, n;
    mpz_init (x); mpz_init (currentFactor); mpz_init (currentMax); mpz_init (n);

    int i;
    bool done = false;
    while (!done) {
        mpz_set_ui (value, 1);
        mpz_set_ui (currentFactor, 0);

        // generate random sequence using RN and save the primes
        i = -1;
        mpz_set (currentMax, max);
        do {
            mpz_urandomm (n, rstate, currentMax); // between 0 and currentMax - 1
            mpz_add_ui (n, n, 1); // now between 1 and currentMax
            if (mpz_probab_prime_p (n, 10)) {

                mpz_mul (value, value, n);

                if (mpz_cmp (value, max) > 0) {
                    //printf ("value too big, breaking\n");
                    break;
                }

                if (mpz_cmp (n, currentFactor) != 0) {

                    i++;

                    if (!ensureMallocInitTo (i)) { return false; }

                    mpz_set (currentFactor, n);
                    //gmp_printf ("currentFactor = %Zd\n", currentFactor);
                    //gmp_printf ("seq[i] = %Zd\n", seq[i]);
                    //gmp_printf ("factorIndex = %d\n", factorIndex);

                    mpz_set (factors[i].prime, n);
                    factors[i].power = 1;

                } else {
                    factors[i].power++;
                    //gmp_printf ("Inc power for index %d, value %Zd\n", i, currentFactor);
                }

            }

            mpz_set (currentMax, n);
            
        } while (mpz_cmp_ui (currentMax, 1) > 0);

        // now test if we generated a suitible factored integer
        if (mpz_cmp_ui (value, 0) > 0 && mpz_cmp (value, max) <= 0) {
            mpz_urandomm (x, rstate, max);
            mpz_add_ui (x, x, 1);
            //gmp_printf ("    x = %Zd\n", x);
            if (mpz_cmp (value, x) >= 0) {
                    done = true;
            } else {
                //printf ("value smaller than x\n");
            }
        } else {
            //printf ("value too big or zero \n");
        }

    }
    
    nFactors = i+1;

    mpz_clear (x); mpz_clear (currentFactor); mpz_clear (currentMax); mpz_clear (n);
    
    computePrimePowerValues ();

    return true;

}

/*
 * Convenience method.
 */
bool CFactoredInteger::random (unsigned int maxBits, gmp_randstate_t rstate) {
    if (factorsSize == 0 && mallocError)
        return false;
    mpz_t max;
    mpz_init_set_ui (max, 1);

    mpz_mul_2exp (max, max, maxBits);
    mpz_sub_ui (max, max, 1);

    bool isOK = random (max, rstate);

    mpz_clear (max);

    return isOK;

}

/*
 * Generate a random SMOOTH factored integer in {1 ... 2^maxBits -1}, with factors
 * no larger than 2^factorBitLimit. This is a modification of the algorithm used above,
 * and is not uniform. In particular it favors smaller numbers.
 *
 * Basically we use the above algorithm to get a factored integer in {1, ... 2^factorBitLimit},
 * and then repeat (maxBits / factorBitLimit) times. As a consequence this only works
 * well if maxBits is a multiple of factorBitLimit.
 */
bool CFactoredInteger::randomSmooth (unsigned int maxBits, gmp_randstate_t rstate,
                                     unsigned int factorBitLimit) {
    if (factorsSize == 0 && mallocError)
        return false;

    mpz_t max, v, x, currentFactor, currentMax, n;
    mpz_init (max); mpz_init (v);
    mpz_init (x); mpz_init (currentFactor); mpz_init (currentMax); mpz_init (n);

    mpz_set_ui (value, 1);
    nFactors = 0;

    mpz_ui_pow_ui (max, 2, factorBitLimit);
    //mpz_sub_ui (max, max, 1);

    // we want this to round down, so integer division is fine
    unsigned int runCount = maxBits / factorBitLimit;

    int i;
    bool done, found;

    do {
        done = false;
        while (!done) {
            mpz_set_ui (v, 1);
            mpz_set_ui (currentFactor, 0);

            // generate random sequence using RN and save the primes
            i = nFactors - 1;
            mpz_set (currentMax, max);
            do {
                mpz_urandomm (n, rstate, currentMax); // between 0 and currentMax - 1
                mpz_add_ui (n, n, 1); // now between 1 and currentMax

                if (mpz_probab_prime_p (n, 10)) {

                    mpz_mul (v, v, n);

                    if (mpz_cmp (v, max) > 0) {
                        //printf ("value too big, breaking\n");
                        break;
                    }
                    if (mpz_cmp (n, currentFactor) != 0) {

                        i++;

                        if (!ensureMallocInitTo (i)) { return false; }

                        mpz_set (currentFactor, n);
                        //gmp_printf ("currentFactor = %Zd\n", currentFactor);
                        //gmp_printf ("seq[i] = %Zd\n", seq[i]);
                        //gmp_printf ("factorIndex = %d\n", factorIndex);

                        mpz_set (factors[i].prime, n);
                        factors[i].power = 1;

                    } else {
                        factors[i].power++;
                        //gmp_printf ("Inc power for index %d, value %Zd\n", i, currentFactor);
                    }

                }

                mpz_set (currentMax, n);
                
            } while (mpz_cmp_ui (currentMax, 1) > 0);

            // now test if we generated a suitible factored integer
            if (mpz_cmp_ui (v, 0) > 0 && mpz_cmp (v, max) <= 0) {
                mpz_urandomm (x, rstate, max);
                mpz_add_ui (x, x, 1);
                //gmp_printf ("    x = %Zd\n", x);
                if (mpz_cmp (v, x) >= 0) {
                    done = true;
                }
            }
        }

        mpz_mul (value, value, v);

        // fold in the new factors
        unsigned int nextFactorSlot = nFactors;
        for (unsigned int j = nFactors; (int)j < i + 1; j++) {
            // search for old factor which matches
            found = false;
            for (unsigned int k = 0; k < nFactors; k++) {
                if (mpz_cmp (factors[j].prime, factors[k].prime) == 0) {
                    factors[k].power++;
                    found = true;
                    break;
                }
            }
            // no matching old factor, put this in the new slot
            if (!found) {
                if (nextFactorSlot != j) { // unless it's already in the correct slot
                    mpz_set (factors[nextFactorSlot].prime, factors[j].prime);
                    factors[nextFactorSlot].power = factors[j].power;
                }
                nextFactorSlot++;
            }
        }

        nFactors = nextFactorSlot;

        runCount--;

    } while (runCount > 0);
    
    mpz_clear (max); mpz_clear (v);
    mpz_clear (x); mpz_clear (currentFactor); mpz_clear (currentMax); mpz_clear (n);
    
    computePrimePowerValues ();

    return true;
}

/*
 * Generate a random SMOOTH factored integer in {2^(bits-1) ... 2^bits -1}, with factors
 * no larger than 2^factorBitLimit. This is a modification of the algorithm used above,
 * and is not uniform.
 *
 * Basically we use CFactoredInteger::random to get a factored integer in {1, ... 2^factorBitLimit},
 * and then repeat until we obtain the required number of bits.
 */
bool CFactoredInteger::randomSmoothExactBits (unsigned int bits, gmp_randstate_t rstate,
                                              unsigned int factorBitLimit) {

    if (factorsSize == 0 && mallocError)
        return false;
    mpz_t max, v, x, currentFactor, currentMax, n;
    mpz_init (max); mpz_init (v);
    mpz_init (x); mpz_init (currentFactor); mpz_init (currentMax); mpz_init (n);

    mpz_set_ui (value, 1);
    nFactors = 0;

    mpz_ui_pow_ui (max, 2, factorBitLimit);
    //mpz_sub_ui (max, max, 1);

    int i;
    bool done, found;

    unsigned int bitsLeft = bits;

    do {
        done = false;
        while (!done) {
            mpz_set_ui (v, 1);
            mpz_set_ui (currentFactor, 0);

            // generate random sequence using RN and save the primes
            i = nFactors - 1;

            if (bitsLeft < factorBitLimit) {
                mpz_ui_pow_ui (max, 2, bitsLeft);
                //mpz_sub_ui (max, max, 1);
                factorBitLimit = bitsLeft;
            }
            mpz_set (currentMax, max);
            do {
                mpz_urandomm (n, rstate, currentMax); // between 0 and currentMax - 1
                mpz_add_ui (n, n, 1); // now between 1 and currentMax

                if (mpz_probab_prime_p (n, 10)) {

                    mpz_mul (v, v, n);

                    if (mpz_cmp (v, max) > 0) {
                        //printf ("value too big, breaking\n");
                        break;
                    }
                    if (mpz_cmp (n, currentFactor) != 0) {

                        i++;

                        if (!ensureMallocInitTo (i)) { return false; }

                        mpz_set (currentFactor, n);
                        //gmp_printf ("currentFactor = %Zd\n", currentFactor);
                        //gmp_printf ("seq[i] = %Zd\n", seq[i]);
                        //gmp_printf ("factorIndex = %d\n", factorIndex);

                        mpz_set (factors[i].prime, n);
                        factors[i].power = 1;

                    } else {
                        factors[i].power++;
                        //gmp_printf ("Inc power for index %d, value %Zd\n", i, currentFactor);
                    }

                }

                mpz_set (currentMax, n);
                
            } while (mpz_cmp_ui (currentMax, 1) > 0);

            // now test if we generated a suitible factored integer
            if (mpz_cmp_ui (v, 0) > 0 && mpz_cmp (v, max) <= 0) {
                mpz_urandomm (x, rstate, max);
                mpz_add_ui (x, x, 1);
                //gmp_printf ("    x = %Zd\n", x);
                if (mpz_cmp (v, x) >= 0) {
                    done = true;
                }
            }
        }

        mpz_mul (value, value, v);
        bitsLeft = bits - mpz_sizeinbase (value, 2);

        // fold in the new factors
        unsigned int nextFactorSlot = nFactors;
        for (unsigned int j = nFactors; (int)j < i + 1; j++) {
            // search for old factor which matches
            found = false;
            for (unsigned int k = 0; k < nFactors; k++) {
                if (mpz_cmp (factors[j].prime, factors[k].prime) == 0) {
                    factors[k].power += factors[j].power;
                    found = true;
                    break;
                }
            }
            // no matching old factor, put this in the new slot
            if (!found) {
                if (nextFactorSlot != j) { // unless it's already in the correct slot
                    mpz_set (factors[nextFactorSlot].prime, factors[j].prime);
                    factors[nextFactorSlot].power = factors[j].power;
                }
                nextFactorSlot++;
            }
        }

        nFactors = nextFactorSlot;

    } while (bitsLeft > 0);
    
    mpz_clear (max); mpz_clear (v);
    mpz_clear (x); mpz_clear (currentFactor); mpz_clear (currentMax); mpz_clear (n);
    
    computePrimePowerValues ();

    return true;

}

bool CFactoredInteger::compactify () {
    for (unsigned int i = nFactors; i < factorsMpzInitCount; i++) {
        mpz_clear (factors[i].prime);
        mpz_clear (factors[i].value);
    }
    factorsMpzInitCount = nFactors;

    
    if (factorsSize > nFactors) {
        factorsSize = nFactors;
        // realloc does the right thing if nFactors == 0 or factors == NULL
        PrimePower *newFactors = (PrimePower *) realloc (factors, factorsSize * sizeof (*factors));
        if (newFactors != NULL || nFactors == 0) {
            factors = newFactors;
        } else {
            mallocError = true;
            return false;
        }
    }

    return true;
}

// TODO: fread/fwrite return number of items read/written, not number of bytes
size_t CFactoredInteger::write (FILE *f) {

    if (factorsSize == 0 && mallocError)
        return 0;

    size_t bytes = 0;

    bytes += mpz_out_raw (f, value);
    
    bytes += fwrite (&nFactors, sizeof (nFactors), 1, f);

    for (unsigned int i = 0; i < nFactors; i++) {
        bytes += fwrite (&(factors[i].power), sizeof (factors[i].power), 1, f);
        bytes += mpz_out_raw (f, factors[i].prime);
    }

    return bytes;

}

size_t CFactoredInteger::read (FILE *f) {

    if (factorsSize == 0 && mallocError)
        return 0;

    size_t bytes = 0;

    bytes += mpz_inp_raw (value, f);

    bytes += fread (&nFactors, sizeof (nFactors), 1, f);

    if (!ensureMallocInitTo (nFactors-1, true)) { return 0; }

    for (unsigned int i = 0; i < nFactors; i++) {

        bytes += fread (&(factors[i].power), sizeof (factors[i].power), 1, f);
        bytes += mpz_inp_raw (factors[i].prime, f);

    }

    computePrimePowerValues ();

    return bytes;

}

void CFactoredInteger::print () {

    if (factorsSize == 0 && mallocError)
        return;
    gmp_printf ("value = %Zd\n", value);
    gmp_printf ("\tbits = %d\n", mpz_sizeinbase (value, 2));
    printf ("\tnFactors = %u\n", nFactors);

    for (unsigned int i = 0; i < nFactors; i++) {
        gmp_printf ("\t%Zd, %d\n", factors[i].prime, factors[i].power);
    }

}

