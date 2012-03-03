/* 
 * Class to represent an ElGamal cryptosystem.
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

//*********************** PRIVATE METHODS *******************************

bool ElgamalCryptosystem::init () {
    mpz_init (prime);
    mpz_init (base);
    mpz_init (enc);
    mpz_init (dec);
    mpz_init (baseOrder);
    mpz_init (baseOrderMinus1);
    mpz_init (y);
    mpz_init (r);
    mpz_init (sGenerator);

    s = new CFactoredInteger();

    mallocError = s->hasMallocError ();

    return !mallocError;
}

void ElgamalCryptosystem::findSGenerator (gmp_randstate_t rstate) {

    mpz_t z, gamma, delta, pow, pMinus1;
    mpz_init (z); mpz_init (gamma); mpz_init (delta); mpz_init (pow); mpz_init (pMinus1);

    mpz_sub_ui (pMinus1, prime, 1);

    // compute z = r * baseOrder, so that p-1 = z * s
    mpz_mul (z, r, baseOrder);

    mpz_set_ui (sGenerator, 1);
    for (unsigned int i = 0; i < s->nFactors; i++) {
        //gmp_printf ("[%d] (%Zd)^(%lu):\n", i, s->factors[i].prime, s->factors[i].power);
        mpz_div (pow, s->value, s->factors[i].prime);
        do {
            mpz_urandomm (gamma, rstate, pMinus1);
            mpz_add_ui (gamma, gamma, 1); // 1 <= gamma <= p-1
            mpz_powm (gamma, gamma, z, prime); // random element in subgroup
                                               // of order s

            mpz_powm (delta, gamma, pow, prime);
        } while (mpz_cmp_ui (delta, 1) == 0);

        //gmp_printf ("\tgamma = %Zd\n", gamma);

        // delta and pow are no longer used, so we reuse them as temporaries
        // delta = q^c = value of current prime power
        mpz_pow_ui (delta, s->factors[i].prime, s->factors[i].power);

        // pow = n/q^c = product of other prime powers
        mpz_div (pow, s->value, delta);

        // reuse delta again
        // delta = gamma^(n/q^c), where gamma^(n/q) != 1
        mpz_powm (delta, gamma, pow, prime);

        mpz_mul (sGenerator, sGenerator, delta);
        mpz_mod (sGenerator, sGenerator, prime);
    }

    mpz_clear (z); mpz_clear (gamma); mpz_clear (delta); mpz_clear (pow); mpz_clear (pMinus1);

}


//*********************** PUBLIC METHODS & INSTRUCTORS *******************************

ElgamalCryptosystem::ElgamalCryptosystem () {
    init ();
}

// Generate a cryptosystem with p = n+1
ElgamalCryptosystem::ElgamalCryptosystem (unsigned int primeBits, gmp_randstate_t rstate) {
    if (!init ())
        return;

    //mpz_t highBit;
    //mpz_init_set_ui (highBit, 1);
    //mpz_mul_2exp (highBit, highBit, baseOrderBits - 1);

    //mpz_ui_pow_ui (tmp, 2, baseOrderBits);
    //mpz_sub_ui (tmp, 1);
    
    //unsigned int bitDebug;

    // pick factored n so that p = n+1 is prime
    CFactoredInteger n;
    do {
        do {
            n.random (primeBits, rstate);
            if (n.hasMallocError ()) {
                mallocError = true;
                return; // no mpz_init calls above which need clearing
            }
        } while (mpz_sizeinbase (n.value, 2) != primeBits);

        mpz_set (baseOrder, n.value);
        mpz_sub_ui (baseOrderMinus1, baseOrder, 1);

        mpz_add_ui (prime, n.value, 1);
    } while (!mpz_probab_prime_p (prime, 4) || mpz_sizeinbase (prime, 2) != primeBits);
    
#ifdef DEBUG
    printf ("generated random prime with specified length\n");
    gmp_printf ("prime = %Zd\n", prime);
    gmp_printf ("prime bits = %u\n", mpz_sizeinbase (prime, 2));
#endif

    // find a primitive element, store in base
    mpz_t gamma, delta, e;
    mpz_init (gamma); mpz_init (delta); mpz_init (e);
    mpz_set_ui (base, 1);
    for (unsigned int i = 0; i < n.nFactors; i++) {
        //gmp_printf ("[%d] (%Zd)^(%lu):\n", i, n.factors[i].prime, n.factors[i].power);
        mpz_div (e, n.value, n.factors[i].prime);
        do {
            mpz_urandomm (gamma, rstate, n.value); // 0 <= gamma <= n-1 = p-2
            mpz_add_ui (gamma, gamma, 1); // 1 <= gamma <= n = p-1
            mpz_powm (delta, gamma, e, prime);
        } while (mpz_cmp_ui (delta, 1) == 0);

        //gmp_printf ("\tgamma = %Zd\n", gamma);

        // e = n/q^c = product of other prime powers
        mpz_div (e, n.value, n.factors[i].value);

        // delta = gamma^(n/q^c), where gamma^(n/q) != 1
        mpz_powm (delta, gamma, e, prime);

        mpz_mul (base, base, delta);
        mpz_mod (base, base, prime);
    }
    mpz_clear (gamma); mpz_clear (delta); mpz_clear (e);

#ifdef DEBUG
    printf ("found base of desired order (p-1)\n");
    gmp_printf (" base = %Zd\n", base);
#endif

    mpz_urandomm (dec, rstate, baseOrderMinus1); // 0 - baseOrder-2
    mpz_add_ui (dec, dec, 1); // 1 - baseOrder-1

    mpz_powm (enc, base, dec, prime);

    mpz_set_ui (sGenerator, 0);
    mpz_set_ui (y, 1);
    mpz_set_ui (r, 1);
    mpz_set_ui (s->value, 1);

    //mpz_clear (tmp);
}

ElgamalCryptosystem::ElgamalCryptosystem (unsigned int primeBits,
                             unsigned int baseOrderBits, gmp_randstate_t rstate,
                             unsigned int smoothBits, unsigned int smoothnessBitLimit) {
    if (!init ())
        return;

    //mpz_t highBit;
    //mpz_init_set_ui (highBit, 1);
    //mpz_mul_2exp (highBit, highBit, baseOrderBits - 1);

    //mpz_ui_pow_ui (tmp, 2, baseOrderBits);
    //mpz_sub_ui (tmp, 1);
    
    //unsigned int bitDebug;

    // pick prime baseOrder, smooth s, and y such that p = baseOrder * y + 1 is prime and s is a factor of y.
    do {
        do {
            mpz_urandomb (baseOrder, rstate, baseOrderBits - 1);
            mpz_setbit (baseOrder, baseOrderBits - 1);
            mpz_nextprime (baseOrder, baseOrder);
        } while (mpz_sizeinbase (baseOrder, 2) > baseOrderBits);

        mpz_sub_ui (baseOrderMinus1, baseOrder, 1);

        // pick random even integer so the product baseOrder * (s * y) is at most primeBits bits,
        // and s is smooth
        unsigned int rbits;
        if (smoothBits == 0) {
            // We want y to have primeBits - baseOrderBits and be even,
            // so we generate 2 less random bits than needed and set
            // the first and last bits accordingly.
            rbits = primeBits - baseOrderBits - 2;
            mpz_urandomb (y, rstate, rbits);
            mpz_setbit (y, rbits); // set high order bit
            mpz_mul_2exp (y, y, 1); // shift left by one, making y even
            mpz_set_ui (s->value, 1);
            mpz_set (r, y);
        } else {
            s->randomSmoothExactBits (smoothBits, rstate, smoothnessBitLimit);
            //printf ("s bits = %zu\n", mpz_sizeinbase (s->value, 2));
            if (s->hasMallocError ()) {
                mallocError = true;
                return; // not mpz_init calls above which need clearing
            }

            rbits = primeBits - baseOrderBits - mpz_sizeinbase (s->value, 2) - 2;
            mpz_urandomb (r, rstate, rbits);
            mpz_setbit (r, rbits); // set high order bit
            mpz_mul_2exp (r, r, 1); // shift left by one, making y even
            mpz_mul (y, r, s->value);
            //bitDebug = mpz_sizeinbase (y, 2);
            //printf ("y bits = %u\n", bitDebug);
        }

        //gmp_printf ("baseOrder = %Zd\n", baseOrder);
        //gmp_printf ("        y = %Zd\n", y);

        mpz_mul (prime, y, baseOrder);
        //mpz_mul (prime, prime, s->value);
        mpz_add_ui (prime, prime, 1);
        //gmp_printf ("   prime? = %Zd\n", prime);
        //bitDebug = mpz_sizeinbase (prime, 2);
        //printf ("prime bits = %u\n", bitDebug);

        //mpz_urandomb (prime, rstate, primeBits);
        //mpz_nextprime (prime, prime);
    } while (!mpz_probab_prime_p (prime, 10) || mpz_sizeinbase (prime, 2) < primeBits);
    
#ifdef DEBUG
    printf ("generated random prime with specified base order length\n");
    gmp_printf ("prime = %Zd\n", prime);
    gmp_printf ("prime bits = %u\n", mpz_sizeinbase (prime, 2));
#endif

    // Find an element of order baseOrder.
    // Note: in a real implementation having a small base would improve efficiency. However
    // we only need to encrypt/decrypt a few messages, and the attacks never use the base,
    // so it doesn't matter for our purposes.
    do {
        mpz_urandomm (base, rstate, prime);
        mpz_powm (base, base, y, prime);
    } while (mpz_cmp_ui (base, 1) == 0);

#ifdef DEBUG
    printf ("found base of desired order\n");
    gmp_printf (" base = %Zd\n", base);
#endif

    // TODO: make sure this is a good value. (also research what that means)
    mpz_urandomm (dec, rstate, baseOrderMinus1); // 0 - baseOrder-2
    mpz_add_ui (dec, dec, 1); // 1 - baseOrder-1

    mpz_powm (enc, base, dec, prime);

    if (smoothBits == 0) {
        mpz_set_ui (sGenerator, 0);
    } else {
        findSGenerator (rstate);
    }

    //mpz_clear (tmp);

}

void ElgamalCryptosystem::encrypt (ElgamalCipherText *ct, const mpz_t m,
                                   gmp_randstate_t rstate) {
    elgamalEncrypt (ct, m, prime, base, baseOrderMinus1, enc, rstate);
}

void ElgamalCryptosystem::decrypt (mpz_t m, const ElgamalCipherText ct) {
    elgamalDecrypt (m, ct, prime, dec);
}

void ElgamalCryptosystem::print () {
    gmp_printf ("prime = %Zd\n", prime);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (prime, 2));
    gmp_printf ("base = %Zd\n", base);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (base, 2));
    gmp_printf ("baseOrder = %Zd\n", baseOrder);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (baseOrder, 2));
    gmp_printf ("enc = %Zd\n", enc);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (enc, 2));
    gmp_printf ("dec = %Zd\n", dec);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (dec, 2));
    gmp_printf ("y = %Zd\n", y);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (y, 2));
    gmp_printf ("r = %Zd\n", r);
    gmp_printf (" (%lu bits)\n", mpz_sizeinbase (r, 2));
    if (s->nFactors > 0) {
        printf ("s: ");
        s->print ();
    } else {
        printf ("s = 1\n");
    }
    if (mpz_cmp_ui (sGenerator, 1) > 0) {
        gmp_printf ("sGenerator = %Zd\n", sGenerator);
        gmp_printf (" (%lu bits)\n", mpz_sizeinbase (sGenerator, 2));
    }
}

ElgamalCryptosystem::~ElgamalCryptosystem () {

    mpz_clear (prime);
    mpz_clear (base);
    mpz_clear (enc);
    mpz_clear (dec);
    mpz_clear (baseOrder);
    mpz_clear (baseOrderMinus1);
    mpz_clear (y);
    mpz_clear (r);
    mpz_clear (sGenerator);

    if (s != NULL) {
        delete s;
    }

    // caller is responsible for reclaiming label if necessary
    //if (label != NULL) {
    //    free (label);
    //}
}

// TODO: check every call for zero return value.
int ElgamalCryptosystem::write (FILE *f) {
    int bytes = 0;
    bytes += mpz_out_raw (f, prime); 
    bytes += mpz_out_raw (f, base); 
    bytes += mpz_out_raw (f, baseOrder); 
    bytes += mpz_out_raw (f, r);
    bytes += mpz_out_raw (f, enc); 
    bytes += mpz_out_raw (f, dec);
    bytes += s->write (f);
    bytes += mpz_out_raw (f, sGenerator);

/*
    int labelLen = 0;
    if (label != NULL) {
        labelLen = strlen (label);
    }
    bytes += fwrite (&labelLen, sizeof (labelLen), 1, f);
    if (label != NULL) {
        fwrite (label, sizeof (*label), labelLen, f);
    }
    */

    return bytes;
}

int ElgamalCryptosystem::read (FILE *f) {

    int bytes = 0;
    bytes += mpz_inp_raw (prime, f); 
    bytes += mpz_inp_raw (base, f); 
    bytes += mpz_inp_raw (baseOrder, f); 
    bytes += mpz_inp_raw (r, f);
    bytes += mpz_inp_raw (enc, f); 
    bytes += mpz_inp_raw (dec, f); 
    bytes += s->read (f);

    bytes += mpz_inp_raw (sGenerator, f);

    mpz_mul (y, s->value, r);
    mpz_sub_ui (baseOrderMinus1, baseOrder, 1);

/*
    int labelLen;
    bytes += fread (&labelLen, sizeof (labelLen), 1, f);

    if (labelLen > 0) {
        label = (char *) malloc ((labelLen + 1) * sizeof (*label));

        bytes += fread (label, sizeof (*label), labelLen, f);

        label[labelLen] = '\0';
    } else {
        label = NULL;
    }
*/
    return bytes;
}
size_t ElgamalCryptosystem::primeBits () { return mpz_sizeinbase (prime, 2); }
size_t ElgamalCryptosystem::baseBits () { return mpz_sizeinbase (base, 2); }
size_t ElgamalCryptosystem::baseOrderBits () { return mpz_sizeinbase (baseOrder, 2); }
size_t ElgamalCryptosystem::smoothBits () {
    if (s->nFactors > 0)
        return mpz_sizeinbase (s->value, 2);
    return 0;
}
size_t ElgamalCryptosystem::encBits () { return mpz_sizeinbase (enc, 2); }
size_t ElgamalCryptosystem::decBits () { return mpz_sizeinbase (dec, 2); }

