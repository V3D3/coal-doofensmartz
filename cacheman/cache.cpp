#include <iostream>
#include <fstream>
#include <cassert>
#include <iterator>
#include <set>
#include <cstring>

/*-------------------------------------------------------------------------------------------------
*    Author   : Team DOOFENSMARTZ
*    Code     : CPP code for a Cache Simulator
*    Question : CS2610 A6
-------------------------------------------------------------------------------------------------*/

typedef unsigned int uint;
typedef unsigned char word; //unused, need to switch over

// Cache Replacement Policies
#define C_CRP_LRU     1
#define C_CRP_RANDOM  0
#define C_CRP_TREE   2

// Output Scheme
#define C_COUT 0
#define C_HOUT 1

// Input Scheme
#define C_CIN 0
#define C_HIN 1

// Address Constraints
#define C_TRAC_HEX_LEN 8
#define C_ADDR_LEN 32

// Miss indicators
#define C_HIT 0
#define C_MISS_INV 1
#define C_MISS_VAL 2
#define C_MISS_DIR 3

//Note: Cache supports 32-bits, but leading bit is masked off since
//      it indicates r/w. If input is provided and processed differently
//      in main(), Cache can handle 32-bit addresses.

//////      UTILITY FUNCTIONS      //////

int log2(uint x)
{
    int r = 0;
    while(x > 0)
    {
        x = x >> 1;
        r++;
    }

    return r - 1;
}

int pow2(uint n)
{
    int r = 1;
    while(n > 0)
    {
        r = r << 1;
        n--;
    }

    return r;
}

//////      CLASSES      //////

class Memory;
class CacheBlock;
class Set;
class VictimManager;
class RandomVictimManager;
class LRUVictimManager;
class TreeVictimManager;
class Cache;

/*-------------------------------------------------------------------------------------------------
*    Class Name         : Memory
*    Application        : Simulates memory
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/

class Memory
{
public:
    //reads <wordCount> words from memory into buffer[]
    void read(uint addr, uint* buffer, uint wordCount = 1);
    void write(uint addr, uint* buffer, uint wordCount = 1);
};

/*-------------------------------------------------------------------------------------------------
*    Class Name         : CacheBlock
*    Application        : Used to represent a block of data in a cache
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class CacheBlock
{
private:
    int    tag = -1;         //stores tag
    bool   dirty = false;    //any writes to block?
    bool   valid = false;    //any reads into block?
    
    int    blockSize;
    uint*  data = NULL;      //stores data of the cacheBlock

    //gets offset from an address
    uint getOffset(uint addr);
public:
    //constructor: inits data
    CacheBlock(int blockSize);
    
    //getters and setters
    bool isValid();
    bool isDirty();
    
    uint getTag();
    void setTag(uint tag);

    //writes <count> words into <data> from this block given the address
    void write(uint address, uint* data, uint count = 1);
    void read(uint address, uint* data, uint count = 1);
};

//adds doubly-linked-list functionality to CacheBlock
typedef struct BlockNodest
{
    CacheBlock*          block = NULL;
    struct BlockNodest*  prev  = NULL;
    struct BlockNodest*  next  = NULL;
}  BlockNode;

/*-------------------------------------------------------------------------------------------------
*    Class Name         : Set
*    Application        : Used to represent a set of blocks
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class Set
{
private:
    BlockNode*  head;       //head pointer of linked list of cache blocks
    int         size;       //size is number of blocks in the set
    int         repPolicy;  //repPolicy is to identify replacement policy
    int         blockSize; 
    int         validBlocks = 0; //number of valid blocks in this set

    //lengths of these fields in bits
    int offsetLength;
    int indexLength;
    int tagLength;
    //index of this set
    int index;
    
    //reference to memory, and a victim manager
    Memory* memReference;
    VictimManager* vicMan;

    //get tag from address
    uint getTag(uint addr);
    //reflects block access in PLRU/LRU/others
    void reflectBlockAccess(BlockNode* blockPtr);
    //adds new block to set, replaces victim block
    int addNewBlock(BlockNode* blockPtr);
    //writes back a victim block
    void writeBack(BlockNode* victimPtr);

public:
    //constructor: many functions
    Set(Memory* mR, int index, int numSets, int setSize, int blockSize, int repPolicy);

    //read <count> words from address into <data[]>
    int read(uint address, uint* data, uint count = 1);
    int write(uint address, uint* data, uint count = 1);

    //friends since they access the Blocks LinkedList
    friend class VictimManager;
    friend class RandomVictimManager;
    friend class LRUVictimManager;
    friend class TreeVictimManager;
};

/*-------------------------------------------------------------------------------------------------
*    Classes            : VictimManager (and specific implementations)
*    Application        : Assist in tracking victims in Set
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
//base interface/abstract class
class VictimManager
{
public:
    //triggers bookkeeping for the replacement policy when a block is accessed
    virtual void reflectBlockAccess(BlockNode* accessedPtr) = 0;
    //gets the victim block address
    virtual BlockNode* getVictim() = 0; //inclusive of invalid blocks
};

//random (counter-based, not random in the exact sense)
//advantage: no need to check block validity
class RandomVictimManager : public VictimManager
{
private:
    uint counter = 0;   //iterates across block linked list
    Set* setRef = NULL; //refers to original set
public:
    RandomVictimManager(Set* sR);

    //nothing to reflect here
    void reflectBlockAccess(BlockNode* accessedPtr)  {}
    BlockNode* getVictim();
};

//least recently used: brings accessed block to head
//evicted: last block
class LRUVictimManager : public VictimManager
{
private:
    Set* setRef = NULL;
public:
    LRUVictimManager(Set* sR);

    void reflectBlockAccess(BlockNode* accessedPtr);
    BlockNode* getVictim();
};

//tree-based psuedo-lru: uses a complete binary tree
class TreeVictimManager : public VictimManager
{
private:
    //a bool array representing the tree
    bool* tree = NULL;
    Set* setRef = NULL;

    //needed for tree implementation
    uint setSize;
public:
    TreeVictimManager(Set* sR);

    void reflectBlockAccess(BlockNode* accessedPtr);
    BlockNode* getVictim();
};


/*-------------------------------------------------------------------------------------------------
*    Class Name         : Cache
*    Application        : Used to represent the cache
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class Cache
{
private:
    int numSets;   //number of sets in the cache
    int numWays;    //number of ways in the cache
    int numBlocks;  //number of blocks in the cache

    int cacheSize;  //size of the cache (words)
    int blockSize;  //size of the cache block (words)

    //length of the offset field in bits in an address
    int offsetLength;
    
    int repPolicy;  //replacement policy
    Set** sets;     //pointer to represent sets

    //gets an index from an address
    uint getIndex(uint address);

    //stores the accessed addresses from cache
    //useful to determine compulsory misses
    std::set<int, std::greater<int>> stat_addr_queried;
public:
    Cache(Memory* mR, int cacheSize, int blockSize, int org, int repPolicy);

    //read <count> words from <address> into <buffer[]>
    void read(uint address, uint* buffer, uint count = 1);
    void write(uint address, uint* buffer, uint count = 1);

    //statistics
    int stat_cache_read = 0;
    int stat_cache_write = 0;
    int stat_cache_access = 0;

    int stat_cache_miss = 0;
    int stat_cache_miss_read = 0;
    int stat_cache_miss_write = 0;

    int stat_cache_miss_compulsory = 0;
    int stat_cache_miss_capacity = 0;
    int stat_cache_miss_conflict = 0;
    
    int stat_cache_dirty_evicted = 0;
};


//////////////////////////////////////////////////////////////////////
/////////////////////     MEMORY DEFINITIONS     /////////////////////
//////////////////////////////////////////////////////////////////////

void Memory::read(uint addr, uint* buffer, uint wordCount)
{
    //dummy memory, does nothing
    for(uint i = 0; i < wordCount; i++)
    {
        buffer[i] = 0;
    }
}

void Memory::write(uint addr, uint* buffer, uint wordCount)
{
    //does absolutely nothing
}

//////////////////////////////////////////////////////////////////////
/////////////////////     BLOCK DEFINITIONS     //////////////////////
//////////////////////////////////////////////////////////////////////

CacheBlock::CacheBlock(int blockSize)
{
    //absorb param and allocate memory for data
    this->blockSize = blockSize;
    this->data      = new uint[blockSize];
}

//gets offset from address
uint CacheBlock::getOffset(uint addr)
{
    //gets last bits
    return addr & (blockSize - 1);
}

//getters and setters
bool CacheBlock::isValid()          {return valid;}
bool CacheBlock::isDirty()          {return dirty;}
uint CacheBlock::getTag()           {return tag;}
void CacheBlock::setTag(uint tag)   {this->tag = tag;}

void CacheBlock::write(uint address, uint* data, uint count)
{
    uint offset = getOffset(address);

    assert(count <= blockSize);
    assert(offset + count <= blockSize);

    //first write: valid false => dirty false (getting from memory into block)
    //second write: dirty: true
    dirty = valid;
    valid = true;

    //write data into the block, from given data[]   
    for(int i = 0; i < count; i++)
    {
        this->data[offset + i] = data[i];
    }
}

void CacheBlock::read(uint address, uint* data, uint count)
{
    uint offset = getOffset(address);

    //block must be valid to be read from

    //these are just for correctness of code, validity is checked before
    // a read is requested

    assert(valid);
    assert(count <= blockSize);
    assert(offset + count <= blockSize);
    
    //read from this block into data[]
    for(int i = 0; i < count; i++)
    {
        data[i] = this->data[offset + i];
    }
}

//////////////////////////////////////////////////////////////////////
////////////////      VICTIM MANAGER DEFINITIONS      ////////////////
//////////////////////////////////////////////////////////////////////

//constructor: get reference to set
RandomVictimManager::RandomVictimManager(Set* sR)
{
    this->setRef = sR;
}

BlockNode* RandomVictimManager::getVictim()
{
    //counter points to current empty location
    //when set has invalid blocks, else to a valid block
    //copy counter
    uint t = counter;

    //increment counter
    counter = (counter + 1) % setRef->size;;
    
    //traverse linked list to invalid/victim location
    BlockNode* tmp = setRef->head;
    while(t > 0)  {
        tmp = tmp->next;
        t--;
    }

    //return the node's address
    return tmp;
}

LRUVictimManager::LRUVictimManager(Set* sR)
{
    this->setRef = sR;
}

void LRUVictimManager::reflectBlockAccess(BlockNode* accessedPtr)
{
    //store current head
    BlockNode* tmp = setRef->head;


    //remove accessed block from middle of set
    if(accessedPtr->prev != NULL)  {
        accessedPtr->prev->next = accessedPtr->next;
    }
    if(accessedPtr->next != NULL)  {
        accessedPtr->next->prev = accessedPtr->prev;
    }

    //make it the head
    setRef->head = accessedPtr;
    accessedPtr->prev = NULL;

    //add the rest of the list to the end of the new head
    setRef->head->next = tmp;
    tmp->prev = setRef->head;
}

BlockNode* LRUVictimManager::getVictim()
{
    //get last node
    BlockNode* tmp = setRef->head;
    while(tmp->next != NULL)
    {
        tmp = tmp->next;
    }

    return tmp;
}

TreeVictimManager::TreeVictimManager(Set* sR)
{
    this->setRef = sR;
    this->setSize = setRef->size;

    //() : all init to false
    tree = new bool[setSize - 1]();
}

void TreeVictimManager::reflectBlockAccess(BlockNode* accessedPtr)
{
    //intent: make bits in the path to root point away

    //get index of accessedPtr in list
    uint index = 0;
    while(accessedPtr->prev != NULL)
    {
        index++;
        accessedPtr = accessedPtr->prev;
    }

    //adjusted for 0-based indexing
    int curr = index + setSize - 1; //issue: curr is int here
    while(curr > -1)
    {
        //make the parent bit point away
        tree[(curr - 1) / 2] = (curr % 2);
        //go to parent bit
        curr = (curr - 1) / 2;
    }
}

BlockNode* TreeVictimManager::getVictim()
{
    //adjusted for 0-based indexing
    uint curr = 0;

    while(curr < setSize - 1)
    {
        //go to child according to current bit
        curr = (2 * curr) + tree[curr] + 1;

        //invert the bit (/2 to go back up)
        tree[(curr - 1) / 2] = !tree[(curr - 1) / 2];
    }

    curr = curr - setSize + 1;

    //at this point, curr points to index of victim in set
    BlockNode* temp = setRef->head;
    while(curr > 0)
    {
        temp = temp->next;
        curr--;
    }

    return temp;
}

//////////////////////////////////////////////////////////////////////
///////////////////////    SET DEFINITIONS     ///////////////////////
//////////////////////////////////////////////////////////////////////

uint Set::getTag(uint addr)
{
    return (addr >> (C_ADDR_LEN - tagLength));
}

void Set::reflectBlockAccess(BlockNode* blockPtr)
{
    vicMan->reflectBlockAccess(blockPtr);
}

int Set::addNewBlock(BlockNode* blockPtr)
{
    int hitstatus; //for stats

    //get victim ptr
    BlockNode* victimPtr = vicMan->getVictim();

    //check if it is valid, report accordingly
    bool valid = victimPtr->block->isValid();

    hitstatus = C_MISS_INV;
    if(valid)
    {
        if(validBlocks == size)  {hitstatus = C_MISS_VAL;}
        if(victimPtr->block->isDirty())
        {
            if(validBlocks == size)  {hitstatus = C_MISS_DIR;}
            writeBack(victimPtr);
        }
    }
    else
    {
        validBlocks++;
    }

    //add new block into list
    if(victimPtr->prev != NULL)
    {
        victimPtr->prev->next = blockPtr;
        blockPtr->prev = victimPtr->prev;
    }
    if(victimPtr->next != NULL)
    {
        victimPtr->next->prev = blockPtr;
        blockPtr->next = victimPtr->next;
    }
    reflectBlockAccess(blockPtr);
    //get rid of the victim block
    delete victimPtr;

    return hitstatus;
}

void Set::writeBack(BlockNode* victimPtr)
{
    //compute address in memory
    uint memAddr = victimPtr->block->getTag();
    memAddr = (memAddr << indexLength) + index;
    memAddr = (memAddr << offsetLength);

    //write the block into memory
    uint* buffer = new uint[blockSize];
    victimPtr->block->read(0, buffer, blockSize);
    memReference->write(memAddr, buffer, blockSize);
}

Set::Set(Memory* mR, int index, int numSets, int setSize, int blockSize, int repPolicy)
{
    //absorb params
    this->memReference = mR;
    this->index = index;
    
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
        temp->next->prev = temp;

        temp = temp->next;
    }

    //at this point, all blocks have been allocated, and are marked as invalid (default)

    //instantiate a victim manager
    if(repPolicy == C_CRP_RANDOM)
    {
        vicMan = new RandomVictimManager(this);
    }
    else if(repPolicy == C_CRP_LRU)
    {
        vicMan = new LRUVictimManager(this);
    }
    else if(repPolicy == C_CRP_TREE)
    {
        vicMan = new TreeVictimManager(this);
    }
}

int Set::read(uint address, uint* data, uint count)
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
                ////// HIT ///////

                //read data
                tmp->block->read(address, data, count);

                //update block ordering (for r-policy)
                reflectBlockAccess(tmp);

                return C_HIT;
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
    
    uint readaddr = (address / blockSize) * blockSize;

    memReference->read(readaddr, buffer, blockSize);
    fetchedBlock->block->write(readaddr, buffer, blockSize);
    fetchedBlock->block->setTag(getTag(readaddr));

    //read data into given buffer
    fetchedBlock->block->read(address, data, count);

    //add it to the set (replacing victim if needed)
    //automatically handles reflecting access
    return addNewBlock(fetchedBlock);
}

int Set::write(uint address, uint* data, uint count)
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

                return C_HIT;
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

    //make offset 0 to get block
    uint readaddr = (address / blockSize) * blockSize;

    //get block
    memReference->read(readaddr, buffer, blockSize);
    fetchedBlock->block->write(readaddr, buffer, blockSize);
    fetchedBlock->block->setTag(getTag(readaddr));

    //write into it
    fetchedBlock->block->write(address, data, count);

    //add the new block
    return addNewBlock(fetchedBlock);

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

//////////////////////////////////////////////////////////////////////
////////////////////      CACHE DEFINITIONS      /////////////////////
//////////////////////////////////////////////////////////////////////

Cache::Cache(Memory* mR, int cacheSize, int blockSize, int org, int repPolicy)
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
        sets[i] = new Set(mR, i, numSets, numWays, blockSize, repPolicy);
    }

    //for compulsory misses stat
    
}

uint Cache::getIndex(uint address)
{
    return (address >> offsetLength) & (numSets - 1);
}

void Cache::read(uint address, uint* buffer, uint count)
{
    stat_cache_access++;
    stat_cache_read++;

    //these two lines are actual read, others are for stats
    uint index = getIndex(address);
    int hitstatus = sets[index]->read(address, buffer, count);

    if(hitstatus != C_HIT)  {
        stat_cache_miss++;
        stat_cache_miss_read++;
        if(numSets == 1)  { //fully assoc
            stat_cache_miss_capacity++;
        }
        if(hitstatus == C_MISS_VAL)  {
            stat_cache_miss_conflict++;
        }
        if(hitstatus == C_MISS_DIR)  {
            stat_cache_miss_conflict++;
            stat_cache_dirty_evicted++;
        }
    }

    if(stat_addr_queried.find(address) == stat_addr_queried.end())  {
        stat_addr_queried.insert(address);
        stat_cache_miss_compulsory++;
    }
}

void Cache::write(uint address, uint* buffer, uint count)
{
    stat_cache_access++;
    stat_cache_write++;

    uint index = getIndex(address);
    int hitstatus = sets[index]->write(address, buffer, count);

    if(hitstatus != C_HIT)  {
        stat_cache_miss++;
        stat_cache_miss_write++;
        if(numSets == 1)  { //fully assoc
            stat_cache_miss_capacity++;
        }
        if(hitstatus == C_MISS_VAL)  {
            stat_cache_miss_conflict++;
        }
        if(hitstatus == C_MISS_DIR)  {
            stat_cache_miss_conflict++;
            stat_cache_dirty_evicted++;
        }
    }

    if(stat_addr_queried.find(address) == stat_addr_queried.end())  {
        stat_addr_queried.insert(address);
        stat_cache_miss_compulsory++;
    }
}

/*-------------------------------------------------------------------------------------------------
*    Function Name : main
*    Args          : Nil
*    Return Type   : int(0)
*    Application   : Entry point to the Proram
-------------------------------------------------------------------------------------------------*/
int main()
{
    std::cout << "Cache Simulator" << std::endl;
    int cacheSize, blockSize, org, repPolicy;   //parameters required to define the cache
    char command;
    uint buffer;
    std::string hexCode;   //hexcode for request and address
    std::string filename;

    std::cin >> cacheSize >> blockSize >> org >> repPolicy;  //cache parameters
    std::cin >> filename;   //taking input for the filename
    std::ifstream fileObj;       //ofstream class object for the file Handling
    fileObj.open(filename, std::ios::in); //opens a file for reading

    Memory* MainMem = new Memory(); //creating a main memory object
    Cache L1(MainMem,cacheSize, blockSize, org, repPolicy); //creating a cache object

    while(fileObj >> hexCode)   //while EOF is not reached
    {
        fileObj >> command;
        if(command == 'r')
            L1.read(std::stoi(hexCode,0,16), &buffer);

        else if(command == 'w')
            L1.write(std::stoi(hexCode,0,16), &buffer);
    }

    std::cout << L1.stat_cache_access << std::endl;
    std::cout << L1.stat_cache_read << std::endl;
    std::cout << L1.stat_cache_write << std::endl;
    std::cout << L1.stat_cache_miss << std::endl;
    std::cout << L1.stat_cache_miss_compulsory << std::endl;

    if(!org)
        std::cout << L1.stat_cache_miss_capacity << std::endl;

    else
        std::cout << 0 << std::endl;
    std::cout << L1.stat_cache_miss_conflict << std::endl;
    std::cout << L1.stat_cache_miss_read << std::endl;
    std::cout << L1.stat_cache_miss_write << std::endl;
    std::cout << L1.stat_cache_dirty_evicted << std::endl;
    
    fileObj.close();    //closing the inputfile
    return 0;   //succesful run of the code
}