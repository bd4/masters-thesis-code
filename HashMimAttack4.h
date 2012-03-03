/*
 * Modification to HashMimAttack3 which reduces the size of the hashtable.
 */
#include "include/types.h"

typedef struct {
    UIntType value;
    UIntType link;
} UIntShortHashTableEntry;

typedef struct {
    size_t length;
    size_t indexMask;
    size_t fullFrom;
    UIntShortHashTableEntry *entries;
} UIntShortHashTable;

class HashMimAttack4 : public ElgamalAttack {

    private:
        UIntShortHashTable table;
        char *cacheFilePath;

    public:
        HashMimAttack4 (ElgamalCryptosystem *c, unsigned int bits1, unsigned int bits2, const char *cacheFile);
        ~HashMimAttack4 ();
        bool buildTable (gmp_randstate_t rstate);
        size_t crackMessage (MpzList *results, ElgamalCipherText ct, gmp_randstate_t rstate, size_t maxResults=0);
        const char* getAttackName () const { return "hashmim3"; }
        
};
