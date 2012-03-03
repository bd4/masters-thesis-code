/*
 * =====================================================================================
 *
 *       Filename:  MpzList.cc
 *
 *    Description:  Implementation of class for dynamic list of mpz_t.
 *
 *        Version:  1.0
 *        Created:  11/02/2008 10:40:00 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <gmp.h>
#include <stdlib.h>
#include "MpzList.h"

//#define MPZLIST_INITIAL_SIZE 5
//#define MPZLIST_SIZE_INCREMENT 5

MpzList::MpzList (size_t initialSize, size_t sizeInc) {

    mallocSize = initialSize;
    mallocSizeIncrement = sizeInc;
    initCount = 0;
    size = 0;

    list = (mpz_t *) malloc (mallocSize * sizeof (*list));
    
}

MpzList::~MpzList () {
    for (size_t i = 0; i < initCount; i++) {
        mpz_clear (list[i]);
    }

    free (list);
}

size_t MpzList::append (mpz_t value) {
    // assert size <= mallocSize
    if (size >= mallocSize) {
        mallocSize += mallocSizeIncrement;
        list = (mpz_t *) realloc (list, mallocSize * sizeof (*list));
    }
    if (size >= initCount) {
        initCount++;
        mpz_init_set (list[size], value);
    } else {
        mpz_set (list[size], value);
    }
    return ++size;
}

void MpzList::clear () {
    size = 0;
}

size_t MpzList::compactify () {
    for (size_t i = size; i < initCount; i++) {
        mpz_clear (list[i]);
    }
    initCount = size;

    if (mallocSize > size) {
        mallocSize = size;
        list = (mpz_t *) realloc (list, mallocSize * sizeof (*list));
    }
    return size;
}

size_t MpzList::getSize () {
    return size;
}

size_t MpzList::getInitCount () {
    return initCount;
}

mpz_t &MpzList::operator [] (const size_t i) {
    return (list[i]);
}

bool MpzList::find (size_t *index, mpz_t value) {
    for (size_t i=0; i < size; i++) {
        if (mpz_cmp (list[i], value) == 0) {
            if (index != NULL)
                *index = i;
            return true;
        }
    }
    return false;
}
