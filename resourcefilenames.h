/*
 * =====================================================================================
 *
 *       Filename:  resourcefilenames.h
 *
 *    Description:  Get filenames for resources.
 *
 *        Version:  1.0
 *        Created:  10/11/2008 05:47:16 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

#define CRYPTOSYSTEM_DIR "cryptosystems/"
#define CRYPTOSYSTEM_EXT ".elg"
#define MESSAGE_EXT ".msg"
#define TABLE_EXT ".tbl"

char *getCryptosystemFileName (char *label, ElgamalCryptosystem *e);
char *getCryptosystemFilePath (char *label, ElgamalCryptosystem *e);
char *getCryptosystemDirPath (char *label, ElgamalCryptosystem *e);

char *getCipherTextFilePath (char *label, ElgamalCryptosystem *e, ElgamalCipherText ct);
char *getAttackTableFilePath (ElgamalCryptosystem *e, char *attackName, int bits1);
char *getAttackResultsFilePath (ElgamalCryptosystem *e, char *attackName, int bits1, ElgamalCipherText ct);
