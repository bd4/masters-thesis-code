/*
 * Modification of modularExponentiation.cc adding the multiplication and inversion
 * needed when cracking a message vs building the table. The "ciphertext" is chosen
 * randomly.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmp.h>
#include <sys/time.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"

void usage (char *argv0) {
    printf ("Usage: %s [-o] bits cryptosystemFilePath\n", argv0);
}

int main (int argc, char **argv) {

    int firstArg = 1;
    bool optimize = false;
    if (argc == 4) {
        if (strcmp (argv[1], "-o") == 0) {
            optimize = true;
            firstArg = 2;
        } else {
            usage (argv[0]);
            exit (EXIT_FAILURE);
        }
    } else if (argc != 3) {
        usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    // process bits argument
    char *endptr;
    unsigned long bits = strtoul (argv[firstArg], &endptr, 10);
    if (*endptr != '\0') {
        usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    // process cryptosystem argument
    ElgamalCryptosystem *e = new ElgamalCryptosystem ();
    FILE *f = fopen (argv[firstArg+1], "r");
    if (f == NULL) {
        perror (NULL);
        exit (EXIT_FAILURE);
    }
    e->read (f);
    //e->print ();
    fclose (f);

    gmp_randstate_t rstate;
    gmp_randinit_default (rstate);
    if (!seedRandState (rstate)) {
        fprintf (stderr, "Failed to seed random state, exiting\n");
        exit (EXIT_FAILURE);
    }

    unsigned long max = (1ul << bits);

    mpz_t delta1;
    mpz_t tmp, exp, uq;

    size_t primeBits = mpz_sizeinbase (e->prime, 2);
    mpz_init_set_ui (delta1, 0);
    mpz_init2 (tmp, primeBits);
    mpz_init2 (uq, primeBits);

    mpz_urandomm (uq, rstate, e->prime);
    mpz_setbit (uq, primeBits-1); // make sure uq is full size

    mpz_init_set (exp, e->baseOrder); 
    if (optimize) {
        mpz_neg (exp, exp);
    }

    timeval start, end;
    gettimeofday (&start, NULL);

    for (unsigned long i = 0; i < max; i++) {
        mpz_add_ui (delta1, delta1, 1);

        mpz_powm (tmp, delta1, exp, e->prime);
        if (!optimize)
            mpz_invert (tmp, tmp, e->prime);
        mpz_mul (tmp, tmp, uq);
        mpz_mod (tmp, tmp, e->prime);

    }
    
    gettimeofday (&end, NULL);

    long sdiff = end.tv_sec - start.tv_sec;
    long udiff = 0;
    if (start.tv_usec > end.tv_usec) {
        sdiff--;
        udiff = 1000000l - start.tv_usec + end.tv_usec;
    } else {
        udiff = end.tv_usec - start.tv_usec;
    }

    ldiv_t min = ldiv (sdiff, 60);
    ldiv_t msec = ldiv (udiff, 1000);
    printf ("[%s %lu %s]: %ldmin %lds %ldms %ldus (%ld.%6ld tot sec)\n",
            "^n*inv", bits, argv[firstArg+1],
            min.quot, min.rem, msec.quot, msec.rem, sdiff, udiff);

    mpz_clear (delta1);
    mpz_clear (tmp);
    mpz_clear (exp);
    gmp_randclear (rstate);

    delete e;
 
    return EXIT_SUCCESS;

}
