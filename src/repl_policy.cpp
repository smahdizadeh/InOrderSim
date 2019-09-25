#include "repl_policy.h"


ReplacementPolicy :: ReplacementPolicy(Cache* cache) : cache(cache) 
{

}

LRU :: LRU(Cache* cache) : ReplacementPolicy(cache)
{
  uint8_t** replBlocks;
    
  total_sets  = cache->getNumSets();
  associativity  = cache->getAssociativity();
  blkSize = cache->getBlockSize();
  time = 0;
 
  replBlocks = new uint8_t*[total_sets];
  for (int i = 0; i < (int) total_sets; i ++)
    {
       replBlocks[i] = new uint8_t[associativity];
       for (int j = 0; j < (int)associativity; j++)
         {
            replBlocks[i][j] = j;
         }
    }
}

Block* LRU :: checkCacheSetFull(uint32_t addr, bool isWrite)
{
    addr = addr / Blksize;
    
    uint64_t setIndex = (numSets - 1) & addr;

    Block** cache_blocks = cache->getCacheBlock();
    Block* candidateBlock  = cache_blocks[setIndex];

    for (int i = 0; i < (int) associativity; i++)
    {
       if (candidateBlock[i]->getValid() == 1)
         {
           return candidateBlock[i];
         }     
    }
    return NULL;
}

Block* LRU :: getVictim(uint32_t addr, bool isWrite)
{
   Block* vic = checkCacheSetFull(uint32_t addr, bool isWrite);

   if (vic == NULL)
      {
        addr = addr / Blksize;
    
        uint64_t setIndex = (numSets - 1) & addr;

        Block** cache_blocks = cache->getCacheBlock();
        Block* candidateBlock  = cache_blocks[setIndex];

        uint8_t* candidate_replaced_blocks = replBlocks[setIndex];
        for (int i = 0; i < (int)associativity; i++)
        {
          if (candidate_replaced_blocks[i] == 0)
              break;    
        }

      //Do the update of decrementing the LRU Counter
      for (int j = 0; j < (int)associativity; i++)
        {
          if (i != j)
             candidate_replaced_blocks[j]--; 
        }

      vic =  candidateBlock[i];
      }
    return vic;  
}

void LRU :: update(uint32_t addr, bool isWrite)
{
    
}

Random :: Random(Cache* cache) : ReplacementPolicy(cache)
{
  total_sets  = cache->getNumSets();
  associativity  = cache->getAssociativity();
  blkSize = cache->getBlockSize();
}

Block* Random :: checkCacheSetFull(uint32_t addr, bool isWrite)
{
    addr = addr / Blksize;
    
    uint64_t setIndex = (numSets - 1) & addr;

    Block** cache_blocks = cache->getCacheBlock();
    Block* candidateBlock  = cache_blocks[setIndex];

    for (int i = 0; i < (int) associativity; i++)
    {
       if (candidateBlock[i]->getValid() == 1)
         {
           return candidateBlock[i];
         }     
    }
    return NULL;
}

Block* Random :: getVictim(uint32_t addr, bool isWrite)
{
   Block* vic = checkCacheSetFull(uint32_t addr, bool isWrite);

   if (vic == NULL)
      {
        addr = addr / Blksize;
    
        uint64_t setIndex = (numSets - 1) & addr;

        Block** cache_blocks = cache->getCacheBlock();
        Block* candidateBlock  = cache_blocks[setIndex];

        int repl_ind = rand() % associativity;
        vic = candidateBlock[repl_ind]
      }
    return vic;  
}

void Random :: update(uint32_t addr, bool isWrite)
{
    
}
