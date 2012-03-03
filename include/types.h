/*
 * =====================================================================================
 *
 *       Filename:  types.h
 *
 *    Description:  Include file for types used in many different places.
 *
 *        Version:  1.0
 *        Created:  09/27/2008 05:13:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef _types_h
#define _types_h
typedef struct {
    mpz_t prime;
    unsigned int power;
    mpz_t value;
} PrimePower;

// need to include stdint.h?
typedef uint32_t UIntType;

typedef struct {
    UIntType key;
    UIntType value;
} UIntTableEntry;

typedef struct {
    size_t length;
    UIntTableEntry *entries;
} UIntTable;

/*
typedef struct {
    PrimePower *factors;
    unsigned int nFactors;
    mpz_t value;
} FactoredInteger;
*/

#include "CFactoredInteger.h"
#endif
