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

    cacheblock()    //constructor for the cacheblock
    {

    }

}cacheblock;


/*-------------------------------------------------------------------------------------------------
*    Class Name         : set
*    Application        : Used to represent a set of blocks
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class set
{

};


/*-------------------------------------------------------------------------------------------------
*    Class Name         : Cache
*    Application        : Used to represent the cache memory
*    Inheritances       : Nil
-------------------------------------------------------------------------------------------------*/
class cache
{

};