/*
 * =====================================================================================
 *
 *       Filename:  MpzList.h
 *
 *    Description:  Class for dynamic mpz_t lists.
 *
 *        Version:  1.0
 *        Created:  11/02/2008 10:37:36 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bryce Allen (bda), bryce@bda.ath.cx
 *        Company:  
 *
 * =====================================================================================
 */

/*
typedef struct {
    mpz_t value;
    MpzTreeNode *left;
    MpzTreeNode *right;
} MpzTreeNode;
*/

class MpzList {
    private:
        mpz_t *list;
        //MpzTreeNode *root;
        size_t size;
        size_t mallocSize;
        size_t mallocSizeIncrement;
        size_t initCount;

    public:
        MpzList (size_t initialSize=5, size_t sizeIncrement=5);
        ~MpzList ();
        size_t append (mpz_t value);
        //size_t appendInt (int value);
        //size_t appendUInt (unsigned int value);
        void clear ();
        size_t compactify ();
        size_t getSize ();
        size_t getInitCount ();
        bool find (size_t *index, mpz_t value);

        mpz_t &operator[] (const size_t index);
};
