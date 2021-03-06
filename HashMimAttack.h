/*
 * =====================================================================================
 *
 *       Filename:  HashMimAttack.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/19/2008 08:40:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */
#include "include/types.h"

class HashMimAttack : public ElgamalAttack {

    private:
        UIntTable table;

    public:
        HashMimAttack (ElgamalCryptosystem *c, unsigned int bits1, unsigned int bits2);
        ~HashMimAttack ();
        bool buildTable (gmp_randstate_t rstate);
        size_t crackMessage (MpzList *results, ElgamalCipherText ct, gmp_randstate_t rstate, size_t maxResults=0);
        const char* getAttackName () const { return "hashmim"; }
        
};
