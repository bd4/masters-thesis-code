/*
 * Modification to HashMimAttack which stores modular exponentaitions computed during
 * buildTable, and reads them during crackMessage.
 */
#include "include/types.h"

class HashMimAttack2 : public ElgamalAttack {

    private:
        UIntTable table;
        char *cacheFilePath;

    public:
        HashMimAttack2 (ElgamalCryptosystem *c, unsigned int bits1, unsigned int bits2, const char *cacheFile);
        ~HashMimAttack2 ();
        bool buildTable (gmp_randstate_t rstate);
        size_t crackMessage (MpzList *results, ElgamalCipherText ct, gmp_randstate_t rstate, size_t maxResults=0);
        const char* getAttackName () const { return "hashmim2"; }
        
};
