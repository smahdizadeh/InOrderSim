#ifndef BLOCK
#define BLOCK

#include <cstdint>
#include <cassert>
#include "util.h"

class Block
{
   private:
      uint8_t blkSize;
      uint8_t* data;
      uint64_t tag, dirty;
      uint8_t valid;
   public:
       uint8_t getValid()
         {
            return valid;
         }

         uint64_t getTag()
         {
            return tag;
         }

         uint8_t* getData()
         {
            return data;
         } 

         void writeData(int offset, uint8_t* wdata, uint8_t size)
         {
            assert (offset >= 0 && offset + size < blkSize);
            for(int i = 0; i < size; i++){
               *(data + offset + i) = *(wdata + i);
            }
            dirty  = 1;
         }

   Block(uint8_t blkSize) : blkSize(blkSize)
   {
      valid = 0; 
      dirty = 0;
      data = new uint8_t [blkSize];
   }
   ~Block(){
	   delete(data);
   }
   
};
