#include "cache.h"
#include "repl_policy.h"
#include <cstdlib>

Cache :: Cache(uint64_t ucSize, uint64_t uassociativity, uint64_t ublkSize, enum ureplPolicy, uint32_t udelay)
{
    cSize = ucSize;
    associativity = uassociativity;
    blkSize = ublkSize;
    replPolicy = ureplPolicy;
    delay = udelay;
    currentBlock = NULL;
}

Cache ::~Cache
{

}

void Cache :: makeCacheAllocation()
{
    numSets = cSize/(blkSize * associativity);
    blocks = new Block*[numSets];

    for (int i = 0; i < (int) numSets; i ++)
    {
       blocks[i] = new Block[associativity];
    }
}

uint64_t Cache :: getAssociativity()
{
  return associativity;
}

uint64_t Cache :: getNumSets()
{
  return numSets;
}

uint64_t Cache :: getBlockSize()
{
  return Blksize;
}

Block** Cache :: getCacheBlock()
{
  return **blocks;
}

bool Cache :: checkIfBlockPresent(uint64_t addr)
{
    addr = addr / Blksize;
    
    uint64_t setIndex = (numSets - 1) & addr;
    uint64_t addrTag = addr / numSets;

    Block* candidateBlock  = blocks[setIndex];

    for (int i = 0; i < (int) associativity; i++)
        {
           if ((candidateBlock[i]->getValid() == 1) && (candidateBlock[i]->getTag() == addrTag))
               {
                 currentBlock = candidateBlock[i];   
                 return True;   
               } 
        }
    return False;    
}

uint32_t Cache :: read(uint32_t addr, uint32_t size, uint8_t* data)
{
    //in case we are getting a cache hit
    if (checkIfBlockPresent(addr))
       {
           data = currentBlock->getData();
           return delay;
       }
    //goto next level of memory hierarchy in case of cache miss
    return delay + next->read();
}

uint32_t Cache :: write(uint32_t addr, uint32_t size, uint8_t* data)
{
    //in case we are getting a cache hit
    if (checkIfBlockPresent(addr))
       {
           currentBlock->writeData(data);
           return delay;
       }

    //goto next level of memory hierarchy in case of cache miss
    return delay + next->write();
}