/*
 * ============================================================================
 *
 *       Filename:  splittingProbabilities.cc
 *
 *    Description:  Program to experimentally determine splitting probabilities.
 *
 *        Version:  1.0
 *        Created:  11/08/2008 10:28:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmp.h>

#include "include/types.h"
#include "include/randomhelpers.h"

void usage (char *argv0) {
    printf ("Usage: %s [-f] bits b1 b2 count\n", argv0);
}

void printArray (unsigned int a[], int n) {
    for (int i = 0; i < n; i++) {
        printf ("%d ", a[i]);
    }
    printf ("\n");
}

void generateRandomPreFactoredIntegers (unsigned long bits, unsigned long b1,
                                        unsigned long b2, unsigned long count) {

    unsigned long b12max = (b1 > b2) ? b1 : b2;
    unsigned long b12min = (b1 < b2) ? b1 : b2;

    CFactoredInteger f;
    gmp_randstate_t rstate;
    mpz_t max, splitMax, splitMin;
    mpz_t s1, s2, tmp;
    
    gmp_randinit_default (rstate);
    seedRandState (rstate);

    mpz_init (s1); mpz_init (s2); mpz_init (tmp);
    mpz_init_set_ui (max, 1);
    mpz_init_set_ui (splitMax, 1);
    mpz_init_set_ui (splitMin, 1);

    mpz_mul_2exp (max, max, bits);
    mpz_sub_ui (max, max, 1);
    mpz_mul_2exp (splitMax, splitMax, b12max);
    mpz_mul_2exp (splitMin, splitMin, b12min);

    unsigned int powers[bits];

    unsigned long splitCount = 0;
    unsigned long noSplitCount = 0;
    unsigned int i;
    bool end;
    for (unsigned int k = count; k > 0; k--) {
        f.random (max, rstate);
        //f.print ();
        if (mpz_cmp (f.factors[0].prime, splitMax) > 0) {
            noSplitCount++;
        } else if (mpz_cmp (f.value, splitMax) <= 0) {
            splitCount++;
        } else {
            mpz_set_ui (s1, 1);
            mpz_set (s2, f.value);
            for (i = 0; i < f.nFactors; i++) {
                powers[i] = 0;
            }
            
            end = false;
            while (1) {
                i = f.nFactors-1;
                while (powers[i] == f.factors[i].power) {
                    if (i > 0) {
                        powers[i] = 0;
                        mpz_fdiv_q (s1, s1, f.factors[i].value);
                        mpz_mul (s2, s2, f.factors[i].value);
                        i--;
                    } else {
                        noSplitCount++;
                        end = true;
                        break;
                    }
                }
                if (end) { break; }

                powers[i]++;
                //printArray (powers, f.nFactors);
                mpz_mul (s1, s1, f.factors[i].prime);
                //if (mpz_cmp (s1, splitMin) > 0)
                //    break;
                mpz_fdiv_q (s2, s2, f.factors[i].prime);
                if (mpz_cmp (s2, splitMax) <= 0 && mpz_cmp (s1, splitMin) <= 0) {
                    mpz_mul (tmp, s1, s2);
                    if (mpz_cmp (tmp, f.value) != 0) {
                        gmp_printf ("ERROR: bad split value = %Zd != %Zd = prod\n", f.value, tmp);
                        exit (EXIT_FAILURE);
                    }
                    splitCount++;
                    break;
                    //gmp_printf ("Split found: %Zd * %Zd = %Zd\n", s1, s2, tmp);
                    //printArray (powers, f.nFactors);
                }
                //if (carry)
                //    i = f.nFactors - 1;
            }
        }
    }

    printf ("   split: %lu\n", splitCount);
    printf ("no split: %lu\n", noSplitCount);

    printf ("[%lu %lu %lu] ", bits, b1, b2);
    printf ("%0.1f\n", (splitCount * 100.00)/count);

    mpz_clear (s1); mpz_clear (s2); mpz_clear (tmp);
    mpz_clear (max);
    mpz_clear (splitMax);
    mpz_clear (splitMin);
    gmp_randclear (rstate);

}

void factorRandomIntegers (unsigned long bits, unsigned long b1, unsigned long b2,
                           unsigned long count) {

    unsigned long b12max = (b1 > b2) ? b1 : b2;
    unsigned long b12min = (b1 < b2) ? b1 : b2;

    CFactoredInteger f;
    gmp_randstate_t rstate;
    mpz_t max, splitMax, splitMin;
    mpz_t s1, s2, tmp;
    
    gmp_randinit_default (rstate);
    seedRandState (rstate);

    mpz_init (s1); mpz_init (s2); mpz_init (tmp);
    mpz_init_set_ui (max, 1);
    mpz_init_set_ui (splitMax, 1);
    mpz_init_set_ui (splitMin, 1);

    mpz_mul_2exp (max, max, bits);
    mpz_sub_ui (max, max, 1);
    mpz_mul_2exp (splitMax, splitMax, b12max);
    mpz_mul_2exp (splitMin, splitMin, b12min);

    unsigned int powers[bits];

    unsigned long splitCount = 0;
    unsigned long noSplitCount = 0;
    unsigned int i;
    bool end;
    for (unsigned int k = count; k > 0; k--) {
        mpz_urandomm (tmp, rstate, max); // 0 to max - 1
        mpz_add_ui (tmp, tmp, 1); // 1 to max
        f.factorValue (tmp);
        //f.print ();
        if (mpz_cmp (f.factors[0].prime, splitMax) > 0) {
            noSplitCount++;
        } else if (mpz_cmp (f.value, splitMax) <= 0) {
            splitCount++;
        } else {
            mpz_set_ui (s1, 1);
            mpz_set (s2, f.value);
            for (i = 0; i < f.nFactors; i++) {
                powers[i] = 0;
            }
            
            end = false;
            while (1) {
                i = 0;
                while (powers[i] == f.factors[i].power) {
                    if (i + 1 < f.nFactors) {
                        powers[i] = 0;
                        mpz_fdiv_q (s1, s1, f.factors[i].value);
                        mpz_mul (s2, s2, f.factors[i].value);
                        i++;
                    } else {
                        noSplitCount++;
                        end = true;
                        break;
                    }
                }
                if (end) { break; }

                powers[i]++;
                //printArray (powers, f.nFactors);
                mpz_mul (s1, s1, f.factors[i].prime);
                //if (mpz_cmp (s1, splitMin) > 0)
                //    break;
                mpz_fdiv_q (s2, s2, f.factors[i].prime);
                if (mpz_cmp (s2, splitMax) <= 0 && mpz_cmp (s1, splitMin) <= 0) {
                    mpz_mul (tmp, s1, s2);
                    if (mpz_cmp (tmp, f.value) != 0) {
                        gmp_printf ("ERROR: bad split value = %Zd != %Zd = prod\n", f.value, tmp);
                        exit (EXIT_FAILURE);
                    }
                    splitCount++;
                    break;
                    //gmp_printf ("Split found: %Zd * %Zd = %Zd\n", s1, s2, tmp);
                    //printArray (powers, f.nFactors);
                }
                //if (carry)
                //    i = f.nFactors - 1;
            }
        }
    }

    printf ("   split: %lu\n", splitCount);
    printf ("no split: %lu\n", noSplitCount);

    printf ("[%lu %lu %lu] ", bits, b1, b2);
    printf ("%0.1f\n", (splitCount * 100.00)/count);

    mpz_clear (s1); mpz_clear (s2); mpz_clear (tmp);
    mpz_clear (max);
    mpz_clear (splitMax);
    mpz_clear (splitMin);
    gmp_randclear (rstate);

}

int main (int argc, char **argv) {
    
    char *argv0 = argv[0];
    bool useFactoredRandom = false;

    if (argc == 6) {
        if (strcmp (argv[1], "-f") == 0) {
            useFactoredRandom = true;
            argv++;
        } else {
            usage (argv0);
            exit (EXIT_FAILURE);
        }
    } else if (argc != 5) {
        usage (argv0);
        exit (EXIT_FAILURE);
    }


    unsigned long bits, b1, b2, count;
    char *endptr;
    bits = strtoul (argv[1], &endptr, 10);
    if (*endptr != '\0') {
        usage (argv0);
        exit (EXIT_FAILURE);
    }

    b1 = strtoul (argv[2], &endptr, 10);
    if (*endptr != '\0') {
        usage (argv0);
        exit (EXIT_FAILURE);
    }

    b2 = strtoul (argv[3], &endptr, 10);
    if (*endptr != '\0') {
        usage (argv0);
        exit (EXIT_FAILURE);
    }

    count = strtoul (argv[4], &endptr, 10);
    if (*endptr != '\0') {
        usage (argv0);
        exit (EXIT_FAILURE);
    }

    if (useFactoredRandom) {
        factorRandomIntegers (bits, b1, b2, count);
    } else {
        generateRandomPreFactoredIntegers (bits, b1, b2, count);
    }

}
