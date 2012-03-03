/* 
 * Class for creating factored integers by factoring or random generation.
 * Factors are stored in a C-style dynamic array of PrimePower structures.
 * Encapsulation is intentionally weak - the PrimePower array is exposed
 * as a public member.
 *
 * Copyright (C) 2008 by Bryce Allen
 *
 * Distributed under the MIT license: see COPYING.MIT
 */
#ifndef _CFactoredInteger_h
#define _CFactoredInteger_h

#define INITIAL_FACTORS_SIZE 5
#define FACTORS_SIZE_INCREMENT 5

class CFactoredInteger {
    protected:
        bool mallocError;

        size_t factorsSize;
        size_t factorsMpzInitCount;
        //unsigned int factorBitLimit;
        //size_t extraFactorSpace;
        void computePrimePowerValues ();

        // return false if memory allocation fails
        bool init (size_t initialSize = INITIAL_FACTORS_SIZE);

        // up to and including index
        bool ensureMallocInitTo (size_t index, bool exactSize = false);

    public:
        unsigned int nFactors;
        PrimePower *factors;
        mpz_t value;

        CFactoredInteger (size_t initialSize = INITIAL_FACTORS_SIZE);
        /*
        CFactoredInteger (mpz_t n, unsigned int factorBitLimit = 0);
        CFactoredInteger (unsigned int n, unsigned int factorBitLimit = 0);
        CFactoredInteger (mpz_t max, gmp_randstate_t rstate);
        CFactoredInteger (unsigned int maxBits, gmp_randstate_t rstate,
                          unsigned int factorBitLimit = 0);
        */

        virtual ~CFactoredInteger ();

        bool hasMallocError () { return mallocError; }

        bool factorValue (unsigned int n, unsigned int factorBitLimit = 0);
        virtual bool factorValue (mpz_t n, unsigned int factorBitLimit = 0);

        bool random (mpz_t max, gmp_randstate_t rstate);
        bool random (unsigned int maxBits, gmp_randstate_t rstate);
        bool randomSmooth (unsigned int maxBits, gmp_randstate_t rstate,
                           unsigned int factorBitLimit);
        bool randomSmoothExactBits (unsigned int bits, gmp_randstate_t rstate,
                                    unsigned int factorBitLimit);

        //void multiply (mpz_t prime, unsigned int power);
        bool compactify ();

        size_t write (FILE *f);
        size_t read (FILE *f);
        void print ();

//        mpz_t getValue ();

 //       PrimePower &operator[] (const unsigned int index);
};
#endif
