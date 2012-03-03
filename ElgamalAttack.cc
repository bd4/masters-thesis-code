/*
 * =====================================================================================
 *
 *       Filename:  ElgamalAttack.cc
 *
 *    Description:  Implementation of abstract base class for attacks.
 *
 *        Version:  1.0
 *        Created:  10/05/2008 10:35:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <gmp.h>
#include "include/types.h"
#include "include/elgamal.h"
#include "MpzList.h"
#include "ElgamalAttack.h"

int mpzTableEntryCompare (const void *a, const void *b) {
    return mpz_cmp (((MpzTableEntry *)a)->key, ((MpzTableEntry *)b)->key);
}

int mpzTableEntryReverseCompare (const void *a, const void *b) {
    return mpz_cmp (((MpzTableEntry *)b)->key, ((MpzTableEntry *)a)->key);
}

bool ElgamalAttack::crackMessage (mpz_t result, ElgamalCipherText ct, gmp_randstate_t rstate) {
    MpzList results (1);
    return (crackMessage (&results, ct, rstate, 1) > 0);
}

// TODO: make abstract
size_t ElgamalAttack::crackMessage (MpzList *results, const ElgamalCipherText ct,
                                    gmp_randstate_t rstate, size_t maxResults) {
    return 0;
}

/*
char * ElgamalAttack::tableFileName () {

    // TABLEDIR + cryptosystem + '_{bits}', allowing 3 digits for the bits
    char *fileName = (char *) malloc (strlen(TABLEDIR) + strlen(label) + 1 + 4);
    strncpy (fileName, TABLEDIR, strlen (TABLEDIR));
    strncpy (fileName + strlen(TABLEDIR), label, strlen(label));
    *(fileName + strlen(TABLEDIR) + strlen(label)) = '_';
    snprintf (fileName + strlen(TABLEDIR) + strlen(label) + 1, 4, "%d", bits1);

}
*/
