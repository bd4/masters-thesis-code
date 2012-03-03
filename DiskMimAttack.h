/*
 * =====================================================================================
 *
 *       Filename:  DiskMimAttack.h
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

#include <tcutil.h>
#include <tchdb.h>
#include <tcbdb.h>

typedef uint32_t UIntType;

/*
inline UIntType hash (mpz_t n) {
    return (UIntType) mpz_get_ui (n);
    //return x && ((1 << bits) - 1);
}
*/

class DiskMimAttack : public ElgamalAttack {

    private:
        TCBDB *bdb;
        char *fileName;

    public:
        DiskMimAttack (ElgamalCryptosystem *c, char *fileName, unsigned int bits1, unsigned int bits2);
        ~DiskMimAttack ();
        bool buildTable (gmp_randstate_t rstate);
        size_t crackMessage (MpzList *results, const ElgamalCipherText ct,
                             gmp_randstate_t rstate, size_t maxResults=0);
        const char* getAttackName () const { return "diskmim"; }

};
