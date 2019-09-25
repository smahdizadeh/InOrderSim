#ifndef REPLACEMENTPOLICY
#define REPLACEMENTPOLICY

class Cache;

class ReplacementPolicy
{
private:    
  ReplacementPolicy(Cache* cache);
  Cache* cache;
  uint64_t associativity, blkSize, total_sets;
  

public:
  virtual Block* getVictim(uint32_t addr, bool isWrite) = 0;
  virtual void update(uint32_t addr, bool isWrite) = 0;
  virtual Block* checkCacheSetFull(uint32_t addr, bool isWrite) = 0;
};

class LRU : public ReplacementPolicy
{
  LRU(Cache* cache);
  
  uint64_t total_sets, associativity, blkSize;
  public:
    virtual Block* checkCacheSetFull(uint32_t addr, bool isWrite);
    virtual Block* getVictim(uint32_t addr, bool isWrite);
    virtual void update(uint32_t addr, bool isWrite);
}; 


class Random : public ReplacementPolicy
{
  Random(Cache* cache);
  uint64_t total_sets, associativity, blkSize;
  public:
     virtual Block* checkCacheSetFull(uint32_t addr, bool isWrite);


}