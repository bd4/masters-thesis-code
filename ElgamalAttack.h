/*
 * Abstract base class for two-phase attacks on ElGamal
 */

typedef struct {
    mpz_t key;
    mpz_t value;
} MpzTableEntry;

typedef struct {
    size_t length;
    MpzTableEntry *entries;
} MpzTable;

int mpzTableEntryCompare (const void *a, const void *b);
int mpzTableEntryReverseCompare (const void *a, const void *b);

class ElgamalAttack {
    protected:
        unsigned int bits1, bits2;
        ElgamalCryptosystem *e;

    public:
        virtual ~ElgamalAttack () {}

        // return false if the table build failed, e.g. not enough memory
        virtual bool buildTable (gmp_randstate_t rstate) = 0;

        virtual bool crackMessage (mpz_t result, const ElgamalCipherText ct, gmp_randstate_t rstate);
        virtual size_t crackMessage (MpzList *results, const ElgamalCipherText ct,
                                     gmp_randstate_t rstate, size_t maxResults=0);
        virtual const char* getAttackName () const = 0; 
};
