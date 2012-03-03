#ifndef _elgamal_h
#define _elgamal_h
typedef mpz_t ElgamalPlainText;
typedef struct {
    mpz_t gk;
    mpz_t myk;
} ElgamalCipherText;

typedef mpz_t ElgamalDecKey;
typedef mpz_t ElgamalEncKey;

void elgamalEncrypt (ElgamalCipherText *ct, const mpz_t m,
                     const mpz_t p, const mpz_t g, const mpz_t nMinus1,
                     const mpz_t y, gmp_randstate_t rstate);
void elgamalDecrypt (mpz_t m, const ElgamalCipherText ct,
                              const mpz_t p, const mpz_t x);

int elgamalWriteCipherText (FILE *f, const ElgamalCipherText ct);
int  elgamalReadCipherText (ElgamalCipherText ct, FILE *f);

#include "ElgamalCryptosystem.h"

/*
typedef struct {
    mpz_t prime;
    mpz_t base;
    mpz_t baseOrder;
    CFactoredInteger *s;
    mpz_t r;
    char * label;

    //PrimePower *factors;
    //int nFactors;
    ElgamalDecKey dec;
    ElgamalEncKey enc;
} ElgamalCryptosystem;


//void createCryptosystem (ElgamalCryptosystem *elgamal, gmp_randstate_t rstate);
void createCryptosystem (ElgamalCryptosystem *elgamal, int primeBits, int maxBaseOrderBits, gmp_randstate_t rstate,
                         int minSmoothBits = 0, int targetSmoothBits = 0, int smoothnessBitLimit = 16);
//void createCryptosystem (ElgamalCryptosystem *elgamal, int primeBits, int maxBaseOrderBits, gmp_randstate_t rstate);
void deleteCryptosystem (ElgamalCryptosystem *elgamal);
void printCryptosystem (ElgamalCryptosystem *elgamal);
int saveCryptosystem (ElgamalCryptosystem *c, FILE *f);
int loadCryptosystem (ElgamalCryptosystem *elgamal, FILE *f);
void elgamalEncrypt (ElgamalCipherText *ct, const mpz_t m,
                     const ElgamalCryptosystem elgamal,
                     gmp_randstate_t rstate);

void elgamalDecrypt (mpz_t m, const ElgamalCipherText ct,
                     const ElgamalCryptosystem elgamal);
*/
#endif
