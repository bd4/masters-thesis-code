/*
 * An implementation of the basic meet-in-the-middle attack outlined in
 * "Why Textbook ElGamal and RSA Encryption are Insecure"
 * by Boneh, Joux, and Nguyen section 3.1
 *
 * Copyright (C) 2008 by Bryce Allen
 *
 * Distributed under the MIT license: see COPYING.MIT
 */
#include <stdlib.h>
#include <gmp.h>
#include <math.h>
#include <time.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"

#include "MpzList.h"
#include "ElgamalAttack.h"
#include "MimAttack.h"

static void printTable (MpzTable *table) {
    MpzTableEntry e;
    for (size_t i=0; i < table->length; i++) {
        e = table->entries[i];
        gmp_printf ("%Zd: %Zd\n", e.key, e.value);
    }
}

static void deleteTable (MpzTable *table) {
    for (size_t i=0; i < table->length; i++) {
        mpz_clear (table->entries[i].key);
        mpz_clear (table->entries[i].value);
    }

    free (table->entries);
}

MimAttack::MimAttack (ElgamalCryptosystem *elg, unsigned int b1, unsigned int b2) {
    bits1 = b1;
    bits2 = b2;
    e = elg;
}


MimAttack::~MimAttack () {
    if (table != NULL) {
        deleteTable (table);
    }
}

/*
static int mpzCompare (const void * a, const void * b) {
    return mpz_cmp (*((mpz_t *)a), *((mpz_t *)b));
}
*/

bool MimAttack::buildTable (gmp_randstate_t rstate) {

    if (bits1 > 25) {
        // too much space!!!!! ACK!!!
        return false;
    }

    table = (MpzTable *) malloc (sizeof(*table));
    table->length = (1l << bits1); // 1 to 2^bits1 = 2^bits1 entries
    table->entries = (MpzTableEntry *) malloc (table->length * sizeof(MpzTableEntry));
    MpzTableEntry *entries = table->entries;

    mpz_t delta1;
    mpz_t tmp;

    mpz_init_set_ui (delta1, 0);
    mpz_init (tmp);

    //printf ("Generating table...\n");
    for (size_t i = 0; i < table->length; i++) {
        mpz_add_ui (delta1, delta1, 1);
        mpz_init (table->entries[i].key);
        mpz_init_set (table->entries[i].value, delta1);

        mpz_powm (table->entries[i].key, delta1, e->baseOrder, e->prime);
    }
    //printf (" done generating table.\n");

    time_t start = time (NULL);
    qsort (entries, table->length, sizeof (*entries), mpzTableEntryCompare);
    double diff = difftime (time (NULL), start);

    printf ("sort time: %dm %ds : %ld\n", (int) floor (diff / 60),
                                          ((int)diff) % 60, (long)diff);
 
    mpz_clear (delta1);
    mpz_clear (tmp);

    return true;
}

size_t MimAttack::crackMessage (MpzList *results, const ElgamalCipherText ct,
                                gmp_randstate_t rstate, size_t maxResults) {

    if (table == NULL) {
        return false;
    }

    size_t resultCount = 0;

    mpz_t delta2, uq, result;
    mpz_init_set_ui (delta2, 0);
    mpz_init (uq);
    mpz_init (result);

    //gmp_printf ("ct.myk = %Zd\n", uq);
    mpz_powm (uq, ct.myk, e->baseOrder, e->prime);
    // TODO: fail if uq = 1?
    if (mpz_cmp_ui (uq, 1) == 0) {
        gmp_printf ("WARN: u^q == 1!\n");
    }

    MpzTableEntry target;
    mpz_init (target.key);

    size_t max = (1l << bits2);

    //printTable (table);

    MpzTableEntry *entry = NULL;
    while (mpz_cmp_ui (delta2, max) < 0) {
        mpz_add_ui (delta2, delta2, 1);
        //gmp_printf ("delta2 = %Zd\n", delta2);
        mpz_powm (target.key, delta2, e->baseOrder, e->prime);
        mpz_invert (target.key, target.key, e->prime);
        mpz_mul (target.key, target.key, uq);
        mpz_mod (target.key, target.key, e->prime);

        //gmp_printf (" ...looking for %Zd\n", target.key);

        //mpzTableBinarySearch (&m, table, delta2);
        entry = (MpzTableEntry *)bsearch (&target, table->entries, table->length,
                                          sizeof(*(table->entries)), mpzTableEntryCompare);
        if (entry != NULL) {
            mpz_mul (result, entry->value, delta2);
            results->append (result);
            resultCount++;
            if (maxResults > 0 && resultCount >= maxResults)
                break;
        }
    }

    mpz_clear (target.key);
    mpz_clear (delta2);
    mpz_clear (uq);
    mpz_clear (result);

    return resultCount;
}
