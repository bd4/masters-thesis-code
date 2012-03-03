/*
 * =====================================================================================
 *
 *       Filename:  2TableAttack.h
 *
 *    Description:  2 Table Attack
 *
 *        Version:  1.0
 *        Created:  10/06/2008 04:44:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */


class TwoTableAttack : public ElgamalAttack {
    private:
        MpzTable t1;
        MpzTable t2;
        bool oneTable;

    public:
        TwoTableAttack (ElgamalCryptosystem *e, unsigned int bits1, unsigned int bits2);
        ~TwoTableAttack ();
        bool buildTable (gmp_randstate_t rstate);
        size_t crackMessage (MpzList *results, const ElgamalCipherText ct,
                             gmp_randstate_t rstate, size_t maxResults=0);
        const char* getAttackName () const { return "2table"; }
};
