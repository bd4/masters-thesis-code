/* 
 * Program to test encryption time of different elgamal cryptosystems.
 *
 * Copyright (C) 2008 by Bryce Allen
 *
 * Distributed under the MIT license: see COPYING.MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gmp.h>
#include <sys/time.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"

void usage (char *argv0) {
    printf ("%s [-v] [-m msgBits] [-c count] cryptoSystemFilePath\n", argv0);
}

// TODO: requires factoring of n?
//bool isPrimitive (mpz_t a, mpz_t n);

int main(int argc, char **argv) {


    gmp_randstate_t rstate;
    gmp_randinit_default (rstate);
    seedRandState (rstate);

    bool verbose = false;
    char *filePath = NULL;
    
    char *endptr;
    int opt;
    int count = 0;
    unsigned int messageBits = 0;
    while ((opt = getopt (argc, argv, "m:c:v")) != -1) {
        switch (opt) {
        case 'v':
            verbose = true;
            break;
        case 'c':
            count = strtol (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage (argv[0]);
                exit (1);
            }
            break;
        case 'm':
            messageBits = strtoul (optarg, &endptr, 10);
            if (*endptr != '\0') {
                usage (argv[0]);
                exit (1);
            }
            break;
        case 'f':
            filePath = optarg;
            break;
        case ':':
        case '?':
            usage (argv[0]);
            exit (EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        filePath = argv[optind];
    } else {
        usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    mpz_t m, m2, max;
    ElgamalCipherText ct;
    mpz_init (ct.gk);
    mpz_init (ct.myk);
    mpz_init (m2);
    mpz_init (m);
    mpz_init (max);

    int returnValue = EXIT_SUCCESS;

    ElgamalCryptosystem *e = NULL;

    if (filePath != NULL) {
        e = new ElgamalCryptosystem ();
        FILE *f = fopen (filePath, "r");
        if (f == NULL) {
            perror (NULL);
            exit (EXIT_FAILURE);
        }
        e->read (f);
        fclose (f);
    } else {
        usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    if (verbose) { e->print (); }

    if (messageBits > 0) {
        mpz_set_ui (max, 1);
        mpz_mul_2exp (max, max, messageBits);
        mpz_sub_ui (max, max, 1);
    } else {
        mpz_set (max, e->prime);
        mpz_sub_ui (max, max, 1);
    }

    timeval start, end;
    gettimeofday (&start, NULL);

    for (int i=count; i > 0; i--) {
        mpz_urandomm (m, rstate, max); // 0 to max-1
        mpz_add_ui (m, m, 1); // 1 to max

        //if (verbose)
        //    gmp_printf ("%d: %Zd\n", i, m);

        //e->encrypt (&ct, m, rstate);
        //e->decrypt (m2, ct);
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
    printf ("[%u %s]: %ldmin %lds %ldms %ldus (%ld.%6ld tot sec)\n", messageBits, filePath,
            min.quot, min.rem, msec.quot, msec.rem, sdiff, udiff);

    delete (e);

    mpz_clear (ct.gk);
    mpz_clear (ct.myk);
    mpz_clear (m);
    mpz_clear (m2);
    mpz_clear (max);
    gmp_randclear (rstate);

    return returnValue;
}
