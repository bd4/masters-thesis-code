/*
 * =====================================================================================
 *
 *       Filename:  DiskMimAttack.cc
 *
 *    Description:  Implementation of the attack outlined in
 *                  "Why Textbook ElGamal and RSA Encryption are Insecure"
 *                  by Boneh, Joux, and Nguyen section 3.1
 *                  With 32-bit hashes and tokyocabinet B+ tree for table storage.
 *
 *        Version:  1.0
 *        Created:  07/15/2008 03:31:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

/*
 * TODO
 * Load/store table from disk
 */

#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <math.h>
#include <unistd.h>
//#include <time.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"

#include "MpzList.h"
#include "ElgamalAttack.h"
#include "DiskMimAttack.h"

// We want to time the table build on batch runs. This compilation
// option will make sure that the table will be built even if the
// bdb file already exists, so we get full statistics on multiple
// runs even if nobody deletes the table.
#define ALWAYS_BUILD_TABLE

DiskMimAttack::DiskMimAttack (ElgamalCryptosystem *elg, char *fileName, unsigned int b1, unsigned int b2) {
    bits1 = b1;
    bits2 = b2;
    e = elg;
    this->fileName = fileName;
    bdb = tcbdbnew();
    //int64_t bnum = (2 << bits1); // suggested 1 to 4 times # pages to be stored, default 32749
    //tcbdbtune (bdb, 512, 1024, bnum, -1, -1, BDBTLARGE);
    tcbdbtune (bdb, 0, 0, 0, -1, -1, BDBTLARGE);
    tcbdbsetcmpfunc (bdb, tcbdbcmpint32, NULL);
}

DiskMimAttack::~DiskMimAttack () {

    /* close the database */
    if(!tcbdbclose(bdb)){
        int ecode = tcbdbecode(bdb);
        fprintf(stderr, "close error: %s\n", tcbdberrmsg(ecode));
    }

    tcbdbdel (bdb);

}

static inline UIntType hash (mpz_t n) {
    return (UIntType) mpz_get_ui (n);
    //return x && ((1l << bits) - 1);
}

/*
 * Build a table of pairs (key, value) sorted on key, where
 * key = hash (delta1^q mod p) and value = delta1.
 */
bool DiskMimAttack::buildTable (gmp_randstate_t rstate) {
 
    int ecode;
    UIntType key, value;

#ifndef ALWAYS_BUILD_TABLE
    if (access (fileName, R_OK) == 0) {
        // open existing database for this cryptosystem
        if (!tcbdbopen (bdb, fileName, BDBOREADER)) {
            ecode = tcbdbecode (bdb);
            fprintf (stderr, "open error: %s\n", tcbdberrmsg(ecode));
        } else {
            printf ("Using existing table.\n");
            return false;
        }
    }
#endif

    // build new table
    if (!tcbdbopen (bdb, fileName, BDBOWRITER | BDBOCREAT | BDBOTRUNC)) {
        ecode = tcbdbecode(bdb);
        fprintf (stderr, "open error: %s\n", tcbdberrmsg(ecode));
        return false;
    }

    mpz_t delta1;
    mpz_t tmp;

    mpz_init_set_ui (delta1, 0);
    mpz_init (tmp);

    bool success = true;

    size_t max = (1l << bits1);

    // as i goes from 0 to 2^bits1 - 1, delta1 goes from 1 to 2^bits1
    for (size_t i = 0; i < max; i++) {
        mpz_add_ui (delta1, delta1, 1);

        value = (UIntType) mpz_get_ui (delta1);
        mpz_powm (tmp, delta1, e->baseOrder, e->prime);
        key = hash (tmp);

        /* store records */
        if (!tcbdbputdup (bdb, &key, sizeof (key), &value, sizeof (value))) {
            ecode = tcbdbecode(bdb);
            fprintf(stderr, "put error: %s\n", tcbdberrmsg(ecode));
            success = false;
            break;
        }
    }

    mpz_clear (tmp);
    mpz_clear (delta1);
    
    return success;

}

size_t DiskMimAttack::crackMessage (MpzList *results, const ElgamalCipherText ct,
                                    gmp_randstate_t rstate, size_t maxResults) {

    if (tcbdbpath(bdb) == NULL) {
        return 0;
    }

    size_t resultCount = 0;

    //BDBCUR *cur;
    UIntType *value = NULL;

    // TABLEDIR + cryptosystem + '_{bits}', allowing 3 digits for the bits
    /*
    char *fileName = (char *) malloc (strlen(TABLEDIR) + strlen(label) + 1 + 4);
    strncpy (fileName, TABLEDIR, strlen (TABLEDIR));
    strncpy (fileName + strlen(TABLEDIR), label, strlen(label));
    *(fileName + strlen(TABLEDIR) + strlen(label)) = '_';
    snprintf (fileName + strlen(TABLEDIR) + strlen(label) + 1, 4, "%d", bits1);
    */
    //printf ("fileName = '%s'\n", fileName);
    //mpz_set_ui (m, 0);
    //return;

    mpz_t delta2, uq, target, candidate, delta1;
    mpz_init_set_ui (delta2, 0);
    mpz_init_set (uq, ct.myk);

    //gmp_printf ("ct.myk = %Zd\n", uq);
    mpz_powm (uq, uq, e->baseOrder, e->prime);
    // TODO: fail if uq = 1?
    //gmp_printf ("u^q = %Zd\n", uq);

    mpz_init (target);
    mpz_init (candidate);
    mpz_init (delta1);

    size_t end = (1l << bits2);
    size_t matchCount = 0;
    UIntType targetHash; //, candidateHash;
    bool found = false;
    int i, size, max;
    TCLIST *list;
    while (mpz_cmp_ui (delta2, end) < 0) {
        mpz_add_ui (delta2, delta2, 1);
        //gmp_printf ("delta2 = %Zd\n", delta2);
        mpz_powm (target, delta2, e->baseOrder, e->prime);
        mpz_invert (target, target, e->prime);
        mpz_mul (target, target, uq);
        mpz_mod (target, target, e->prime);
        targetHash = hash (target);

        list = tcbdbget4 (bdb, &targetHash, sizeof (targetHash));
        if (list == NULL)
            continue;

        matchCount += tclistnum (list);
        max = tclistnum (list);
        /* traverse records */
        found = false;
        for (i = 0; i < max; i++) {
            value = (UIntType *) tclistval (list, i, &size);
            mpz_set_ui (candidate, *value);
            mpz_powm (candidate, candidate, e->baseOrder, e->prime);
            if (mpz_cmp (target, candidate) == 0) {
                found = true;
                break;
            }
        }

        if (found) {
            mpz_mul_ui (target, delta2, *value);
            results->append (target);
            resultCount++;
        }
        
        if (list)
            tclistdel (list);

        if (maxResults > 0 && resultCount >= maxResults)
            break;
    }

    //double diff = difftime (time (NULL), start);
    //printf ("Search time with bits1 = '%d': %dm %ds\n", bits1, (int) floor (diff / 60), ((int)diff) % 60);

    mpz_clear (target);
    mpz_clear (candidate);
    mpz_clear (delta1);
    mpz_clear (delta2);
    mpz_clear (uq);

    printf ("diskMimAttack match count: %zu\n", matchCount);

    return resultCount;

}
