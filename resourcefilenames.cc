/*
 * =====================================================================================
 *
 *       Filename:  resourcefilenames.cc
 *
 *    Description:  Get filenames for resources.
 *
 *        Version:  1.0
 *        Created:  10/05/2008 06:48:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#include <gmp.h>
#include <stdio.h>
#include <string.h>

#include "include/types.h"
#include "include/elgamal.h"
#include "resourcefilenames.h"

static size_t cryptosystemFileNameLength (char *label, ElgamalCryptosystem *e) {
    // label_XXXX_XXXX.elg
    return strlen(label) + 5 + 5 + 4 + 1;
}

// copies cryptosystemFileNameLength chars to target, or cryptosystemFileNameLength - 4 if omitExt == true
static void strcpyCryptosystemFileName (char *target, char *label, ElgamalCryptosystem *e, bool omitExt = false) {
    //size_t len = cryptosystemFileNameLength (label, e);
    char *p = target;
    strcpy (p, label);
    p += strlen (label);
    *p = '_'; p++;
    snprintf (p, 5, "%04zu", mpz_sizeinbase (e->prime, 2));
    p += 4; // overwrite null byte
    *p = '_'; p++;
    snprintf (p, 5, "%04zu", mpz_sizeinbase (e->baseOrder, 2));
    if (!omitExt) {
        p += 4; // overwrite null byte
        strcpy (p, ".elg"); // includes the null byte
    }
}

char *getCryptosystemFileName (char *label, ElgamalCryptosystem *e) {
    // label_XXXX_XXXX.elg
    size_t len = cryptosystemFileNameLength (label, e);
    char *fileName = (char *) malloc (len * sizeof (*fileName));

    strcpyCryptosystemFileName (fileName, label, e);

    return fileName;
}

char *getCryptosystemFilePath (char *label, ElgamalCryptosystem *e) {

    size_t len = cryptosystemFileNameLength (label, e) + strlen(CRYPTOSYSTEM_DIR) + 1;

    char *filePath = (char *) malloc (len * sizeof (*filePath));

    strncpy (filePath, CRYPTOSYSTEM_DIR, strlen (CRYPTOSYSTEM_DIR)); // don't copy null byte
    strcpyCryptosystemFileName (filePath + strlen(CRYPTOSYSTEM_DIR), label, e);

    return filePath;

}

char *getCryptosystemDirPath (char *label, ElgamalCryptosystem *e) {

    size_t len = cryptosystemFileNameLength (label, e) + strlen(CRYPTOSYSTEM_DIR) + 1 - 4;

    char *dirPath = (char *) malloc (len * sizeof (*dirPath));

    strncpy (dirPath, CRYPTOSYSTEM_DIR, strlen (CRYPTOSYSTEM_DIR)); // don't copy null byte
    strcpyCryptosystemFileName (dirPath + strlen(CRYPTOSYSTEM_DIR), label, e, true);
    //*(dirPath + len - 1) = '\0';

    //free (fileName);

    return dirPath;

}

// TODO: finish
char *getCipherTextFilePath (char *label, ElgamalCryptosystem *e, ElgamalCipherText ct) {
    char *path = getCryptosystemDirPath (label, e);

    size_t len = strlen (path) + 10;

    path = (char *) realloc (path, len * sizeof (*path));

    return path;
}

//char *getAttackTableFilePath (ElgamalCryptosystem *e, char *attackName, int bits1);
//char *getAttackResultsFilePath (ElgamalCryptosystem *e, char *attackName, int bits1, ElgamalCipherText ct);
