/*
 * Main program for executing different attacks.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <gmp.h>
#include <time.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"
#include "include/dlog.h"

#include "MpzList.h"
#include "ElgamalAttack.h"
#include "MimAttack.h"
#include "HashMimAttack.h"
#include "HashMimAttack2.h"
#include "HashMimAttack3.h"
#include "HashMimAttack4.h"
#include "DiskMimAttack.h"
#include "TwoTableAttack.h"

//const char *BASEDIR = "cryptosystems/";

void usage () {
    printf ("mimattack -n attackName -t tableFilePath -b messageBits -c cryptosystemFilePath message1Path [message2Path...]\n");
}

int main (int argc, char **argv) {
    
    char *csFilePath = NULL;
    char *attackName = NULL;
    char *tableFilePath = NULL;
    //char **messageFilePaths = NULL;

    unsigned int messageBits = 0;
    unsigned int bits1 = 0;
    unsigned int bits2 = 0;

    gmp_randstate_t rstate;
    gmp_randinit_default (rstate);
    if (!seedRandState (rstate)) {
        fprintf (stderr, "Failed to seed random state, exiting\n");
        exit (EXIT_FAILURE);
    }

    char *endptr = NULL;
    int opt;
    while ((opt = getopt (argc, argv, "n:t:b:c:")) != -1) {
        switch (opt) {
        case 'c':
            csFilePath = optarg;
            break;
        case 'b':
            messageBits = strtoul (optarg, &endptr, 10);
            if (*endptr == '\0') {
                bits1 = bits2 = (messageBits >> 1);
            } else {
                usage ();
                exit (1);
            }
            break;
        case 'n':
            attackName = optarg;
            break;
        case 't':
            tableFilePath = optarg;
            break;
        case ':':
        case '?':
            usage ();
            exit (1);
        }
    }

    if (messageBits == 0) {
        printf ("ERR: messageBits not specified with -b, exiting\n");
        usage ();
        exit (EXIT_FAILURE);
    }

    //if (optind < argc) {
    //    *messageFilePaths = argv[optind];
    //}

    FILE *f = fopen (csFilePath, "r");
    ElgamalCryptosystem e;
    e.read (f);
    fclose (f);

    ElgamalAttack *attack = NULL;
    
    if (strcmp (attackName, "mim") == 0) {
        attack = new MimAttack (&e, bits1, bits2);
    } else if (strcmp (attackName, "hashmim") == 0) {
        attack = new HashMimAttack (&e, bits1, bits2);
    } else if (strcmp (attackName, "hashmim2") == 0) {
        attack = new HashMimAttack2 (&e, bits1, bits2, tableFilePath);
    } else if (strcmp (attackName, "hashmim3") == 0) {
        attack = new HashMimAttack3 (&e, bits1, bits2, tableFilePath);
    } else if (strcmp (attackName, "hashmim4") == 0) {
        attack = new HashMimAttack4 (&e, bits1, bits2, tableFilePath);
    } else if (strcmp (attackName, "diskmim") == 0) {
        attack = new DiskMimAttack (&e, tableFilePath, bits1, bits2);
    } else if (strcmp (attackName, "2table") == 0) {
        attack = new TwoTableAttack (&e, bits1, bits2);
    } else {
        printf ("Unknown attack '%s', exiting\n", attackName);
        exit (EXIT_FAILURE);
    }

    printf ("INFO: using attack '%s'\n", attack->getAttackName());
    printf ("INFO: bits1 = %u, bits2 = %u\n", bits1, bits2);

    mpz_t m, uq, deltaq;
    ElgamalCipherText ct;
    mpz_init (m); mpz_init (uq); mpz_init (deltaq);
    mpz_init (ct.gk); mpz_init (ct.myk);

    printf ("INFO: using the following cryptosystem, from file '%s'\n", csFilePath);
    e.print ();

    time_t start = time (NULL);
    printf ("INFO: Building table...\n");
    bool builtNew = attack->buildTable (rstate);
    double diff = difftime (time (NULL), start);
    if (builtNew) {
        printf ("TIME[table,bits1=%u]: %dm %ds : %ld\n", bits1, (int) floor (diff / 60),
                                                         ((int)diff) % 60, (long)diff);
    }

    MpzList results (20, 20);
    MpzList resultsUnique (10, 10);
    size_t resultCount;
    for (int i = optind; i < argc; i++) {
        f = fopen (argv[i], "r");

        mpz_inp_raw (m, f);
        mpz_inp_raw (ct.gk, f);
        mpz_inp_raw (ct.myk, f);

        mpz_powm (uq, ct.myk, e.baseOrder, e.prime);

        fclose (f);

        printf ("message:\n");
        gmp_printf ("\tm     = %Zd (%u bits)\n", m, mpz_sizeinbase (m, 2));
        gmp_printf ("\tg^k   = %Zd (%u bits)\n", ct.gk, mpz_sizeinbase (ct.gk, 2));
        gmp_printf ("\tm*y^k = %Zd (%u bits)\n", ct.myk, mpz_sizeinbase (ct.myk, 2));

        time_t start = time (NULL);
        resultCount = attack->crackMessage (&results, ct, rstate);
        diff = difftime (time (NULL), start);
        printf ("TIME[crack,file=%s]: %dm %ds : %ld\n", argv[i], (int) floor (diff / 60),
                                                        ((int)diff) % 60, (long)diff);
        printf ("RESULTS[file=%s]: %zu\n", argv[i], resultCount);
        // iterate through results, then clear
        bool found = false;
        gmp_printf ("actual message: %Zd\n", m);
        for (size_t j = 0; j < resultCount; j++) {
            if (!resultsUnique.find (NULL, results[j])) {
                resultsUnique.append (results[j]);
                // verify that this result really works
                mpz_powm (deltaq, results[j], e.baseOrder, e.prime);
                if (mpz_cmp (deltaq, uq) != 0) {
                    gmp_printf ("!!results[%zu] = %Zd\n", j, results[j]);
                    printf ("ERR: found result which doesn't work!\n");
                } else if (mpz_cmp (results[j], m) == 0) {
                    gmp_printf ("**results[%zu] = %Zd\n", j, results[j]);
                    found = true;
                } else {
                    gmp_printf ("  results[%zu] = %Zd\n", j, results[j]);
                }
            }
        }
        printf ("URESULTS[file=%s]: %zu\n", argv[i], resultsUnique.getSize());
        results.clear ();
        resultsUnique.clear();
        //gmp_printf ("crack result   : %Zd\n", result);
        //gmp_printf ("expected result: %Zd\n", m);
        if (!found) {
            printf ("ERR: message not found\n");
        }

    }

    mpz_clear (m); mpz_clear (uq); mpz_clear (deltaq);
    mpz_clear (ct.gk); mpz_clear (ct.myk);
    gmp_randclear (rstate);

    delete attack;


/*
    if (mpz_cmp_ui (m, 0) == 0) {
        printf ("Generating random splitting message with %d bits:\n", messageBits);
        mpz_urandomb (m, rstate, halfBits - 1);
        mpz_setbit (m, halfBits - 1);
        mpz_urandomb (m2, rstate, halfBits - 1);
        mpz_setbit (m2, halfBits - 1);
        gmp_printf ("message factors = %Zd; %Zd\n", m, m2);

        mpz_mul (m, m, m2);
    }

    e->encrypt (&ct, m, rstate);

    gmp_printf ("message = %Zd\n", m);
    gmp_printf ("message bits = %d\n", messageBits);
    gmp_printf ("half bits = %d\n", halfBits);
    gmp_printf ("ciphertext (g^k, m*y^k) = (%Zd, %Zd)\n", ct.gk, ct.myk);

    printf ("Attempting to crack message...\n");
    if (useDisk) {
        if (!label) {
            printf ("Stored cryptosystem required for disk method, not running.\n");
            usage ();
            exit (1);
        }
        diskMim1CrackMessage (m2, ct, e, label, halfBits, halfBits);
    } else if (useHash) {
        hashMim1CrackMessage (m2, ct, e, halfBits, halfBits);
    } else {
        mim1CrackMessage (m2, ct, e, halfBits, halfBits);
    }

    gmp_printf ("crack result = %Zd\n", m2);
    if (mpz_cmp (m, m2) == 0) {
        printf ("SUCESS!!!\n");
    }

    delete e;

*/

}
