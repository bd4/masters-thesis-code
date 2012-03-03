/*
 * =====================================================================================
 *
 *       Filename:  TwoTableAttack.cc
 *
 *    Description:  This is a variation on the meet-in-the-middle attack which uses
 *                  logarithms to reduce the online computation time.
 *
 *        Version:  1.0
 *        Created:  10/06/2008 04:45:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <math.h>
#include <gmp.h>
#include <time.h>

#include "include/types.h"
#include "include/randomhelpers.h"
#include "include/elgamal.h"
#include "include/dlog.h"

#include "MpzList.h"
#include "ElgamalAttack.h"
#include "TwoTableAttack.h"

static inline size_t circularIncrement (size_t i, int inc, size_t n) {
    if (i == 0 && inc < 0) {
        return n + inc; // we assume n > inc
    } else if (i == n-1 && inc > 0) {
        return inc - 1;
    }
    return i + inc;
}

TwoTableAttack::TwoTableAttack (ElgamalCryptosystem *elg, unsigned int b1,
                                unsigned int b2) {
    bits1 = b1;
    bits2 = b2;
    e = elg;

    oneTable = (bits1 == bits2);

    t1.length = (1l << bits1);
    t2.length = (1l << bits2);
}


static void deleteTableEntries (MpzTable table) {
    if (table.entries != NULL) {
        for (size_t i=0; i < table.length; i++) {
            mpz_clear (table.entries[i].key);
            mpz_clear (table.entries[i].value);
        }
        free (table.entries);
    }
}

TwoTableAttack::~TwoTableAttack () {
    deleteTableEntries (t1);   
    if (!oneTable)
        deleteTableEntries (t2);   
}

bool TwoTableAttack::buildTable (gmp_randstate_t rstate) {

    t1.entries = (MpzTableEntry *) malloc (t1.length * sizeof(MpzTableEntry));
    if (oneTable) {
        t2.entries = t1.entries;
        printf ("INFO: bits1 == bits2, using one table for memory savings.\n");
    } else {
        t2.entries = (MpzTableEntry *) malloc (t2.length * sizeof(MpzTableEntry));
    }

    mpz_t z, delta, gamma, key;
    mpz_init (z); mpz_init (delta); mpz_init (gamma); mpz_init (key);

    // compute z = r * baseOrder, so that p-1 = z * s
    mpz_mul (z, e->baseOrder, e->r);

    size_t tMaxLen = (bits1 > bits2) ? t1.length : t2.length;

    DECLARE_PH_LOCALS
    INIT_PH_LOCALS(e->prime)

    MpzTableEntry *entries1 = t1.entries;
    MpzTableEntry *entries2 = t2.entries;
    mpz_set_ui (delta, 0);
    size_t i = 0;
    do {
        mpz_add_ui (delta, delta, 1);
        mpz_powm (gamma, delta, z, e->prime);
        pohlig_hellman (key, e->sGenerator, e->prime, e->s, gamma, rstate, LIST_PH_LOCALS);
        if (i < t1.length) {
            mpz_init_set (entries1[i].value, delta);
            mpz_init_set (entries1[i].key, key);
            /*
            if (mpz_size (entries1[i].value) > 3)
                printf ("value mpz_size > 3: %zu\n", mpz_size (entries1[i].value));
            if (mpz_size (entries1[i].key) > 3)
                printf ("key mpz_size > 3: %zu\n", mpz_size (entries1[i].key));
                */
        }
        if (!oneTable && i < t2.length) {
            mpz_init_set (entries2[i].value, delta);
            mpz_init_set (entries2[i].key, key);
        }
        i++;
    } while (i < tMaxLen);

/*
    // wait for enter, so we can check pre-sort memory usage
    printf ("Done generating table, about to sort. Hit ENTER to continue\n");
    char *input = NULL;
    size_t n;
    getline (&input, &n, stdin);
    if (input != NULL)
        free (input);
        */

    time_t start = time (NULL);
   
    // sort ASC
    qsort (entries1, t1.length, sizeof (*entries1), mpzTableEntryCompare);
    // sort DESC
    if (!oneTable) {
        qsort (entries2, t2.length, sizeof (*entries2), mpzTableEntryReverseCompare);
    }

    double diff = difftime (time (NULL), start);

    printf ("sort time: %dm %ds : %ld\n", (int) floor (diff / 60),
                                          ((int)diff) % 60, (long)diff);
 
//#define LIST_PH_LOCALS xi, q, qj, lj, ndivq, alphaBar, betaBar, betaStripped, alphaStrip, x, a, b, x1, a1, b1

/*
    printf ("xi: %zu\n", mpz_size (xi));
    printf ("q: %zu\n", mpz_size (q));
    printf ("qj: %zu\n", mpz_size (qj));
    printf ("lj: %zu\n", mpz_size (lj));
    printf ("ndivq: %zu\n", mpz_size (ndivq));
    printf ("alphaBar: %zu\n", mpz_size (alphaBar));
    printf ("betaBar: %zu\n", mpz_size (betaBar));
    printf ("betaStripped: %zu\n", mpz_size (betaStripped));
    printf ("alphaStrip: %zu\n", mpz_size (alphaStrip));
    printf ("x: %zu\n", mpz_size (x));
    printf ("a: %zu\n", mpz_size (a));
    printf ("b: %zu\n", mpz_size (b));
    printf ("x1: %zu\n", mpz_size (x1));
    printf ("a1: %zu\n", mpz_size (a1));
    printf ("b1: %zu\n", mpz_size (b1));
    */
/*
    entries = t2.entries;
    mpz_set_ui (delta, 0);
    i = 0;
    do {
        mpz_add_ui (delta, delta, 1);
        mpz_powm (gamma, delta, z, e->prime);
        mpz_init_set (entries[i].value, delta);
        mpz_init (entries[i].key);
        pohlig_hellman (entries[i].key, e->sGenerator, e->prime, e->s, gamma, rstate, LIST_PH_LOCALS);
        i++;
    } while (i < t2.length);
    // sort desc

*/
    /*
    printf ("t2, ASC order\n");
    for (size_t i=0; i < t2.length; i++) {
        gmp_printf ("%Zd -> %Zd\n", t2.entries[i].key, t2.entries[i].value);
    }
    */

    mpz_clear (z); mpz_clear (gamma); mpz_clear (delta); mpz_clear (key);
    CLEAR_PH_LOCALS

    return true;
}

/*
 * Find the index where value is located, or if value is not in the table, find
 * the index of the next largest (left) element;
 *
 * table should be sorted in descending order.
 */
static bool mpzTableBinaryIndexReverseSearch (size_t *index, MpzTable table, mpz_t value) {
    size_t i, m, M;
    m = 0;
    M = table.length - 1;
    MpzTableEntry *e = table.entries;
    int cmp;
    while (m <= M) {
        i = (M+m)/2; // rounded down
        cmp = mpz_cmp (value, e[i].key);
        if (cmp == 0) {
            *index = i;
            return true;
        }
        if (cmp > 0) {
            if (i == 0) {
                *index = 0;
                return false;
            }
            M = i-1; i = (M+m)/2; // rounded down
        } else {
            m = i+1;
            i = (M+m)/2; // rounded down
        }
    }
    // value is not in the table. Since we round indexes down, i will be set correctly
    // on the last call to the while loop (when m == M).
    *index = i;
    return false;
}

/*
 * Find the index where value is located, or if value is not in the table, find
 * the index of the next largest (right) element;
 *
 * table should be sorted in increasing order.
 */
static bool mpzTableBinaryIndexSearch (size_t *index, MpzTable table, mpz_t value) {
    size_t i, m, M;
    m = 0;
    M = table.length - 1;
    MpzTableEntry *e = table.entries;
    int cmp;
    while (m <= M) {
        i = ceil ((M+m)/2.0); // round up
        cmp = mpz_cmp (value, e[i].key);
        if (cmp == 0) {
            *index = i;
            return true;
        }
        if (cmp < 0) {
            if (i == 0) {
                *index = 0;
                return false;
            }
            M = i-1; i = ceil ((M+m)/2.0); // round up
        } else {
            m = i+1;
            i = ceil ((M+m)/2.0); // round up
        }
    }
    // value is not in the table. Since we round indexes down, i will be set correctly
    // on the last call to the while loop (when m == M).
    *index = i;
    return false;
}

size_t TwoTableAttack::crackMessage (MpzList *results, const ElgamalCipherText ct,
                                     gmp_randstate_t rstate, size_t maxResults) {

    mpz_t z, n, delta, gamma;
    mpz_init (z); mpz_init (n); mpz_init (delta); mpz_init (gamma);

    size_t resultCount = 0;

    DECLARE_PH_LOCALS
    INIT_PH_LOCALS(e->prime)

    // compute z = r * baseOrder, so that p-1 = z * s
    mpz_mul (z, e->baseOrder, e->r);

    // compute target n = log (myk^z) [base sGenerator]
    //printf ("Computing target...\n");
    mpz_powm (gamma, ct.myk, z, e->prime);
    pohlig_hellman (n, e->sGenerator, e->prime, e->s, gamma, rstate, LIST_PH_LOCALS);
    //gmp_printf ("target = %Zd\n", n);

    // search for n+1 in t2, since the elements surrounding n+1 will be the end and start
    // of the circular list containing the elements n-t2[i]
    //printf ("Finding pivot...\n");
    mpz_add_ui (gamma, n, 1);
    size_t pivot, i1, i2start, i2;
    int inc2 = 1;
    if (oneTable) {
        // t2 = t1 and it's sorted ASC
        mpzTableBinaryIndexSearch (&pivot, t2, gamma);
        i2start = (pivot - 1) % t2.length;
        inc2 = -1;
    } else {
        // t2 is sorted DESC
        mpzTableBinaryIndexReverseSearch (&pivot, t2, gamma);
        i2start = (pivot + 1) % t2.length;
    }
    i1 = 0;
    i2 = i2start;

    /*
    printf ("pivot = %zu\n", pivot);
    gmp_printf ("biggest  = n - %Zd\n", t2.entries[pivot].key);
    gmp_printf ("smallest = n - %Zd\n", t2.entries[(pivot+1)%t2.length].key);
    */

    //printf ("Searching tables...\n");
    // We seek elements a in t1 and b in t2 so that n = a + b mod s. This
    // is equivalent to having c = n - b mod s such that a = c.
    int cmp;

    // gamma is used to store the current entry of the virtual table (n - t1)
    mpz_sub (gamma, n, t2.entries[i2].key);
    if (mpz_sgn (gamma) < 0) { // compute mod s
        mpz_add (gamma, gamma, e->s->value);
    }
    //gmp_printf ("n - b (gamma) = %Zd\n", gamma);
    do {
        // We want equality. If not, move to the next
        // biggest element in the table of the smaller
        // of the elements.
        //gmp_printf ("t1[%zu]     = %Zd\n", i1, t1.entries[i1].key);
        //gmp_printf ("t2[%zu] = %Zd\n", i2, t2.entries[i2].key);
        //gmp_printf ("n - t2[%zu] = %Zd\n", i2, gamma);
        cmp = mpz_cmp (t1.entries[i1].key, gamma);
        if (cmp < 0) {
            i1++;
        } else if (cmp > 0) {
            i2 = circularIncrement (i2, inc2, t2.length);
            if (i2 == i2start)
                break;
            mpz_sub (gamma, n, t2.entries[i2].key);
            if (mpz_sgn (gamma) < 0) { // compute mod s
                mpz_add (gamma, gamma, e->s->value);
            }
            //gmp_printf ("n - b (gamma) = %Zd\n", gamma);
        } else {
            mpz_mul (z, t1.entries[i1].value, t2.entries[i2].value);
            results->append (z);
            resultCount++;
            if (maxResults > 0 && resultCount >= maxResults)
                break;
            i1++;
            i2 = circularIncrement (i2, inc2, t2.length);
            if (i2 == i2start)
                break;
        }
    } while (i1 < t1.length);

    CLEAR_PH_LOCALS
    mpz_clear (z); mpz_clear (n); mpz_clear (delta); mpz_clear (gamma);

    return resultCount;

}

