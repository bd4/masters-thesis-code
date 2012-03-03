/*
 * Modification to HashMimAttack2 which uses a hash table instead
 * of a sorted array. Offers only very slight performance increase
 * at the expense of a 50% table size increase. Not worth it.
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
#include "HashMimAttack3.h"

HashMimAttack3::HashMimAttack3 (ElgamalCryptosystem *elg, unsigned int b1, unsigned int b2,
                                const char *cacheFile) {
    bits1 = b1;
    bits2 = b2;
    e = elg;
    cacheFilePath = (char *) malloc ((strlen (cacheFile) + 1) * sizeof (*cacheFilePath));
    strcpy (cacheFilePath, cacheFile);

}

HashMimAttack3::~HashMimAttack3 () {
    if (table.entries != NULL) {
        free (table.entries);
    }
    if (cacheFilePath != NULL) {
        free (cacheFilePath);
    }
}

static inline UIntType hash (mpz_t n) {
    return (UIntType) mpz_get_ui (n);
    //return x && ((1l << bits) - 1);
}

/*
 * Build a table of pairs (key, value) sorted on key, where
 * key = hash (delta1^q mod p) and value = delta1.
 */
bool HashMimAttack3::buildTable (gmp_randstate_t rstate) {

    FILE *cache = fopen (cacheFilePath, "w");
    if (cache == NULL) {
        perror ("Unable to open cache file for HashMimAttack3");
        return false;
    }

    table.length = (1l << bits1) + 1; // table will contain range 1 to 2^bits1 as values,
                                      // but the zero position is not used.
    size_t indexMask = table.length - 2;
    table.indexMask = indexMask;
    table.fullFrom = table.length;
    table.entries = (UIntHashTableEntry *) calloc (table.length, sizeof(UIntHashTableEntry));
    if (table.entries == NULL) {
        return false;
    }
    UIntHashTableEntry *entries = table.entries;

    mpz_t delta1;
    mpz_t tmp;

    mpz_init_set_ui (delta1, 0);
    mpz_init (tmp);

    UIntType keyHash;
    UIntType index;
    // as i goes from 0 to 2^bits1 - 1, delta1 goes from 1 to 2^bits1
    for (size_t i = 0; i < table.length; i++) {
        mpz_add_ui (delta1, delta1, 1);

        mpz_powm (tmp, delta1, e->baseOrder, e->prime);
        keyHash = hash (tmp); // range 0 to 2^sizeof(UIntType)-1
        index = (keyHash & indexMask) + 1; // range 1 to 2^bits1
                                           //     = 1 to table.length-1

        if (entries[index].key == 0) {
            entries[index].key = keyHash;
            entries[index].value = (UIntType) mpz_get_ui (delta1);
        } else {
            // follow link chain until we find a free link slot
            while (entries[index].link != 0) {
                index = entries[index].link;   
            }
            // Now index points to a reachable node with a free link.
            // Find a free key/value slot.
            do {
                if (table.fullFrom == 0) {
                    fprintf (stderr, "HashMimAttack3: table overflow\n");
                    mpz_clear (tmp);
                    mpz_clear (delta1);
                    return false;
                }
                table.fullFrom--;
            } while (entries[table.fullFrom].key != 0);
            entries[index].link = table.fullFrom;
            entries[table.fullFrom].key = keyHash;
            entries[table.fullFrom].value = (UIntType) mpz_get_ui (delta1);
        }

        if (!mpz_out_raw (cache, tmp)) {
            fprintf (stderr, "Write failed at %zu\n", i);
        }

        //table.entries[i].key = hash (tmp);
    }

    mpz_clear (tmp);
    mpz_clear (delta1);

    if (fclose (cache) != 0) {
        perror ("Unable to close cache file for HahsMimAttack3");
        return false;
    }

    /*
    for (size_t i=0; i < table.length; i++) {
        gmp_printf ("%lo -> %lo\n", table.entries[i].key, table.entries[i].value);
    }
    */

    //qSortUIntHashTable (entries, 0, table.length - 1);
    return true;

}


/*
 * Note: This assumes that the message decomposition is unique. In reality,
 * we could sanity check the result and keep checking if necessary,
 * or just find all matches.
 */
size_t HashMimAttack3::crackMessage (MpzList *results, const ElgamalCipherText ct,
                                    gmp_randstate_t rstate, size_t maxResults) {

    FILE *cache = fopen (cacheFilePath, "r");
    if (cache == NULL) {
        perror ("Unable to open cache file for HashMimAttack3");
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
    unsigned long targetHash;
    bool found;
    UIntType index;
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
        targetHash = hash (target); // range 0 to 2^sizeof(UIntType)-1

        // If an entry is found, it's only a candidate, since we are using hashes.
        // Check to see if it really matches, and check neighbors if that fails.
        index = (targetHash & table.indexMask) + 1; // range 1 to 2^bits1
                                                    //     = 1 to table.length-1
        found = false;
        while (1) {
            if (table.entries[index].key == targetHash) {
                // is this a real match?
                matchCount++;
                mpz_set_ui (candidate, table.entries[index].value);
                mpz_powm (candidate, candidate, e->baseOrder, e->prime);
                if (mpz_cmp (target, candidate) == 0) {
                    found = true;
                    break;
                }
            }
            if (table.entries[index].link == 0) {
                break;
            } else {
                index = table.entries[index].link;
            }
        }

        if (found) {
            mpz_mul_ui (delta, delta2, table.entries[index].value);
            //gmp_printf ("DEBUG: results[%zu] = %u * %Zd\n", resultCount,
            //            table.entries[currentIndex].value, delta2);
            results->append (delta);
            resultCount++;
            if (maxResults > 0 && resultCount >= maxResults)
                break;
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
        perror ("Unable to close cache file for HahsMimAttack3");
    }

    return resultCount;

}
