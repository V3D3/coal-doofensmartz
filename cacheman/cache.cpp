#include <iostream>
#include<fstream>
/*-------------------------------------------------------------------------------------------------
*    Author   : Team DOOFENSMARTZ
*    Code     : CPP code for a Cache Simulator
*    Question : CS2610 A6
-------------------------------------------------------------------------------------------------*/

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
*    Class Name         : CacheBlock
*    Application        : Used to represent a block of data in a cache
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
typedef struct cacheblockst
{
    uint*       tag = NULL;       //tag of a cache block
    bool        dirty = false;    //a bit to indicate whether any changes are made to a block or not.
    bool        valid = false;    //valid bit
    cacheblock* nextPtr = NULL;   //pointer to a cache block.
    uint*       data = NULL;      //stores data of the cacheBlock
    int         blockSize;

    cacheblock(int blockSize)    //constructor for the cacheblock
    {
        this->blockSize = blockSize;    //getting values into struct variables
        this->data  = new uint[blockSize];   //the data of cache according to block
    }

    void  write(uint* data);
    uint* read();

} cacheblock;


/*-------------------------------------------------------------------------------------------------
*    Class Name         : set
*    Application        : Used to represent a set of blocks
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class set
{
    private:
        cacheblock* setRoot;    //head pointer of linked list of cache blocks
        int setSize;
        int repPolicy; //setSize is number of blocks in the cache, repPolocy is to identify replcement policy
        int blockSize;
        void blockReplacement();

    public: //publicFunctions
        set(int setSize, int repPolicy, int blockSize)
        {
            this->setSize = setSize;    //storing arguments into private variables of set
            this->repPolicy = repPolicy;
            this->blockSize = blockSize;    

            this->setRoot = new cacheblock(/*enter Args*/); 
            cacheblock* temp = this->setRoot;

            for(int i=1; i<setSize; i++)
            {
                temp->nextPtr = new cacheblock(/*enter Args*/); //creating next cacheBlock
                temp = temp->nextPtr;
            }
        }

        void setRead(); //reading a data from a paticular set
        void setWrite();    //writing the data to a particular set
};


/*-------------------------------------------------------------------------------------------------
*    Class Name         : Cache
*    Application        : Used to represent the cache memory
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class cache
{
    private:    //private variables
        int setCount;   //number of sets in the cache
        int cacheSize;  //size of the cache
        int blockSize;  //size of the cache block
        int numBlocks;  //number of blocks in the cache
        int numWays;    //number of ways in the cache
        int repPolicy;  //replacement policy
        set* cacheSet;  //pointer to represent sets

    public: //public variables
        int compMiss = 0, capMiss = 0, confMiss = 0, readMiss = 0, writeMiss = 0, dirtEvic = 0;

    public: //public functions
        cache(int cacheSize, int blockSize, int org, int repPolicy)
        {
            //absorb params
            this->cacheSize = cacheSize;
            this->blockSize = blockSize;
            this->numBlocks = cacheSize/blockSize;
            this->repPolicy = repPolicy;

            //set associativity
            if(org == 0)  {
                org = this->numBlocks;
            }
            this->setCount = (this->numBlocks) / org;

            //allocating required memory for sets
            cacheSet = new set[setCount]((this->numBlocks)/(this->setCount), repPolicy, blockSize);
        }

        int read(int);      //reading from a required adress in cache
        void write(int);    //writing into a given adress in cache
};


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

    ifstream fileObj;       //ofstream class object for the file Handling   //inputfile
    ofstream fileObjOut;    //ofstream class object for the file Handling   //outputfile
    uint hexCode;   //hexcode for request and address

    fileObj.open("input.txt", ios::in); //opens a file for reading
    fileObjOut.open("output.txe", ios::trunc | ios::out);   //opens a file and clears previous contents for new output

    fileObj >> cacheSize >> blocksize >> org >> repPolicy;  //cache parameters
    cache L1(cacheSize, blockSize, org, repPolicy); //creating a cache object

    while(fileObj >> hexCode)   //while EOF is not reached
    {
        //code for processing requests
    }

    fileObjOut << L1.compMiss << std::endl;
    fileObjOut << L1.capMiss << std::endl;
    fileObjOut << L1.confMiss << std::endl;
    fileObjOut << L1.readMiss << std::endl;
    fileObjOut << L1.writeMiss << std::endl;
    fileObjOut << L1.dirtEvic << std::endl;
    
    fileObj.close();    //closing the inputfile
    fileObjOut.close(); //closing the ouput file 
    return 0;   //succesful run of the code
}