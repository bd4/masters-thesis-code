/*
 * =====================================================================================
 *
 *       Filename:  mpzSizeTest.cc
 *
 *    Description:  Simple program to determine the total size of
 *                  an mpz_t variable and its allocated memory.
 *
 *        Version:  1.0
 *        Created:  11/05/2008 11:31:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>

#define SIZE 1024

int main (int argc, char **argv) {
    
    mpz_t a;
    mpz_init_set_ui (a, 1);

    mpz_t *list = (mpz_t *) malloc (sizeof(*list) * SIZE);

    mpz_mul_2exp (a, a, 71);

    printf ("sizeof(mpz_t) = %zu\n", sizeof(mpz_t));
    printf ("sizeof(__mpz_struct) = %zu\n", sizeof(__mpz_struct));
    printf ("sizeof(mpz_t[0]._mp_alloc) = %zu\n", sizeof(a[0]._mp_alloc));
    printf ("sizeof(mpz_t[0]._mp_size) = %zu\n", sizeof(a[0]._mp_size));
    printf ("sizeof(mpz_t[0]._mp_d) = %zu\n", sizeof(a[0]._mp_d));
    printf ("sizeof(mp_limb_t) = %zu\n", sizeof(mp_limb_t));
    printf ("sizeof(mp_limb_signed_t) = %zu\n", sizeof(mp_limb_signed_t));

    printf ("mpz_size(2^71) = %zu\n", mpz_size (a));

    for (int i = 0; i < SIZE; i++) {
        //mpz_init (list[i]);
        mpz_init_set (list[i], a);
        //mpz_set (list[i], a);
    }

    printf ("allocated %d mpz_t, set each to 2^71. Hit ENTER to continue\n", SIZE);
    char *input = NULL;
    size_t n;
    getline (&input, &n, stdin); // wait for enter

    for (int i = 0; i < SIZE; i++) {
        mpz_clear (list[i]);
    }
    free (list);
    if (input != NULL)
        free (input);
}
