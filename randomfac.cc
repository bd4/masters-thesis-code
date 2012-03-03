/* 
 * Program to test the methods of CFactoredInteger which generate
 * random factored integers.
 *
 * Copyright (C) 2008 by Bryce Allen
 *
 * Distributed under the MIT license: see COPYING.MIT
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <gmp.h>

#include "include/types.h"
#include "include/randomhelpers.h"

void usage () {
    printf ("randomfac [-t targetBits] [-e exactBits] [-l smoothBitLimit]\n");
}

int main(int argc, char **argv) {

    int targetBits = 256;
    int exactBits = 0;
    int smoothBitLimit = 0;

    //printf ("argc = %d\n", argc);
  
    char *endptr;
    int opt;
    while ((opt = getopt (argc, argv, "s:t:e:")) != -1) {
        switch (opt) {
        case 't':
            targetBits = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case 'e':
            exactBits = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case 's':
            smoothBitLimit = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage ();
                exit (1);
            }
            break;
        case ':':
        case '?':
            usage ();
            exit (1);
        }
    }

    gmp_randstate_t rstate;

    gmp_randinit_default (rstate);
    seedRandState (rstate);

    
    CFactoredInteger fi;

    if (smoothBitLimit > 0) {
        if (exactBits > 0) {
            fi.randomSmoothExactBits (exactBits, rstate, smoothBitLimit);
        } else {
            fi.randomSmooth (targetBits, rstate, smoothBitLimit);
        }
    } else {
        fi.random (targetBits, rstate);
    }

    if (fi.hasMallocError ()) {
        fputs ("Failed to allocate random factored integer, exiting\n", stderr);
        exit (EXIT_FAILURE);
    }

    gmp_printf ("value = %Zd\n", fi.value);
    gmp_printf ("actual bits = %d\n", mpz_sizeinbase (fi.value, 2));
    
    printf ("\tnFactors = %d\n", fi.nFactors);

    for (unsigned int i = 0; i < fi.nFactors; i++) {
        gmp_printf ("\t%Zd, %d\n", fi.factors[i].prime, fi.factors[i].power);
    }

    gmp_randclear (rstate);

}
