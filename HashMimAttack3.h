/*
 * Modification to HashMimAttack2 which replaces the sorted array with
 * a hash table.
 */
#include "include/types.h"

typedef struct {
    UIntType key;
    UIntType value;
    UIntType link;
} UIntHashTableEntry;

typedef struct {
    size_t length;
    size_t indexMask;
    size_t fullFrom;
    UIntHashTableEntry *entries;
} UIntHashTable;

class HashMimAttack3 : public ElgamalAttack {

    private:
        UIntHashTable table;
        char *cacheFilePath;

    public:
        HashMimAttack3 (ElgamalCryptosystem *c, unsigned int bits1, unsigned int bits2, const char *cacheFile);
        ~HashMimAttack3 ();
        bool buildTable (gmp_randstate_t rstate);
        size_t crackMessage (MpzList *results, ElgamalCipherText ct, gmp_randstate_t rstate, size_t maxResults=0);
        const char* getAttackName () const { return "hashmim3"; }
        
};
