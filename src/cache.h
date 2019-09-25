#ifndef CACHE
#define CACHE

#include "block.h"
#include <cstdint>

class ReplPolicy;

//Cache interface for implementation of simulator
class Cache : public AbstractMemory
{
private:
    //Pointer to an array of block pointers 
    Block **blocks;
    ReplPolicy *policy;
    Prefetcher *prefetcher;
    uint64_t cSize, associativity, blkSize, numSets;
    uint32_t delay;
    Block *currentBlock;
public:
    Cache(uint64_t ucSize, uint64_t uassociativity, uint64_t ublkSize, enum ureplPolicy, uint32_t delay);
    virtual ~Cache();

    virtual void makeCacheAllocation();
    bool checkIfBlockPresent(uint64_t addr);

    virtual uint32_t read(uint32_t addr, uint32_t size, uint8_t* data);
    virtual uint32_t write(uint32_t addr, uint32_t size, uint8_t data);

    virtual uint64_t getAssociativity();
    virtual uint64_t getNumSets();
    virtual uint64_t getBlockSize();
    virtual Block** getCacheBlock();
};

