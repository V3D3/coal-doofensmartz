#include <iostream>
#include <fstream>
#include <cassert>
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

// Address Constraints
#define C_TRAC_HEX_LEN 8
#define C_ADDR_LEN 32

//Note: Cache supports 32-bits, but leading bit is masked off since
//      it indicates r/w. If input is provided and processed differently
//      in main(), Cache can handle 32-bit addresses.

int log2(uint x)
{
    int r = 0;
    while(x > 0)
    {
        x >> 1;
        r++;
    }

    return r - 1;
}

int pow2(uint n)
{
    int r = 1;
    while(n > 0)
    {
        r << 1;
        r++;
    }


    return r;
}

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

void Memory::read(uint addr, uint* buffer, uint wordCount = 1)
{
    //dummy memory, does nothing
    for(uint i = 0; i < wordCount; i++)
    {
        buffer[i] = 0;
    }
}

void Memory::write(uint addr, uint* buffer, uint wordCount = 1)
{
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

    uint getOffset(uint addr);
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

    void write(uint address, uint* data, uint count = 1);
    void read(uint address, uint* data, uint count = 1);
};

typedef struct BlockNodest
{
    CacheBlock*          block = NULL;
    struct BlockNodest*  next  = NULL;
}  BlockNode;

uint CacheBlock::getOffset(uint addr)
{
    return addr & (blockSize - 1);
}

bool CacheBlock::isValid()
{
    return valid;
}

bool CacheBlock::isDirty()
{
    return dirty;
}

void CacheBlock::write(uint address, uint* data, uint count = 1)
{
    uint offset = getOffset(address);

    assert(valid);
    assert(count < blockSize);
    assert(offset + count < blockSize);

    dirty = valid;
    valid = valid || (!valid);
    
    for(int i = 0; i < count; i++)
    {
        this->data[offset + i] = data[i];
    }
}

void CacheBlock::read(uint address, uint* data, uint count = 1)
{
    uint offset = getOffset(address);

    assert(valid);
    assert(count < blockSize);
    assert(offset + count < blockSize);
    
    for(int i = 0; i < count; i++)
    {
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

    int offsetLength;
    int indexLength;
    int tagLength;
    
    Memory* memReference;

    uint getTag(uint addr);
    //reflects block access in PLRU/LRU/others
    void reflectBlockAccess(BlockNode* blockPtr);
    //adds new block to set, replaces victim block
    void addNewBlock(BlockNode* blockPtr);
    //writes back a victim block
    void writeBack(BlockNode* victimPtr);

public:
    Set(Memory* mR, int numSets, int setSize, int blockSize, int repPolicy)
    {
        //absorb params
        this->memReference = mR;
        
        this->size = setSize;
        this->blockSize = blockSize;

        this->repPolicy = repPolicy;

        //calculate addr fields' lengths
        offsetLength = log2(blockSize);
        indexLength = log2(numSets);
        tagLength = C_ADDR_LEN - (offsetLength + indexLength);

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

    void read(uint address, uint* data, uint count = 1);
    void write(uint address, uint* data, uint count = 1);
};

uint Set::getTag(uint addr)
{
    return (addr >> (C_ADDR_LEN - tagLength));
}

void Set::read(uint address, uint* data, uint count = 1)
{
    uint tag    = getTag(address);

    //look for block
    BlockNode* tmp = head;

    while(tmp != NULL)
    {
        if(tmp->block->isValid())
        {
            if(tmp->block->getTag() == tag)
            {
                ////// HIT ///////

                //read data
                tmp->block->read(address, data, count);

                //update block ordering (for r-policy)
                reflectBlockAccess(tmp);

                return;
            }
        }

        tmp = tmp->next;
    }

    ////// MISS //////

    //make a new block
    BlockNode* fetchedBlock = new BlockNode();
    fetchedBlock->block = new CacheBlock(blockSize);

    //read required location into it
    uint* buffer = new uint[blockSize];
    
    memReference->read(address, buffer, blockSize);
    fetchedBlock->block->write(address, buffer, blockSize);

    //add it to the set (replacing victim if needed)
    //automatically handles reflecting access
    addNewBlock(fetchedBlock);

    //read data into given buffer
    fetchedBlock->block->read(address, data, count);
}

void Set::write(uint address, uint* data, uint count = 1)
{
    uint tag = getTag(address);

    //look for block
    BlockNode* tmp = head;
    
    while(tmp != NULL)
    {
        if(tmp->block->isValid())
        {
            if(tmp->block->getTag() == tag)
            {
                ////// HIT //////

                //write data
                tmp->block->write(address, data, count);

                //update block ordering (for r-policy)
                reflectBlockAccess(tmp);

                return;
            }
        }

        tmp = tmp->next;
    }

    ////// MISS //////

    //make a new block
    BlockNode* fetchedBlock = new BlockNode();
    fetchedBlock->block = new CacheBlock(blockSize);

    //read from memory into it first*
    uint* buffer = new uint[blockSize];

    memReference->read(address, buffer, blockSize);
    fetchedBlock->block->write(address, buffer, blockSize);

    //write into it
    fetchedBlock->block->write(address, data, count);

    //add the new block
    addNewBlock(fetchedBlock);

    /*
        We need to read from mem into block first since block has just 1 dirty bit for
        the whole block. Hence if only one word is written to it, the other, say 3 words
        in the block are garbage and would overwrite useful data in memory on eviction.

        Hence we first get all the words from the memory that fit into a block, then
        write to this block (and delay reflecting the write into memory).
        The dirty bit is set on the second write to the same block, so this way is also
        consistent for setting that bit.
    */
}

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

    int offsetLength;
    
    int repPolicy;  //replacement policy
    Set** sets;     //pointer to represent sets

    uint getIndex(uint address);

public: //public functions
    Cache(Memory* mR, int cacheSize, int blockSize, int org, int repPolicy)
    {
        //absorb params
        this->cacheSize = cacheSize;
        this->blockSize = blockSize;

        this->numBlocks = cacheSize / blockSize;
        this->repPolicy = repPolicy;

        //set associativity
        if(org == 0)
        {
            org = this->numBlocks;
        }
        this->numSets = (this->numBlocks) / org;
        this->numWays = numBlocks / numSets;
        
        //calculate address field lengths
        offsetLength = log2(blockSize);

        //allocating required memory for sets
        sets = new Set*[numSets];
        for(int i = 0; i < numSets; i++)
        {
            sets[i] = new Set(mR, numSets, numWays, blockSize, repPolicy);
        }
    }

    void read(uint address, uint* buffer, uint count);  //reading from a required adress in cache
    void write(uint address, uint* buffer, uint count); //writing into a given adress in cache
};

uint Cache::getIndex(uint address)
{
    return (address >> offsetLength) & (numSets - 1);
}

void Cache::read(uint address, uint* buffer, uint count)
{
    uint index = getIndex(address);
    sets[index]->read(address, buffer, count);
}

void Cache::write(uint address, uint* buffer, uint count)
{
    uint index = getIndex(address);
    sets[index]->write(address, buffer, count);
}