#include <iostream>
/*-------------------------------------------------------------------------------------------------
*    Author   : Team DOOFENSMARTZ
*    Code     : CPP code for a Cache Simulator
*    Question : CS2610 A6
-------------------------------------------------------------------------------------------------*/

/*
    progress (+ implemented ~ current - left)
        + memory
        + block
        ~ set
        - cache
*/

typedef unsigned int uint;

// Cache Replacement Policies
#define C_CRP_LRU     0
#define C_CRP_RANDOM  1
#define C_CRP_PSLRU   2

// Output Scheme
#define C_COUT 0
#define C_HOUT 1

// Input Scheme
#define C_CIN 0
#define C_HIN 1

/*-------------------------------------------------------------------------------------------------
*    Class Name         : Memory
*    Application        : Simulates memory
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/

class Memory
{
public:
    void read(uint addr, uint* buffer, uint wordCount = 1);
    void write(uint addr, uint* buffer, uint wordCount = 1);
};

void Memory::read(uint addr, uint* buffer, uint wordCount = 1)  {
    //dummy memory, does nothing
    for(uint i = 0; i < wordCount; i++)  {
        buffer[i] = 0;
    }
}

void Memory::write(uint addr, uint* buffer, uint wordCount = 1)  {
    //does absolutely nothing
}

/*-------------------------------------------------------------------------------------------------
*    Class Name         : CacheBlock
*    Application        : Used to represent a block of data in a cache
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class CacheBlock
{
private:
    int    tag = -1;         //tag of a cache block
    bool   dirty = false;    //a bit to indicate whether any changes are made to a block or not.
    bool   valid = false;    //valid bit
    
    int    blockSize;
    uint*  data = NULL;      //stores data of the cacheBlock

public:
    CacheBlock(int blockSize)
    {
        this->blockSize = blockSize;    //getting values into struct variables
        this->data      = new uint[blockSize];   //the data of cache according to block
    }
    
    bool isValid();
    bool isDirty();
    
    uint getTag();
    void setTag(uint tag);

    void write(uint offset, uint* data, uint count = 1);
    void read(uint offset, uint* bufferWord, uint count = 1);
};

typedef struct BlockNodest
{
    CacheBlock*          block = NULL;
    struct BlockNodest*  next  = NULL;
}  BlockNode;

bool CacheBlock::isValid()  {
    return valid;
}

bool CacheBlock::isDirty()  {
    return dirty;
}

void CacheBlock::write(uint offset, uint* data, uint count = 1)  {
    assert(valid);
    assert(count < blockSize);
    assert(offset + count < blockSize);

    dirty = true;
    
    for(int i = 0; i < count; i++)  {
        this->data[offset + i] = data[i];
    }
}

void CacheBlock::read(uint offset, uint* data, uint count = 1)  {
    assert(valid);
    assert(count < blockSize);
    assert(offset + count < blockSize);
    
    for(int i = 0; i < count; i++)  {
        data[i] = this->data[offset + i];
    }
}

/*-------------------------------------------------------------------------------------------------
*    Class Name         : Set
*    Application        : Used to represent a set of blocks
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class Set
{
private:
    BlockNode*  head;       //head pointer of linked list of cache blocks
    int         size;       //size is number of blocks in the cache
    int         repPolicy;  //repPolicy is to identify replcement policy
    int         blockSize;

    void        blockReplacement();
    uint        getTag(uint addr);

public: //publicFunctions
    Set(int setSize, int repPolicy, int blockSize)
    {
        //absorb params
        this->size = setSize;
        this->repPolicy = repPolicy;
        this->blockSize = blockSize;    

        //allocate blocks
        this->head = new BlockNode(); 
        this->head->block = new CacheBlock(blockSize);

        BlockNode* temp = this->head;

        for(int i = 1; i < setSize; i++)
        {
            temp->next = new BlockNode();
            temp->next->block = new CacheBlock(blockSize);

            temp = temp->next;
        }

        //at this point, all blocks have been allocated, and are marked as invalid (default)
    }

    //read from an address
    uint read(uint address);
    void write(uint address, uint data);
};

/*-------------------------------------------------------------------------------------------------
*    Class Name         : Cache
*    Application        : Used to represent the cache memory
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class Cache
{
private:
    int numSets;   //number of sets in the cache
    int numBlocks;  //number of blocks in the cache
    int numWays;    //number of ways in the cache

    int cacheSize;  //size of the cache
    int blockSize;  //size of the cache block
    
    int repPolicy;  //replacement policy
    Set** sets;  //pointer to represent sets

public: //public functions
    Cache(int cacheSize, int blockSize, int org, int repPolicy)
    {
        //absorb params
        this->cacheSize = cacheSize;
        this->blockSize = blockSize;

        this->numBlocks = cacheSize / blockSize;
        this->repPolicy = repPolicy;

        //set associativity
        if(org == 0)  {
            org = this->numBlocks;
        }
        this->numSets = (this->numBlocks) / org;

        //allocating required memory for sets
        sets = new Set*[numSets];
        for(int i = 0; i < numSets; i++)
        {
            sets[i] = new Set((this->numBlocks)/(this->numSets), repPolicy, blockSize);
        }
    }

    int read(int);      //reading from a required adress in cache
    void write(int);    //writing into a given adress in cache
};