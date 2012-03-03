#ifndef _ElgamalCryptosystem_h
#define _ElgamalCryptosystem_h
class ElgamalCryptosystem {
    private:
        bool init ();
        void findSGenerator (gmp_randstate_t rstate);
        mpz_t baseOrderMinus1;
        bool mallocError;

    public:
//        char * label;

        mpz_t prime;
        mpz_t base; // generates the subgroup of order baseOrder

        // (prime - 1) = baseOrder * y, y = r * s where s is smooth
        mpz_t baseOrder;
        CFactoredInteger *s;
        mpz_t sGenerator; // generates the subgroup of order s
        mpz_t y;
        mpz_t r;

        ElgamalDecKey dec;
        ElgamalEncKey enc;

        ElgamalCryptosystem ();

        // use for baseOrder != p-1
        ElgamalCryptosystem (unsigned int primeBits, unsigned int baseOrderBits, gmp_randstate_t rstate,
                             unsigned int smoothBits = 0, unsigned int smoothnessBitLimit = 16);

        // use for baseOrder = p-1
        ElgamalCryptosystem (unsigned int primeBits, gmp_randstate_t rstate);

        ~ElgamalCryptosystem ();

        bool hasMallocError () { return mallocError; }

        void print ();
        int read  (FILE *f);
        int write (FILE *f);

        void encrypt (ElgamalCipherText *ct, const mpz_t m, gmp_randstate_t rstate);
        void decrypt (mpz_t m, const ElgamalCipherText ct);

        size_t primeBits ();
        size_t baseBits ();
        size_t baseOrderBits ();
        size_t smoothBits ();
        size_t encBits ();
        size_t decBits ();
};
#endif
