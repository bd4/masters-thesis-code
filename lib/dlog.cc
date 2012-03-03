/*
 * =====================================================================================
 *
 *       Filename:  dlog.cc
 *
 *    Description:  Algorithms for discrete logs.
 *
 *        Version:  1.0
 *        Created:  09/07/2008 11:53:26 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>

#include "../include/types.h"
#include "../include/dlog.h"

/*
 * Pollard's Rho algorithm for discrete logs.
 * See Handbook of Applied Cryptography, algorithm 3.60 page 106.
 *
 * Computes log base alpha of beta, where all operations are done
 * in Z_p and n is the order of alpha in Z_p.
 *
 * n is assumed to be prime, since this function will be used as
 * a subroutine called by the Pohlig-Hellman algorithm.
 */

/*
 * This is the random-looking function used for the Pollard Rho algorithm
 */
void inline f(mpz_t x, mpz_t a, mpz_t b, mpz_t alpha, mpz_t p, mpz_t n, mpz_t beta) {
    unsigned long int mod = mpz_fdiv_ui (x, 3); 
    if (mod == 1) {

        // x = beta * x (mod p)
        mpz_mul (x, x, beta);
        mpz_mod (x, x, p);
        
        // a = a
        //mpz_set (a, a);

        // b = b + 1 (mod n)
        mpz_add_ui (b, b, 1);
        mpz_mod (b, b, n);

    } else if (mod == 0) {

        // x = x^2 (mod p)
        mpz_mul (x, x, x);
        mpz_mod (x, x, p);
        
        // a = 2 * a (mod n)
        mpz_mul_2exp (a, a, 1);
        mpz_mod (a, a, n);

        // b = 2 * b (mod n)
        mpz_mul_2exp (b, b, 1);
        mpz_mod (b, b, n);

    } else {
 
        // x = alpha * x (mod p)
        mpz_mul (x, x, alpha);
        mpz_mod (x, x, p);
        
        // a = a + 1 (mod n)
        mpz_add_ui (a, a, 1);
        mpz_mod (a, a, n);

        // b = b
        //mpz_set (b, b);
       
    }
}


/*
 * boolean return value indicates wether or not the algorithm was successful.
 */
bool pollard_rho (mpz_t result, mpz_t alpha, mpz_t p, mpz_t n, mpz_t beta, gmp_randstate_t rstate, bool randomStart,
                  mpz_t x, mpz_t a, mpz_t b, mpz_t x1, mpz_t a1, mpz_t b1) {

    if (randomStart) {

        //mpz_init (x); mpz_init (a); mpz_init (b);
        //mpz_init (x1); mpz_init (a1); mpz_init (b1);
        
        mpz_set (x1, n); // use x1 as a temp variable for n-1
        mpz_sub_ui (x1, x1, 1);
        mpz_urandomm (a, rstate, x1);
        mpz_urandomm (b, rstate, x1);
        mpz_add_ui (a, a, 1);
        mpz_add_ui (b, b, 1);

        mpz_powm (x, alpha, a, p);
        mpz_powm (x1, beta, b, p);
        mpz_mul (x, x, x1);
        mpz_mod (x, x, p);

        mpz_set (x1, x);
        mpz_set (a1, a);
        mpz_set (b1, b);

    } else {

        mpz_set_ui (x, 1);
        mpz_set_ui (a, 0);
        mpz_set_ui (b, 0);

        mpz_set_ui (x1, 1);
        mpz_set_ui (a1, 0);
        mpz_set_ui (b1, 0);

    }

    //f (x1, a1, b1, x, a, b, alpha, p, n, beta);

    do {

        f (x, a, b, alpha, p, n, beta);
        f (x1, a1, b1, alpha, p, n, beta);
        f (x1, a1, b1, alpha, p, n, beta);

        //gmp_printf (" i: %Zd, %Zd, %Zd\n", x, a, b);
        //gmp_printf ("2i: %Zd, %Zd, %Zd\n\n", x1, a1, b1);

    } while (mpz_cmp (x, x1) != 0);

    bool success = false;

    mpz_sub (b, b, b1);
    mpz_mod (b, b, n);
    if (mpz_cmp_ui (b, 0) != 0) {
        success = true;

        mpz_sub (a, a1, a);
        mpz_mod (a, a, n);

        mpz_invert (b, b, n);
        mpz_mul (result, a, b);
        mpz_mod (result, result, n);
    }

    return success;
}

inline int pollard_rho (mpz_t result, mpz_t alpha, mpz_t p, mpz_t n,
                        mpz_t beta, gmp_randstate_t rstate,
                        mpz_t x, mpz_t a, mpz_t b, mpz_t x1, mpz_t a1, mpz_t b1, mpz_t alphaPower) {

    //gmp_printf ("pollard_rho (%Zd, %Zd, %Zd, %Zd)\n", alpha,p,n,beta);
    int runCount = 1;
    /*
    if (mpz_cmp_ui (n, 2) == 0) {
        if (mpz_cmp_ui (beta, 1) == 0) {
            mpz_set_ui (result, 0);
        } else { // assume beta is in <alpha>
            mpz_set_ui (result, 1);
        }
    }*/
    
    if (mpz_cmp_ui (n, 11) <= 0) {

        mpz_set (alphaPower, alpha);
        unsigned long int power = 1;
        while (mpz_cmp (alphaPower, beta) != 0) {
            mpz_mul (alphaPower, alphaPower, alpha);
            mpz_mod (alphaPower, alphaPower, p);
            power++;
        }
        mpz_set_ui (result, power);

    } else {

        bool ok = pollard_rho (result, alpha, p, n, beta, rstate, false, x, a, b, x1, a1, b1);

        while (!ok) {
            //printf ("failed, running with random start\n");
            ok = pollard_rho (result, alpha, p, n, beta, rstate, true, x, a, b, x1, a1, b1);
            runCount++;
        }

    }

    //gmp_printf ("\t result = %Zd\n", result);
    return runCount;
}

int pollard_rho (mpz_t result, mpz_t alpha, mpz_t p, mpz_t n, mpz_t beta, gmp_randstate_t rstate) {
    mpz_t x, a, b, x1, a1, b1, alphaPower;

    mpz_init (x); mpz_init (a); mpz_init (b);
    mpz_init (x1); mpz_init (a1); mpz_init (b1);
    mpz_init (alphaPower);

    return pollard_rho (result, alpha, p, n, beta, rstate, x, a, b, x1, a1, b1, alphaPower);

    mpz_clear (x); mpz_clear (a); mpz_clear (b);
    mpz_clear (x1); mpz_clear (a1); mpz_clear (b1);
    mpz_clear (alphaPower);
}

/*
 * Pollard's Rho algorithm for discrete logs.
 * See Handbook of Applied Cryptography, algorithm 3.60 page 106.
 *
 * Computes log base alpha of beta, where all operations are done
 * in Z_p and n is the order of alpha in Z_p.
 *
 * Uses pollard_rho to compute logs in groups of prime order.
 */
void pohlig_hellman (mpz_t result, mpz_t alpha, mpz_t p, CFactoredInteger *n,
                     mpz_t beta, gmp_randstate_t rstate, PH_ARG_LIST) {

    unsigned int c;

    PrimePower *factors = n->factors;
    unsigned int nFactors = n->nFactors;
    
    /*
     * Note that alpha^x = beta = alpha^{x_0} alpha^{x_1 q} alpha^{x_2 q^2}
     *                     ... alpha^{x_{c-1} q^{c-1}} alpha^{q^c n} 
     * The key idea here is that raising beta to
     * the n/q^j power eliminates all factors in the above expansion
     * with a q^k in their exponent, where k >= j.
     */

    mpz_set_ui (result, 0);

    for (unsigned int i=0; i < nFactors; i++) {

        // calculate xi = x mod q^c, where q^c is the current prime power
        mpz_set (betaStripped, beta);

        mpz_set (q, factors[i].prime);
        c = factors[i].power;

        //gmp_printf ("[%u] %Zd^%u\n", i, q, c);

        mpz_div (ndivq, n->value, q);
        //gmp_printf ("ndivq = %Zd\n", ndivq);
        mpz_powm (alphaBar, alpha, ndivq, p);
        //gmp_printf ("alphaBar = %Zd\n", alphaBar);

        mpz_powm (betaBar, beta, ndivq, p);
        //gmp_printf ("betaBar = %Zd\n", betaBar);
        mpz_set_ui (qj, 1);

        pollard_rho (lj, alphaBar, p, q, betaBar, rstate, x, a, b, x1, a1, b1, alphaPower);
        mpz_set (xi, lj);

        //mpz_set (xi, lj); // x_0 = log_alphaBar (beta^(n/q))

        for (unsigned int j=1; j < c; j++) {

            // strip off the alpha^(lj q^j) term we just found
            mpz_neg (lj, lj);
            mpz_powm (alphaStrip, alpha, lj, p);
            mpz_mul (betaStripped, betaStripped, alphaStrip);
            mpz_mod (betaStripped, betaStripped, p);
            //gmp_printf ("betaStripped = %Zd\n", betaStripped);
            
            // now raise to the n/q^(j+2)
            mpz_div (ndivq, ndivq, q);
            //gmp_printf ("ndivq = %Zd\n", ndivq);
            mpz_powm (betaBar, betaStripped, ndivq, p);

            // l_j = log_alphaBar (betaBar)) = log_{alpha^(n/q)} (betaStripped^(n/q^j+2))
            pollard_rho (lj, alphaBar, p, q, betaBar, rstate, x, a, b, x1, a1, b1, alphaPower);

            mpz_mul (qj, qj, q); // mods?
            mpz_mul (lj, lj, qj);
            mpz_add (xi, xi, lj);

        }

        //gmp_printf ("[%u] %Zd^%u: xi = %Zd\n", i, q, c, xi);

        //mpz_pow_ui (qj, q, c);
        mpz_div (ndivq, n->value, factors[i].value);
        mpz_gcdext (lj, betaBar, NULL, ndivq, factors[i].value);
        mpz_mul (betaStripped, betaBar, xi);
        mpz_mul (betaBar, betaStripped, ndivq);
        //mpz_mod (betaBar, betaBar, n->value);
        mpz_add (result, result, betaBar);
        mpz_mod (result, result, n->value);

    }

}

void pohlig_hellman (mpz_t result, mpz_t alpha, mpz_t p, CFactoredInteger *n,
                     mpz_t beta, gmp_randstate_t rstate) {
    /*
    mpz_t xi, q, qj, lj, ndivq;
    mpz_t alphaBar, betaBar;
    mpz_t betaStripped, alphaStrip;
    mpz_t x, a, b, x1, a1, b1;

    mpz_init (x); mpz_init (a); mpz_init (b);
    mpz_init (x1); mpz_init (a1); mpz_init (b1);

    mpz_init (xi); mpz_init (q); mpz_init (qj); mpz_init (lj); mpz_init (ndivq);
    mpz_init (alphaBar); mpz_init (betaBar);
    mpz_init (betaStripped); mpz_init (alphaStrip);

    */
    // 15 mpz_t local variables!!!

    DECLARE_PH_LOCALS
    INIT_PH_LOCALS(p)
    pohlig_hellman (result, alpha, p, n, beta, rstate, LIST_PH_LOCALS);
    CLEAR_PH_LOCALS

/*
    mpz_clear (xi); mpz_clear (q); mpz_clear (qj); mpz_clear (lj); mpz_clear (ndivq);
    mpz_clear (alphaBar); mpz_clear (betaBar);
    mpz_clear (betaStripped); mpz_clear (alphaStrip);

    mpz_clear (x); mpz_clear (a); mpz_clear (b);
    mpz_clear (x1); mpz_clear (a1); mpz_clear (b1);
    */
}

