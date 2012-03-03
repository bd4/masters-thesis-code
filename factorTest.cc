/*
 * =====================================================================================
 *
 *       Filename:  factorTest.cc
 *
 *    Description:  Program to test the factorValue method of CFactoredInteger.
 *
 *        Version:  1.0
 *        Created:  11/08/2008 04:29:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmp.h>

#include "include/types.h"
#include "include/randomhelpers.h"

void usage (char *argv0) {
    printf ("%s -r bits | integer\n", argv0);
}

int main(int argc, char **argv) {

    unsigned long bits = 0;
    char *endptr;

    mpz_t n;
    mpz_init (n);

    if (argc == 3) {
        if (strcmp (argv[1], "-r") == 0) {
            bits = strtoul (argv[2], &endptr, 10);
            if (*endptr != '\0') {
                usage (argv[0]);
                exit (EXIT_FAILURE);
            }
        } else {
            usage (argv[0]);
            exit (EXIT_FAILURE);
        }

    } else if (argc != 2) {
        usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    if (bits == 0) {
        mpz_set_str (n, argv[1], 0);
    }

    gmp_randstate_t rstate;

    gmp_randinit_default (rstate);
    seedRandState (rstate);

    CFactoredInteger fi;
    if (bits > 0) {
        mpz_urandomb (n, rstate, bits);
        mpz_add_ui (n, n, 1);
    }

    gmp_printf ("n = %Zd\n", n);
    fi.factorValue (n);
    fi.print ();

    mpz_set_ui (n, 1);
    for (unsigned int i=0; i < fi.nFactors; i++) {
        mpz_mul (n, n, fi.factors[i].value);
    }

    if (mpz_cmp (n, fi.value) != 0) {
        gmp_printf ("ERR: factored product = %Zd != original value = %Zd\n", fi.value, n);
    }

    mpz_clear (n);
    gmp_randclear (rstate);

}
