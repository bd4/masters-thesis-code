/*
 * Program to determine if storing modular exponentation computations to a file
 * could improve attack performance.
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
    printf ("Usage: %s [-r] bits cryptosystemFilePath\n", argv0);
}

int main (int argc, char **argv) {

    int firstArg = 1;
    bool read = false;
    if (argc == 4) {
        if (strcmp (argv[1], "-r") == 0) {
            firstArg = 2;
            read = true;
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

    size_t max = (1ul << bits);

    mpz_t delta1;
    mpz_t tmp, exp;

    mpz_init_set_ui (delta1, 0);
    mpz_init2 (tmp, mpz_sizeinbase (e->prime, 2));
    mpz_init_set (exp, e->baseOrder); 

    timeval start, end;
    gettimeofday (&start, NULL);

    if (read) {
        f = fopen ("tmpmodexp.bin", "r");
        if (f == NULL) {
            perror ("Failed to open tmp file, exiting");
            exit (EXIT_FAILURE);
        }
        for (size_t i = 0; i < max; i++) {
            if (!mpz_inp_raw (tmp, f)) {
                fprintf (stderr, "Read failed at %zu\n", i);
            }
        }
    } else {
        f = fopen ("tmpmodexp.bin", "w");
        if (f == NULL) {
            perror ("Failed to open tmp file, exiting");
            exit (EXIT_FAILURE);
        }

        //printf ("Generating table...\n");
        for (size_t i = 0; i < max; i++) {
            mpz_add_ui (delta1, delta1, 1);

            mpz_powm (tmp, delta1, exp, e->prime);

            if (!mpz_out_raw (f, tmp)) {
                fprintf (stderr, "Write failed at %zu\n", i);
            }
        }
    }

    if (f != NULL)
        fclose (f);
        
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
            read ? "read": "write", bits, argv[firstArg+1],
            min.quot, min.rem, msec.quot, msec.rem, sdiff, udiff);

    mpz_clear (delta1);
    mpz_clear (tmp);
    mpz_clear (exp);

    delete e;
 
    return EXIT_SUCCESS;

}
