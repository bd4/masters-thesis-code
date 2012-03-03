/*
 * =====================================================================================
 *
 *       Filename:  randomhelpers.h
 *
 *    Description:  Helper functions to generate random numbers.
 *
 *        Version:  1.0
 *        Created:  07/09/2008 04:26:04 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Bryce Allen
 *
 * =====================================================================================
 */

#ifndef _randomhelpers_h
#define _randomhelpers_h
bool randomULong (unsigned long *out);
bool seedRandState (gmp_randstate_t state);
int randomNonIncreasingPrimeSequence (mpz_t *seq, const mpz_t max, gmp_randstate_t rstate);
//FactoredInteger* randomFactoredInteger (mpz_t max, gmp_randstate_t rstate);
//FactoredInteger* randomFactoredInteger (int maxBits, gmp_randstate_t rstate);
#endif
