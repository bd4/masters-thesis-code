/*
 * Modification to HashMimAttack which stores modular exponentaitions computed during
 * buildTable, and reads them during crackMessage.
 */
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"

#include "MpzList.h"
#include "ElgamalAttack.h"
#include "HashMimAttack2.h"

HashMimAttack2::HashMimAttack2 (ElgamalCryptosystem *elg, unsigned int b1, unsigned int b2,
                                const char *cacheFile) {
    bits1 = b1;
    bits2 = b2;
    e = elg;
    cacheFilePath = (char *) malloc ((strlen (cacheFile) + 1) * sizeof (*cacheFilePath));
    strcpy (cacheFilePath, cacheFile);

}

HashMimAttack2::~HashMimAttack2 () {
    if (table.entries != NULL) {
        free (table.entries);
    }
    if (cacheFilePath != NULL) {
        free (cacheFilePath);
    }
}


static int uintTableEntryCompare (const void *a, const void *b) {
    unsigned long au = ((UIntTableEntry *)a)->key;
    unsigned long bu = ((UIntTableEntry *)b)->key;
    if (au < bu)
        return -1;
    if (au > bu)
        return 1;
    return 0;
}

static inline UIntType hash (mpz_t n) {
    return (UIntType) mpz_get_ui (n);
    //return x && ((1l << bits) - 1);
}

/*
 * Build a table of pairs (key, value) sorted on key, where
 * key = hash (delta1^q mod p) and value = delta1.
 */
bool HashMimAttack2::buildTable (gmp_randstate_t rstate) {

    if (bits1 > sizeof (UIntType)) {
        return false;
    }

    FILE *cache = fopen (cacheFilePath, "w");
    if (cache == NULL) {
        perror ("Unable to open cache file for HashMimAttack2");
        return false;
    }

    table.length = (1l << bits1); // table will contain range 1 to 2^bits1 as values
    if (bits1 == sizeof (UIntType)) {
        // Avoid overflow of the last element. This very slightly reduces the search space
        // and success propability.
        table.length--;
    }

    table.entries = (UIntTableEntry *) malloc (table.length * sizeof(UIntTableEntry));
    if (table.entries == NULL) {
        return false;
    }
    UIntTableEntry *entries = table.entries;

    mpz_t delta1;
    mpz_t tmp;

    mpz_init_set_ui (delta1, 0);
    mpz_init (tmp);

    printf ("Generating table...\n");
    // as i goes from 0 to 2^bits1 - 1, delta1 goes from 1 to 2^bits1
    for (size_t i = 0; i < table.length; i++) {
        mpz_add_ui (delta1, delta1, 1);

        table.entries[i].value = (UIntType) mpz_get_ui (delta1);

        mpz_powm (tmp, delta1, e->baseOrder, e->prime);

        if (!mpz_out_raw (cache, tmp)) {
            fprintf (stderr, "Write failed at %zu\n", i);
        }

        table.entries[i].key = hash (tmp);
    }
    printf (" done generating table.\n");

    time_t start = time (NULL);
    qsort (entries, table.length, sizeof (*entries), uintTableEntryCompare);
    double diff = difftime (time (NULL), start);

    printf ("sort time: %dm %ds : %ld\n", (int) floor (diff / 60),
                                          ((int)diff) % 60, (long)diff);
 
    mpz_clear (tmp);
    mpz_clear (delta1);

    if (fclose (cache) != 0) {
        perror ("Unable to close cache file for HahsMimAttack2");
        return false;
    }

    /*
    for (size_t i=0; i < table.length; i++) {
        gmp_printf ("%lo -> %lo\n", table.entries[i].key, table.entries[i].value);
    }
    */

    //qSortUIntTable (entries, 0, table.length - 1);
    return true;

}

/*
 * The version in stdlib does not give us the index of the element found,
 * which we need to search for contiguous entries with the same key.
 */
static bool uintTableBinarySearch (size_t *index, UIntTable *table, unsigned long value) {
    size_t i, m, M;
    m = 0;
    M = table->length - 1;
    UIntTableEntry *e = table->entries;
    while (m <= M) {
        i = (M+m)/2; // round down
        if (value == e[i].key) {
            *index = i;
            return true;
        }
        if (value < e[i].key) {
            if (i == 0)
                return false;
            M = i-1; i = (M+m)/2; // round down
        } else {
            m = i+1;
            i = (M+m)/2; // round down
        }
    }
    return false;
}

/*
 * Note: This assumes that the message decomposition is unique. In reality,
 * we could sanity check the result and keep checking if necessary,
 * or just find all matches.
 */
size_t HashMimAttack2::crackMessage (MpzList *results, const ElgamalCipherText ct,
                                    gmp_randstate_t rstate, size_t maxResults) {

    FILE *cache = fopen (cacheFilePath, "r");
    if (cache == NULL) {
        perror ("Unable to open cache file for HashMimAttack2");
        return 0;
    }

    size_t resultCount = 0;

    mpz_t delta2, uq, target, candidate, delta;
    mpz_init_set_ui (delta2, 0);
    mpz_init_set (uq, ct.myk);

    //gmp_printf ("ct.myk = %Zd\n", uq);
    mpz_powm (uq, uq, e->baseOrder, e->prime);
    // TODO: fail if uq = 1?
    gmp_printf ("u^q = %Zd\n", uq);

    mpz_init (target);
    mpz_init (candidate);
    mpz_init (delta);
    size_t max = (1l << bits2);
    size_t maxTable = (1l << bits1);

    size_t matchCount = 0;
    unsigned long targetHash, candidateHash;
    size_t startIndex, currentIndex;
    bool found;
    while (mpz_cmp_ui (delta2, max) < 0) {
        mpz_add_ui (delta2, delta2, 1);
        //gmp_printf ("delta2 = %Zd\n", delta2);
        if (mpz_cmp_ui (delta2, maxTable) <= 0) {
            if (!mpz_inp_raw (target, cache)) {
                gmp_fprintf (stderr, "Unable to read from cache at %Zd\n", delta2);
                mpz_powm (target, delta2, e->baseOrder, e->prime);
            }
        } else {
            mpz_powm (target, delta2, e->baseOrder, e->prime);
        }
        mpz_invert (target, target, e->prime);
        mpz_mul (target, target, uq);
        mpz_mod (target, target, e->prime);
        targetHash = hash (target);

        //gmp_printf (" ...looking for %Zd\n", target.key);

        // If an entry is found, it's only a candidate, since we are using hashes.
        // Check to see if it really matches, and check neighbors if that fails.
        if (uintTableBinarySearch (&startIndex, &table, targetHash)) {
            currentIndex = startIndex;
            int increment = -1; // search left first
            found = false;
            while (1) {
                candidateHash = table.entries[currentIndex].key;
                if (candidateHash != targetHash) {
                    // If we've been search lefting, start searching right.
                    if (currentIndex < startIndex) {
                        currentIndex = startIndex + 1;
                        increment = 1;
                        if (currentIndex >= table.length)
                            break;
                    } else { // We already searched left and right, give up.
                        break;
                    }
                } else {
                    matchCount++;
                    mpz_set_ui (candidate, table.entries[currentIndex].value);
                    mpz_powm (candidate, candidate, e->baseOrder, e->prime);
                    if (mpz_cmp (target, candidate) == 0) {
                        found = true;
                        break;
                    }
                    if (currentIndex == 0 && increment < 0)
                        break;
                    currentIndex += increment;
                    if (currentIndex >= table.length)
                        break;
                }
            }

            if (found) {
                mpz_mul_ui (delta, delta2, table.entries[currentIndex].value);
                //gmp_printf ("DEBUG: results[%zu] = %u * %Zd\n", resultCount,
                //            table.entries[currentIndex].value, delta2);
                results->append (delta);
                resultCount++;
                if (maxResults > 0 && resultCount >= maxResults)
                    break;
            }
        }
    }

    /*
    if (found)
        mpz_mul (m, delta1, delta2);
    else
        mpz_set_ui (m, 0);
        */

    // TODO: store/load from disc, since this depends only on the cryptosystem
    //hashMim1DeleteTable (table);
    //free (table);

    mpz_clear (target);
    mpz_clear (candidate);
    mpz_clear (delta);
    mpz_clear (delta2);
    mpz_clear (uq);

    printf ("hashMimAttack match count: %zu\n", matchCount);

    if (fclose (cache) != 0) {
        perror ("Unable to close cache file for HahsMimAttack2");
    }

    return resultCount;

}
