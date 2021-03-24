#include<iostream>
/*-------------------------------------------------------------------------------------------------
*    Author   : Team DOOFENSMARTZ
*    Code     : CPP code for a Cache Simulator
*    Question : CS2610 A6
-------------------------------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------------------------------
*    Class Name         : CacheBlock
*    Application        : Used to represent a block of data in a cache
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
typedef struct cacheblock
{
    int* tag = NULL;    //tag of a cache block
    bool dirty = false; //a bit to indicate whether any changes are made to a block or not.
    bool valid = false; //valid bit
    cacheblock* nextPtr = NULL; //pointer to a cache block.
    int* data = NULL;   //stores data of the cacheBlock
    int blockSize;

    cacheblock(int blockSize)    //constructor for the cacheblock
    {
        this->blockSize = blockSize;    //getting values into struct variables
        this->data  = new int[blockSize];   //the data of cache according to block
    }

}cacheblock;


/*-------------------------------------------------------------------------------------------------
*    Class Name         : set
*    Application        : Used to represent a set of blocks
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class set
{
    cacheblock* setRoot;    //head pointer of linked list of cache blocks
    int setSize, repPolicy; //setSize is number of blocks in the cache, repPolocy is to identify replcement policy
    int blockSize;

    public: //publicFunctions
        set(int setSize, int repPolicy, int blockSize)
        {
            this->setSize = setSize;    //storing arguments into private variables of set
            this->repPolicy = repPolicy;
            this->blockSize = blockSize;    

            this->setRoot = new cacheblock(/*enter Args*/); 
            cacheblock temp = this->setRoot;

            for(int i=1; i<setSize; i++)
            {
                temp->nextPtr = new cacheblock(/*enter Args*/); //creating next cacheBlock
                temp = temp->nextPtr;
            }
        }

        void setRead(); //reading a data from a paticular set
        void setWrite();    //writing the data to a particular set

    private:
        void blockReplacement();
};


/*-------------------------------------------------------------------------------------------------
*    Class Name         : Cache
*    Application        : Used to represent the cache memory
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class cache
{
    int setCount;   //number of sets in the cache
    int cacheSize;  //size of the cache
    int blockSize;  //size of the cache block
    int numBlocks;  //number of blocks in the cache
    int numWays;    //number of ways in the cache
    int repPolicy;  //replacement policy
    set* cacheSet;  //pointer to represent sets

    public: //public functions
        cache(int cacheSize, int blockSize, int org, int repPolicy)
        {
            this->cacheSize = cacheSize;   //initialising all the input vals to class variables
            this->blockSize = blockSize;   //initialising all the input vals to class variables
            this->numBlock = cacheSize/blockSize;  //initialising all the input vals to class variables
            this->repPolicy = repPolicy;  //initialising all the input vals to class variables

            switch(org)
            {
                case 0:     this->setCount = 1;  //Fully associative
                            break;

                case 1:     this->setCount = this->numBlock;  //direct mapping
                            break;
                
                default:    this->setCount = (this->numBlock)/org; //set-associative
                            break;
            }

            cacheSet = new set[setCount];   //allocating required memory for sets
            for(int i=0; i<setCount; i++)   //for loop too construct the sets
                cacheSet[i] = set((this->numBlock)/(this->setCount), repPolicy, blockSize);    //calling the set constructor
        }

    int read(int);  //reading from a required adress in cache
    void write(int);    //writing into a given adress in cache
};