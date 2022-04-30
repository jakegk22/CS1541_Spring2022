#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "CacheCore.h"

CacheCore::CacheCore(uint32_t s, uint32_t a, uint32_t b, const char *pStr)
  : size(s)
  ,lineSize(b)
  ,assoc(a)
  ,numLines(s/b)
{
  if (strcasecmp(pStr, "RANDOM") == 0)
    policy = RANDOM;
  else if (strcasecmp(pStr, "LRU") == 0)
    policy = LRU;
  else {
    assert(0);
  }

  content = new CacheLine[numLines + 1];

  for(uint32_t i = 0; i < numLines; i++) {
    content[i].initialize();
  }
}

CacheCore::~CacheCore() {
  delete [] content;
}

CacheLine *CacheCore::accessLine(uint32_t addr)
{
  // TODO: Implement
  
  int cacheBlockOffset = log2i(CacheCore::lineSize); //use the size of each cache block to determine the # of offset bits. 
  int rowBits = log2i(numLines);                      //num bits needed in each addrs to indicate row.
  int tagSize = 32 - rowBits - cacheBlockOffset;      //num bits in tag? 
  uint32_t tag = addr >> 32-tagSize;                  //extract tag? probably gonna have to fix this.

  //then break down into tags 
  for(uint32_t i = 0; i < numLines; i++) {            //search through array of cache blocks i.e content[]
    content[i].incAge();                              //as we go through the cache we increment the ages of all CacheLines 
    if(content[i].getTag() == tag){                   //then match the addr to cache  block 
      content[i].resetAge();                          // and reset age of the access line
      return &content[i];                             //return the matching cacheLine
    }
  }  
  return NULL;
}

CacheLine *CacheCore::allocateLine(uint32_t addr, uint32_t *rplcAddr) {
  // TODO: Implement
  int cacheBlockOffset = log2i(CacheCore::lineSize); 
                                    //^use the size of each cache block to determine the # of offset bits.
  int rowBits = log2i(numLines);    //num bits needed in each addrs to indicate row.
  int tagSize = 32 - rowBits - cacheBlockOffset;      //num bits in tag? 
  uint32_t tag = addr >> 32-tagSize;//extract tag? probably gonna have to fix this.

  // maybe also a good time to see which line is the oldest too?
  int indx =0;                      //index varible to store cacheLine obj that is the oldest.
  uint32_t oldest = 0;              //var used to store the age of the oldest CacheLine.
  for(uint32_t i=0;i<numLines;i++){ //first pass is to search cache for opening first? 
    if(content[i].getAge()>oldest){ //if we find an OlderCache line. 
      indx = i;                     //then change the index to that of the Oldest.
      oldest = content[i].getAge(); //update oldest and the index of the Oldest.
    }
    if(!content[i].isValid()){      //if cache line is not valid (valid bit is zero) then allocate that line.
      content[i].validate();        //validate the cache line.
      content[i].setTag(tag);       //set the tag for our newly allocated block.
      content[i].resetAge();        //reset age of newly allocated block.
      return &content[i];            //return the newly allocated cacheLine. 
    }
  }

  //if we finish the for loop we could not find an invalid(empty) cache line
  //thus we have to replaces the LRU CacheLine. 
  if(content[indx].isDirty()){       //if Line we are replacing is dirty 
    int rplcLen=0; 
    for(int i=0;i<sizeof(rplcAddr);i++){     //traverse through array
      if(rplcAddr[i] = NULL)                  //if an open spot exisits add the address of the dirty thing
        rplcAddr[i]=content[indx].getTag();  //add dirty CacheLine to the rplcAddr
      
      rplcLen++;
    }
  }
  else{
    rplcAddr[0] = 0;                    //else replace address DNE 
  }

  content[indx].setTag(tag);        //change the old lines tag to the new tag
  content[indx].validate();         //revalidate the line, idt we need to do this but just to be safe.
  content[indx].resetAge();         //reset the age/

  return &content[indx];             //return the new line
}
