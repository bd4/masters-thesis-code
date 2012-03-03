/*
 * =====================================================================================
 *
 *       Filename:  elgamalmgr.cc
 *
 *    Description:  Command line program to create an elgamal cryptosystem
 *                  and save for use by mimattack and other programs. Will
 *                  also print information about a saved cryptosystem.
 *
 *        Version:  1.0
 *        Created:  08/13/2008 11:19:14 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gmp.h>
#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"
//#include "resourcefilenames.h"

typedef enum { NONE, CREATE_CS, CREATE_MSG, DISPLAY_CS, DISPLAY_MSG } Mode;

//const char *BASEDIR = "cryptosystems/";

void usage () {
    printf ("elgamalmgr cc outfile [-p prime_bits] [-b base_bits] "
            "[-s smooth_bits -l smoothness_bit_limit] "
            "| cm outfile -m message_bits -c cryptosystem_infile "
            "| lc infile "
            "| lm infile\n");
}

Mode getDisplayModeFromExt (char *fileName) {
    char *p = fileName + strlen (fileName) - 3;

    if (strcmp (p, "elg") == 0) {
        return DISPLAY_CS;
    } else if (strcmp (p, "msg")) {
        return DISPLAY_MSG;
    } else {
        return NONE;
    }
}

int main (int argc, char **argv) {

    Mode mode = NONE;
    //char *label = NULL;
    char *endptr = NULL;

    size_t primeBits = 1024;
    size_t baseOrderBits = 512;
    size_t smoothBitLimit = 16;
    size_t smoothBits = 0;

    size_t msgBits = 40;

    if (argc < 3) {
        printf ("argc = %d\n", argc);
        usage ();
        exit (EXIT_FAILURE);
    }

    char *command = argv[1];

    char *inFileName = NULL;
    char *outFileName = NULL;
    if (strcmp (command, "cc") == 0) {
        mode = CREATE_CS;
        outFileName = argv[2];
    } else if (strcmp (command, "lc") == 0) {
        mode = DISPLAY_CS;
        inFileName = argv[2];
    } else if (strcmp (command, "cm") == 0) {
        mode = CREATE_MSG;
        outFileName = argv[2];
    } else if (strcmp (command, "lm") == 0) {
        mode = DISPLAY_MSG;
        inFileName = argv[2];
    }
    
    argc -= 2;
    argv += 2;

    int opt, x;
    while ((opt = getopt (argc, argv, "c:p:b:m:l:s:")) != -1) {
        switch (opt) {
        case 'c':
            if (mode == CREATE_MSG) {
                inFileName = optarg;
            } else {
                printf ("-c is only accepted for command 'cm'\n");
                usage ();
                exit (EXIT_FAILURE);
            }
            //printf ("create label = %s\n", label);
            break;
        case 'm':
            x = strtol (optarg, &endptr, 10);
            if (*endptr == '\0') {
                msgBits = x;
            } else {
                printf ("-m expects an integer argument\n");
                usage ();
                exit (EXIT_FAILURE);
            }
            break;
        case 'p':
            x = strtol (optarg, &endptr, 10);
            if (*endptr == '\0') {
                primeBits = x;
            } else {
                printf ("-p expects an integer argument\n");
                usage ();
                exit (EXIT_FAILURE);
            }
            break;
        case 'b':
            x = strtol (optarg, &endptr, 10);
            if (*endptr == '\0') {
                baseOrderBits = x;
            } else {
                printf ("-b expects an integer argument\n");
                usage ();
                exit (EXIT_FAILURE);
            }
            break;
        case 'l':
            x = strtol (optarg, &endptr, 10);
            if (*endptr == '\0') {
                smoothBitLimit = x;
            } else {
                printf ("-l expects an integer argument\n");
                usage ();
                exit (EXIT_FAILURE);
            }
            break;
        case 's':
            x = strtol (optarg, &endptr, 10);
            if (*endptr == '\0') {
                smoothBits = x;
            } else {
                printf ("-s expects an integer argument\n");
                usage ();
                exit (EXIT_FAILURE);
            }
            break;
        case ':':
        case '?':
            usage ();
            exit (1);
        }
    }

    /*
    char *fileName = (char *) malloc (strlen(BASEDIR) + strlen(label) + 1);
    strncpy (fileName,  BASEDIR, strlen (BASEDIR));
    strncpy (fileName + strlen(BASEDIR), label, strlen(label) + 1);
    printf ("%s\n", fileName);
    */

    gmp_randstate_t rstate;
    ElgamalCryptosystem *e;

    if (mode == DISPLAY_CS) {

        printf ("Attempting to display cryptosystem in '%s'\n", inFileName);

        FILE *f = fopen (inFileName, "r");
        if (f == NULL) {
            perror (NULL);
            exit (EXIT_FAILURE);
        }

        e = new ElgamalCryptosystem ();
        e->read (f);
        e->print ();
        //loadCryptosystem (&e, f);
        //printCryptosystem (&e);
        
        fclose (f);

        delete e;

    } else if (mode == CREATE_CS) {

        printf ("Creating cryptosystem:\n");
        printf (" primeBits = %zu, baseOrderBits = %zu\n", primeBits, baseOrderBits);
        if (smoothBits != 0) {
            printf (" smoothness parameters: bits = %zu, limit = %zu\n", smoothBits, smoothBitLimit);
        }

        gmp_randinit_default (rstate);
        seedRandState (rstate);

        if (primeBits != baseOrderBits) {
            e = new ElgamalCryptosystem (primeBits, baseOrderBits, rstate,
                                         smoothBits, smoothBitLimit);
        } else {
            e = new ElgamalCryptosystem (primeBits, rstate);
        }

        /*
        printf ("DEBUG: getting path\n");
        fileName = getCryptosystemFilePath (label, e);
        char *dirPath = getCryptosystemDirPath (label, e);

        printf ("Saving to: '%s'\n", fileName);
        printf ("dirPath: '%s'\n", dirPath);

        free (dirPath);
        */

        FILE *f = fopen (outFileName, "w");
        if (f == NULL) {
            perror (NULL);
            exit (EXIT_FAILURE);
        }
        e->print ();
        e->write (f);
        //printCryptosystem (&e);
        //saveCryptosystem (&e, f);
        fclose (f);

        //deleteCryptosystem (&e);
        delete e;
        gmp_randclear (rstate);

    } else if (mode == DISPLAY_MSG) {

        mpz_t m;
        ElgamalCipherText ct;

        mpz_init (m); mpz_init (ct.gk); mpz_init (ct.myk);

        FILE *f = fopen (inFileName, "r");
        if (f == NULL) {
            perror (NULL);
            exit (EXIT_FAILURE);
        }

        mpz_inp_raw (m, f);
        mpz_inp_raw (ct.gk, f);
        mpz_inp_raw (ct.myk, f);

        gmp_printf ("m = %Zd (%u bits)\n", m, mpz_sizeinbase (m, 2));
        gmp_printf ("g^k = %Zd (%u bits)\n", ct.gk, mpz_sizeinbase (ct.gk, 2));
        gmp_printf ("m*y^k = %Zd (%u bits)\n", ct.myk, mpz_sizeinbase (ct.myk, 2));
        //loadCryptosystem (&e, f);

        fclose (f);
        
        mpz_clear (m); mpz_clear (ct.gk); mpz_clear (ct.myk);

    } else if (mode == CREATE_MSG) {

        printf ("Attempting to create splitting message with cryptosystem '%s'\n", inFileName);
        printf ("    messageBits = %zu\n", msgBits);

        mpz_t m, m2;
        mpz_init (m); mpz_init (m2);

        int halfBits = ceil (msgBits / 2.0);

        FILE *f = fopen (inFileName, "r");
        if (f == NULL) {
            perror (NULL);
            exit (EXIT_FAILURE);
        }

        e = new ElgamalCryptosystem ();
        e->read (f);
        
        fclose (f);

        ElgamalCipherText ct;
        mpz_init (ct.gk);
        mpz_init (ct.myk);

        gmp_randinit_default (rstate);
        seedRandState (rstate);

        if (mpz_cmp_ui (m, 0) == 0) {
            printf ("Generating random splitting message with at most %zu bits:\n", msgBits);
            mpz_urandomb (m, rstate, halfBits - 1);
            mpz_setbit (m, halfBits - 1);
            mpz_urandomb (m2, rstate, halfBits - 1);
            mpz_setbit (m2, halfBits - 1);
            gmp_printf ("message factors = %Zd; %Zd\n", m, m2);

            mpz_mul (m, m, m2);
        }

        e->encrypt (&ct, m, rstate);

        delete e;

        f = fopen (outFileName, "w");
        if (f == NULL) {
            perror (NULL);
            exit (EXIT_FAILURE);
        }

        mpz_out_raw (f, m);
        mpz_out_raw (f, ct.gk);
        mpz_out_raw (f, ct.myk);

        gmp_printf ("message = %Zd\n", m);
        gmp_printf ("actual message bits = %zu\n", mpz_sizeinbase (m, 2));
        //gmp_printf ("half bits = %d\n", halfBits);
        gmp_printf ("ciphertext (g^k, m*y^k) = (%Zd, %Zd)\n", ct.gk, ct.myk);

        gmp_randclear (rstate);
        mpz_clear (m); mpz_clear (m2); mpz_clear (ct.gk); mpz_clear (ct.myk);

    } else {
        usage ();
        exit (EXIT_FAILURE);
    }
}
