#ifndef _dlog_h
#define _dlog_h
/*
 * These defines are a dirty hack to make calling pohlig_hellman with pre-initialized
 * mpz_t variables easier. We will be calling the function so many times that it could have
 * significant performance advantages.
 */
#define DECLARE_PH_LOCALS mpz_t xi, q, qj, lj, ndivq, alphaBar, betaBar, betaStripped, alphaStrip, x, a, b, x1, a1, b1, alphaPower;
#define INIT_PH_LOCALS(prime) mpz_init (xi); mpz_init (q); mpz_init (qj); mpz_init (lj); mpz_init (ndivq); mpz_init2 (alphaBar, mpz_sizeinbase(prime,2)); mpz_init (betaBar); mpz_init2 (betaStripped, mpz_sizeinbase(prime,2)); mpz_init (alphaStrip); mpz_init (x); mpz_init (a); mpz_init (b); mpz_init (x1); mpz_init (a1); mpz_init (b1); mpz_init2 (alphaPower, mpz_sizeinbase(prime,2));
#define LIST_PH_LOCALS xi, q, qj, lj, ndivq, alphaBar, betaBar, betaStripped, alphaStrip, x, a, b, x1, a1, b1, alphaPower
#define CLEAR_PH_LOCALS mpz_clear (xi); mpz_clear (q); mpz_clear (qj); mpz_clear (lj); mpz_clear (ndivq); mpz_clear (alphaBar); mpz_clear (betaBar); mpz_clear (betaStripped); mpz_clear (alphaStrip); mpz_clear (x); mpz_clear (a); mpz_clear (b); mpz_clear (x1); mpz_clear (a1); mpz_clear (b1); mpz_clear (alphaPower);
#define PH_ARG_LIST mpz_t xi, mpz_t q, mpz_t qj, mpz_t lj, mpz_t ndivq, mpz_t alphaBar, mpz_t betaBar, mpz_t betaStripped, mpz_t alphaStrip, mpz_t x, mpz_t a, mpz_t b, mpz_t x1, mpz_t a1, mpz_t b1, mpz_t alphaPower
 
int pollard_rho (mpz_t result, mpz_t alpha, mpz_t p, mpz_t n, mpz_t beta, gmp_randstate_t rstate);
bool pollard_rho (mpz_t result, mpz_t alpha, mpz_t p, mpz_t n, mpz_t beta, gmp_randstate_t rstate, bool randomStart,
                  mpz_t x, mpz_t a, mpz_t b, mpz_t x1, mpz_t a1, mpz_t b1);

void pohlig_hellman (mpz_t result, mpz_t alpha, mpz_t p, CFactoredInteger *n, mpz_t beta, gmp_randstate_t rstate);
void pohlig_hellman (mpz_t result, mpz_t alpha, mpz_t p, CFactoredInteger *n, mpz_t beta, gmp_randstate_t rstate, PH_ARG_LIST);
#endif
