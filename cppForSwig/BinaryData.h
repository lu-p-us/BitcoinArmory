////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (C) 2011-2015, Armory Technologies, Inc.                        //
//  Distributed under the GNU Affero General Public License (AGPL v3)         //
//  See LICENSE-ATI or http://www.gnu.org/licenses/agpl.html                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef _BINARYDATA_H_
#define _BINARYDATA_H_

#include <stdio.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
	#if _MSC_PLATFORM_TOOLSET < 110
		#include <stdint.h>
   #endif

   #ifndef ssize_t
      #ifdef _WIN32
         #define ssize_t SSIZE_T
      #else
         #define ssize_t long
      #endif
   #endif

#else
   #include <stdlib.h>
   #include <inttypes.h>   
   #include <cstring>
   #include <stdint.h>

   #ifndef PAGESIZE
      #include <unistd.h>
      #define PAGESIZE sysconf(_SC_PAGESIZE)
   #endif

   #ifndef PAGEFLOOR
      // "Round" a ptr down to the beginning of the memory page containing it
      // PAGERANGE gives us a size to lock/map as a multiple of the PAGESIZE
      #define PAGEFLOOR(ptr,sz) ((void*)(((size_t)(ptr)) & (~(PAGESIZE-1))  ))
      #define PAGERANGE(ptr,sz) (  (((size_t)(ptr)+(sz)-1) | (PAGESIZE-1)) + 1 - PAGEFLOOR(ptr,sz)  )
   #endif
#endif


#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <atomic>

// We can remove these includes (Crypto++ ) if we remove the GenerateRandom()
#include "log.h"

#define DEFAULT_BUFFER_SIZE 32*1048576

#include "UniversalTimer.h"

#define READHEX        BinaryData::CreateFromHex

#define READ_UINT8_LE  BinaryData::StrToIntLE<uint8_t>
#define READ_UINT16_LE BinaryData::StrToIntLE<uint16_t>
#define READ_UINT32_LE BinaryData::StrToIntLE<uint32_t>
#define READ_UINT64_LE BinaryData::StrToIntLE<uint64_t>

#define READ_UINT8_BE  BinaryData::StrToIntBE<uint8_t>
#define READ_UINT16_BE BinaryData::StrToIntBE<uint16_t>
#define READ_UINT32_BE BinaryData::StrToIntBE<uint32_t>
#define READ_UINT64_BE BinaryData::StrToIntBE<uint64_t>

#define READ_UINT8_HEX_LE(A)  (READ_UINT8_LE(READHEX(A)))
#define READ_UINT16_HEX_LE(A) (READ_UINT16_LE(READHEX(A)))
#define READ_UINT32_HEX_LE(A) (READ_UINT32_LE(READHEX(A)))
#define READ_UINT64_HEX_LE(A) (READ_UINT64_LE(READHEX(A)))

#define READ_UINT8_HEX_BE(A)  (READ_UINT8_BE(READHEX(A)))
#define READ_UINT16_HEX_BE(A) (READ_UINT16_BE(READHEX(A)))
#define READ_UINT32_HEX_BE(A) (READ_UINT32_BE(READHEX(A)))
#define READ_UINT64_HEX_BE(A) (READ_UINT64_BE(READHEX(A)))

#define WRITE_UINT8_LE  BinaryData::IntToStrLE<uint8_t>
#define WRITE_UINT16_LE BinaryData::IntToStrLE<uint16_t>
#define WRITE_UINT32_LE BinaryData::IntToStrLE<uint32_t>
#define WRITE_UINT64_LE BinaryData::IntToStrLE<uint64_t>

#define WRITE_UINT8_BE  BinaryData::IntToStrBE<uint8_t>
#define WRITE_UINT16_BE BinaryData::IntToStrBE<uint16_t>
#define WRITE_UINT32_BE BinaryData::IntToStrBE<uint32_t>
#define WRITE_UINT64_BE BinaryData::IntToStrBE<uint64_t>

enum ENDIAN 
{ 
   ENDIAN_LITTLE, 
   ENDIAN_BIG 
};

#define LE ENDIAN_LITTLE
#define BE ENDIAN_BIG

class BinaryDataRef;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BinaryData
{
public:


   /////////////////////////////////////////////////////////////////////////////
   BinaryData(void) : data_(0)                 {                         }
   explicit BinaryData(size_t sz)              { alloc(sz);              }
   BinaryData(uint8_t const * inData, size_t sz)
                                               { copyFrom(inData, sz);   }
   BinaryData(char const * inData, size_t sz)  { copyFrom(inData, sz);   }
   BinaryData(uint8_t const * dstart, uint8_t const * dend )
                                               { copyFrom(dstart, dend); }
   BinaryData(BinaryData const & bd)           { copyFrom(bd);           }

   BinaryData(BinaryData && copy) { data_ = move(copy.data_); }

   BinaryData(BinaryDataRef const & bdRef);
   size_t getSize(void) const { return data_.size(); }

   ~BinaryData(void)
   {
      data_.clear();
   }

   //bool isNull(void) const { return (data_.size()==0);}
   bool empty(void) const { return (data_.size()==0);}
   bool isZero(void) const;
   
   BinaryData& operator=(const BinaryData &o)
   {
      data_ = o.data_;
      return *this;
   }
   BinaryData& operator=(BinaryData &&o)
   {
      swap(data_, o.data_);
      return *this;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint8_t const * getPtr(void) const
   { 
      if(getSize()==0)
         return NULL;
      else
         return &(data_[0]); 
   }

   /////////////////////////////////////////////////////////////////////////////
   uint8_t* getPtr(void)
   { 
      if(getSize()==0)
         return NULL;
      else
         return &(data_[0]); 
   }  
   
   /////////////////////////////////////////////////////////////////////////////
   char const * getCharPtr(void) const
   { 
      if (getSize() == 0)
      {
         LOGERR << "Tried to get pointer of empty BinaryData";
         throw std::runtime_error("Tried to get pointer of empty BinaryData");
      }
      else
         return reinterpret_cast<const char*>(&data_[0]);
   }

   /////////////////////////////////////////////////////////////////////////////
   char* getCharPtr(void)
   { 
      if(getSize()==0)
      {
         LOGERR << "Tried to get pointer of empty BinaryData";
         throw std::runtime_error("Tried to get pointer of empty BinaryData");
      }
      else
         return reinterpret_cast<char*>(&data_[0]);
   }  

   /////////////////////////////////////////////////////////////////////////////
   const std::vector<uint8_t>& getDataVector(void) const
   {
      return data_;
   }

   BinaryDataRef getRef(void) const;
   //uint8_t const * getConstPtr(void) const  { return &(data_[0]); }
   
   /////////////////////////////////////////////////////////////////////////////
   // We allocate space as necesssary
   void copyFrom(uint8_t const * start, uint8_t const * end) 
                  { copyFrom( start, (end-start)); }  // [start, end)
   
   void copyFrom(const std::string& str)
   {
      copyFrom(str.c_str(), str.size());
   }

   void copyFrom(BinaryData const & bd)
                  { copyFrom( bd.getPtr(), bd.getSize() ); }
   void copyFrom(BinaryDataRef const & bdr);

   void copyFrom(char const * inData, size_t sz)
   {
      copyFrom((uint8_t*)inData, sz);
   }

   void copyFrom(uint8_t const * inData, size_t sz)
   { 
      if(inData==NULL || sz == 0)
         alloc(0);
      else
      {
         alloc(sz); 
         memcpy( &(data_[0]), inData, sz);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   // UNSAFE -- you don't know if outData holds enough space for this
   void copyTo(uint8_t* outData) const { memcpy( outData, &(data_[0]), getSize()); }
   void copyTo(uint8_t* outData, size_t sz) const { memcpy( outData, &(data_[0]), (size_t)sz); }
   void copyTo(uint8_t* outData, size_t offset, size_t sz) const { memcpy( outData, &(data_[offset]), (size_t)sz); }
   void copyTo(BinaryData & bd) const 
   {
      if (empty())
         return;

      bd.resize(getSize());
      memcpy(bd.getPtr(), getPtr(), getSize());
   }

   void fill(uint8_t ch) { if(getSize()>0) memset(getPtr(), ch, getSize()); }
               
   uint8_t & operator[](ssize_t i)       { return (i<0 ? data_[getSize()+i] : data_[i]); }
   uint8_t   operator[](ssize_t i) const { return (i<0 ? data_[getSize()+i] : data_[i]); } 

   /////////////////////////////////////////////////////////////////////////////
   friend std::ostream& operator<<(std::ostream& os, BinaryData const & bd)
   {
      os << bd.toHexStr();
      return os;
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryData operator+(BinaryData const & bd2) const
   {
      if (bd2.empty())
         return *this;

      BinaryData out(getSize() + bd2.getSize());

      if (!empty())
         memcpy(out.getPtr(), getPtr(), getSize());

      memcpy(out.getPtr()+getSize(), bd2.getPtr(), bd2.getSize());
      return out;
   }

   /////////////////////////////////////////////////////////////////////////////
   // This is about as efficient as we're going to get...
   BinaryData & append(BinaryData const & bd2)
   {
      if(bd2.getSize()==0) 
         return (*this);
   
      if(getSize()==0) 
         copyFrom(bd2.getPtr(), bd2.getSize());
      else
         data_.insert(data_.end(), bd2.data_.begin(), bd2.data_.end());
      return (*this);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryData & append(BinaryDataRef const & bd2);

   /////////////////////////////////////////////////////////////////////////////
   BinaryData & append(uint8_t const * str, size_t sz);

   /////////////////////////////////////////////////////////////////////////////
   BinaryData & append(uint8_t byte)
   {
      data_.insert(data_.end(), byte);
      return (*this);
   }


   /////////////////////////////////////////////////////////////////////////////
   int32_t find(BinaryDataRef const & matchStr, uint32_t startPos=0);
   /////////////////////////////////////////////////////////////////////////////
   int32_t find(BinaryData const & matchStr, uint32_t startPos=0);

   /////////////////////////////////////////////////////////////////////////////
   bool contains(BinaryDataRef const & matchStr, uint32_t startPos=0);
   /////////////////////////////////////////////////////////////////////////////
   bool contains(BinaryData const & matchStr, uint32_t startPos=0);


   /////////////////////////////////////////////////////////////////////////////
   bool startsWith(BinaryDataRef const & matchStr) const;
   /////////////////////////////////////////////////////////////////////////////
   bool startsWith(BinaryData const & matchStr) const;

   /////////////////////////////////////////////////////////////////////////////
   bool endsWith(BinaryDataRef const & matchStr) const;
   /////////////////////////////////////////////////////////////////////////////
   bool endsWith(BinaryData const & matchStr) const;

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef getSliceRef(ssize_t start_pos, size_t nChar) const;
   /////////////////////////////////////////////////////////////////////////////
   BinaryData    getSliceCopy(ssize_t start_pos, size_t nChar) const;

   /////////////////////////////////////////////////////////////////////////////
   bool operator<(BinaryData const & bd2) const;

   /////////////////////////////////////////////////////////////////////////////
   bool operator<(BinaryDataRef const & bd2) const;

   /////////////////////////////////////////////////////////////////////////////
   bool operator==(BinaryData const & bd2) const;

   /////////////////////////////////////////////////////////////////////////////
   bool operator!=(BinaryData const & bd2) const { return (!((*this)==bd2)); }

   /////////////////////////////////////////////////////////////////////////////
   bool operator==(BinaryDataRef const & bd2) const;

   /////////////////////////////////////////////////////////////////////////////
   bool operator!=(BinaryDataRef const & bd2) const { return (!((*this)==bd2)); }

   /////////////////////////////////////////////////////////////////////////////
   bool operator>(BinaryData const & bd2) const;
   
   /////////////////////////////////////////////////////////////////////////////
   bool operator>=(BinaryData const & bd2) const
   {
      return (*this > bd2 || *this == bd2);
   }


   /////////////////////////////////////////////////////////////////////////////
   // These are always memory-safe
   void copyTo(std::string & str)
   {
      if (empty())
         return;

      str.assign( (char const *)(&(data_[0])), getSize());
   }

   /////////////////////////////////////////////////////////////////////////////
   std::string toBinStr(bool bigEndian=false) const
   { 
      if(getSize()==0)
         return std::string("");

      if(bigEndian)
      {
         BinaryData out = copySwapEndian();
         return std::string((char const *)(out.getPtr()), getSize());
      }
      else
         return std::string((char const *)(getPtr()), getSize());
   }

   char* toCharPtr(void) const  { return  (char*)(&(data_[0])); }
   unsigned char* toUCharPtr(void) const { return (unsigned char*)(&(data_[0])); }

   void resize(size_t sz) { data_.resize(sz); }
   void reserve(size_t sz) { data_.reserve(sz); }

   /////////////////////////////////////////////////////////////////////////////
   // Swap endianness of the bytes in the index range [pos1, pos2)
   BinaryData& swapEndian(size_t pos1=0, size_t pos2=0)
   {
      if(getSize()==0)
         return (*this);

      if(pos2 <= pos1)
         pos2 = getSize();

      size_t totalBytes = pos2-pos1;
      for(size_t i=0; i<(totalBytes/2); i++)
      {
         uint8_t d1    = data_[pos1+i];
         data_[pos1+i] = data_[pos2-(i+1)];
         data_[pos2-(i+1)] = d1;
      }
      return (*this);
   }

   /////////////////////////////////////////////////////////////////////////////
   // Swap endianness of the bytes in the index range [pos1, pos2)
   BinaryData copySwapEndian(size_t pos1=0, size_t pos2=0) const
   {
      BinaryData bdout(*this);
      bdout.swapEndian(pos1, pos2);
      return bdout;
   }

   /////////////////////////////////////////////////////////////////////////////
   std::string toHexStr(bool bigEndian=false) const
   {
      if(getSize()==0)
         return std::string("");

      static char hexLookupTable[16] = {'0','1','2','3',
                                        '4','5','6','7',
                                        '8','9','a','b',
                                        'c','d','e','f' };
      BinaryData bdToHex(*this);
      if(bigEndian)
         bdToHex.swapEndian();

      std::vector<int8_t> outStr(2*getSize());
      for( size_t i=0; i<getSize(); i++)
      {
         uint8_t nextByte = bdToHex.data_[i];
         outStr[2*i  ] = hexLookupTable[ (nextByte >> 4) & 0x0F ];
         outStr[2*i+1] = hexLookupTable[ (nextByte     ) & 0x0F ];
      }
         
      return std::string((char const *)(&(outStr[0])), 2*getSize());
   }

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData CreateFromHex(std::string const & str)
   {
      BinaryData out;
      out.createFromHex(str);
      return out;
   }


   /////////////////////////////////////////////////////////////////////////////
   // This is an architecture-agnostic way to serialize integers to little- or
   // big-endian.  Bit-shift & mod will always return the lowest significant
   // bytes, so we can put them into an array of bytes in the desired order.
   template<typename INTTYPE>
   static BinaryData IntToStrLE(INTTYPE val)
   {
      static const uint8_t SZ = sizeof(INTTYPE);
      BinaryData out(SZ);
      for(uint8_t i=0; i<SZ; i++, val>>=8)
         out[i] = val % 256;
      return out;
   }
   
   /////////////////////////////////////////////////////////////////////////////
   template<typename INTTYPE>
   inline static BinaryData IntToStrBE(INTTYPE val)
   {
      static const uint8_t SZ = sizeof(INTTYPE);
      BinaryData out(SZ);
      for(uint8_t i=0; i<SZ; i++, val>>=8)
         out[(SZ-1)-i] = val % 256;
      return out;
   }

   /////////////////////////////////////////////////////////////////////////////
   template<typename INTTYPE>
   static INTTYPE StrToIntLE(BinaryData binstr)
   {
      uint8_t const SZ = sizeof(INTTYPE);
      if(binstr.getSize() != SZ)
      {
         LOGERR << "StrToInt: strsz: " << binstr.getSize() << " intsz: " << SZ;
         return (INTTYPE)0;
      }
      
      /*INTTYPE out = 0;
      for(uint8_t i=0; i<SZ; i++)
         out |= ((INTTYPE)binstr[i]) << (8*i);*/

      auto intPtr = (INTTYPE*)binstr.getPtr();

      return *intPtr;
   }

   /////////////////////////////////////////////////////////////////////////////
   template<typename INTTYPE>
   static INTTYPE StrToIntBE(BinaryData binstr)
   {
      uint8_t const SZ = sizeof(INTTYPE);
      if(binstr.getSize() != SZ)
      {
         LOGERR << "StrToInt: strsz: " << binstr.getSize() << " intsz: " << SZ;
         return (INTTYPE)0;
      }
      
      INTTYPE out = 0;
      for(uint8_t i=0; i<SZ; i++)
         out |= ((INTTYPE)binstr[i]) << (8*((SZ-1)-i));

      return out;
   }

   /////////////////////////////////////////////////////////////////////////////
   template<typename INTTYPE>
   static INTTYPE StrToIntLE(uint8_t const * ptr)
   {
      //return  *((INTTYPE*)ptr);
      //the kind of typecasts are undefined behavior, use memcpy 
      //instead, TBAA will optimize it away

      INTTYPE result;
      memcpy(&result, ptr, sizeof(INTTYPE));
      return result;
   }

   /////////////////////////////////////////////////////////////////////////////
   template<typename INTTYPE>
   static INTTYPE StrToIntBE(uint8_t const * ptr)
   {
      uint8_t const SZ = sizeof(INTTYPE);

      INTTYPE out = 0;
      for(uint8_t i=0; i<SZ; i++)
         out |= ((INTTYPE)ptr[i]) << (8*((SZ-1)-i));

      return out;
   }

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData fromString(const std::string& str, size_t len = SIZE_MAX)
   {
      if (len == SIZE_MAX)
         len = str.size();
         
      BinaryData data;
      data.copyFrom(str.c_str(), len);
      return data;
   }

   /////////////////////////////////////////////////////////////////////////////
   void createFromHex(const std::string& str);
   void createFromHex(BinaryDataRef const & bdr);

   // For deallocating all the memory that is currently used by this BD
   void clear(void) { data_.clear(); }
   std::vector<uint8_t> release(void)
   { 
      auto vec = move(data_);
      clear();
      return vec; 
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::vector<uint8_t>& getVector(void) const
   {
      return data_;
   }

protected:
   std::vector<uint8_t> data_;

private:
   void alloc(size_t sz)
   { 
      if(sz != getSize())
      {
         data_.clear();
         data_.resize(sz);
      }
   }
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BinaryDataRef
{
public:
   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef(void) : ptr_(NULL), nBytes_(0)
   {
      // Nothing to put here
   }
   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef(uint8_t const * inData, size_t sz)
   { 
      setRef(inData, sz);
   }
   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef(uint8_t const * dstart, uint8_t const * dend)
   { 
      setRef(dstart,dend);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef(BinaryDataRef const & bdr)
   { 
      ptr_ = bdr.ptr_;
      nBytes_ = bdr.nBytes_;
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef(BinaryData const & bd)
   { 
      if(bd.getSize()!=0) 
      {
         ptr_ = bd.getPtr();
         nBytes_ = bd.getSize();
      }
      else
      {
         ptr_= NULL;
         nBytes_ = 0;
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void reset(void)
   {
      ptr_ = nullptr;
      nBytes_ = 0;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint8_t const * getPtr(void) const       { return ptr_;    }
   size_t getSize(void) const               { return nBytes_; }
   bool empty(void) const { return nBytes_ == 0; }

   /////////////////////////////////////////////////////////////////////////////
   void setRef(uint8_t const * inData, size_t sz)
   {
      ptr_ = inData;
      nBytes_ = sz;
   }
   void setRef(uint8_t const * start, uint8_t const * end)
                  { setRef( start, (end-start)); }  // [start, end)
   void setRef(std::string const & str)
                  { setRef( (uint8_t*)str.data(), str.size()); }
   void setRef(BinaryData const & bd)
                  { setRef( bd.getPtr(), bd.getSize() ); }

   static BinaryDataRef fromString(const std::string& str, size_t len=SIZE_MAX)
   {
      if (len == SIZE_MAX)
         len = str.size();
         
      BinaryDataRef data;
      data.setRef((const uint8_t*)str.c_str(), len);
      return data;
   }

   /////////////////////////////////////////////////////////////////////////////
   // UNSAFE -- you don't know if outData holds enough space for this
   void copyTo(uint8_t* outData) const { memcpy( outData, ptr_, (size_t)nBytes_); }
   void copyTo(uint8_t* outData, size_t sz) const { memcpy( outData, ptr_, (size_t)sz); }
   void copyTo(uint8_t* outData, size_t offset, size_t sz) const 
                                    { memcpy( outData, ptr_+offset, (size_t)sz); }
   void copyTo(BinaryData & bd) const
   {
      if (empty())
         return;

      bd.resize(nBytes_);
      memcpy( bd.getPtr(), ptr_, (size_t)nBytes_);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryData copy(void) const 
   {
      BinaryData outData(nBytes_);
      copyTo(outData);
      return outData;
   }

   /////////////////////////////////////////////////////////////////////////////
   // These are always memory-safe
   void copyTo(std::string & str) { str.assign( (char const *)(ptr_), nBytes_); }

   /////////////////////////////////////////////////////////////////////////////
   friend std::ostream& operator<<(std::ostream& os, BinaryDataRef const & bd)
   {
      os << bd.toHexStr();
      return os;
   }

   /////////////////////////////////////////////////////////////////////////////
   std::string toBinStr(bool bigEndian=false) const
   { 
      if(getSize()==0)
         return std::string("");

      if(bigEndian)
      {
         BinaryData out = copy();
         return std::string((char const *)(out.swapEndian().getPtr()), nBytes_);
      }
      else
         return std::string((char const *)(ptr_), nBytes_);
   }

   
   /////////////////////////////////////////////////////////////////////////////
   char* toCharPtr(void) const  { return  (char*)(ptr_); }
   unsigned char* toUCharPtr(void) const { return (unsigned char*)(ptr_); }

   /////////////////////////////////////////////////////////////////////////////
   uint8_t const & operator[](ssize_t i) const { return (i<0 ? ptr_[nBytes_+i] : ptr_[i]); }
   bool isValid(void) const { return ptr_ != NULL; }

   /////////////////////////////////////////////////////////////////////////////
   int32_t find(BinaryDataRef const & matchStr, uint32_t startPos=0)
   {
      int32_t finalAnswer = -1;
      if(matchStr.getSize()==0)
         return startPos;

      for(int32_t i=startPos; i<=(int32_t)nBytes_-(int32_t)matchStr.nBytes_; i++)
      {
         if(matchStr.ptr_[0] != ptr_[i])
            continue;

         for(uint32_t j=0; j<matchStr.nBytes_; j++)
         {
            if(matchStr.ptr_[j] != ptr_[i+j])
               break;

            // If we are at this instruction and is the last index, it's a match
            if(j==matchStr.nBytes_-1)
               finalAnswer = i;
         }

         if(finalAnswer != -1)
            break;
      }

      return finalAnswer;
   }

   /////////////////////////////////////////////////////////////////////////////
   int32_t find(BinaryData const & matchStr, uint32_t startPos=0)
   {
      BinaryDataRef bdr(matchStr);
      return find(bdr, startPos);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool contains(BinaryDataRef const & matchStr, uint32_t startPos=0)
   {
      return (find(matchStr, startPos) != -1);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool contains(BinaryData const & matchStr, uint32_t startPos=0)
   {
      BinaryDataRef bdr(matchStr);
      return (find(bdr, startPos) != -1);
   }


   /////////////////////////////////////////////////////////////////////////////
   bool startsWith(BinaryDataRef const &) const;
   bool startsWith(BinaryData const &) const;

   /////////////////////////////////////////////////////////////////////////////
   bool endsWith(BinaryDataRef const & matchStr) const
   {
      size_t sz = matchStr.getSize();
      if(sz > nBytes_)
         return false;
   
      for(size_t i=0; i<sz; i++)
         if(matchStr[sz-(i+1)] != (*this)[nBytes_-(i+1)])
            return false;
   
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool endsWith(BinaryData const & matchStr) const
   {
      size_t sz = matchStr.getSize();
      if(sz > nBytes_)
         return false;
   
      for(size_t i=0; i<sz; i++)
         if(matchStr[sz-(i+1)] != (*this)[nBytes_-(i+1)])
            return false;
   
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef getSliceRef(ssize_t start_pos, size_t nChar) const
   {
      if(start_pos < 0) 
         start_pos = nBytes_ + start_pos;

      if(start_pos + nChar > nBytes_)
      {
         std::cerr << "getSliceRef: Invalid BinaryData access" << std::endl;
         return BinaryDataRef();
      }
      return BinaryDataRef( getPtr()+start_pos, nChar);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryData getSliceCopy(ssize_t start_pos, size_t nChar) const
   {
      if(start_pos < 0) 
         start_pos = nBytes_ + start_pos;

      if(start_pos + nChar > nBytes_)
      {
         std::cerr << "getSliceCopy: Invalid BinaryData access" << std::endl;
         return BinaryDataRef();
      }
      return BinaryData( getPtr()+start_pos, nChar);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool isSameRefAs(BinaryDataRef const & bdRef2)
   {
      return (ptr_ == bdRef2.ptr_ && nBytes_ == bdRef2.nBytes_);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef& operator=(const BinaryDataRef&);

   /////////////////////////////////////////////////////////////////////////////
   bool operator<(BinaryDataRef const & bd2) const;

   /////////////////////////////////////////////////////////////////////////////
   bool operator==(BinaryDataRef const & bd2) const
   {
      if(nBytes_ != bd2.nBytes_)
         return false;
      else if(ptr_ == bd2.ptr_)
         return true;
      
      return (memcmp(getPtr(), bd2.getPtr(), getSize()) == 0);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool operator==(BinaryData const & bd2) const
   {
      if(nBytes_ != bd2.getSize())
         return false;
      else if(ptr_ == bd2.getPtr())
         return true;

      return (memcmp(getPtr(), bd2.getPtr(), getSize()) == 0);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool operator!=(BinaryDataRef const & bd2) const { return !((*this)==bd2); }
   bool operator!=(BinaryData    const & bd2) const { return !((*this)==bd2); }


   /////////////////////////////////////////////////////////////////////////////
   bool operator>(BinaryDataRef const & bd2) const;

   /////////////////////////////////////////////////////////////////////////////
   std::string toHexStr(bool bigEndian=false) const
   {
      if(getSize() == 0)
         return std::string("");

      static char hexLookupTable[16] = {'0','1','2','3',
                                        '4','5','6','7',
                                        '8','9','a','b',
                                        'c','d','e','f' };
      BinaryData bdToHex(*this);
      if(bigEndian)
         bdToHex.swapEndian();

      std::vector<int8_t> outStr(2*nBytes_);
      for(size_t i=0; i<nBytes_; i++)
      {
         uint8_t nextByte = *(bdToHex.getPtr()+i);
         outStr[2*i  ] = hexLookupTable[ (nextByte >> 4) & 0x0F ];
         outStr[2*i+1] = hexLookupTable[ (nextByte     ) & 0x0F ];
      }
      return std::string((char const *)(&(outStr[0])), 2*nBytes_);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool isZero(void) const
   {
      for (unsigned i=0; i<getSize(); i++)
      {
         if (ptr_[i] != 0)
            return false;
      }

      return true;
   }

private:
   uint8_t const * ptr_;
   size_t  nBytes_;

private:

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BinaryReader
{
public:
   /////////////////////////////////////////////////////////////////////////////
   BinaryReader(int sz=0) :
      bdStr_(sz),
      pos_(0)
   {
      // Nothing needed here
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryReader(BinaryData const & toRead) 
   {
      setNewData(toRead);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryReader(uint8_t* ptr, uint32_t nBytes)
   {
      setNewData(ptr, nBytes);
   }

   /////////////////////////////////////////////////////////////////////////////
   void setNewData(BinaryData const & toRead)
   {
      bdStr_ = toRead;
      pos_ = 0;
   }

   /////////////////////////////////////////////////////////////////////////////
   void setNewData(uint8_t* ptr, uint32_t nBytes)
   {
      bdStr_ = BinaryData(ptr, nBytes);
      pos_ = 0;
   }

   /////////////////////////////////////////////////////////////////////////////
   void advance(uint32_t nBytes);

   /////////////////////////////////////////////////////////////////////////////
   void rewind(size_t nBytes);

   /////////////////////////////////////////////////////////////////////////////
   void resize(size_t nBytes);

   /////////////////////////////////////////////////////////////////////////////
   uint64_t get_var_int(uint8_t* nRead=NULL);


   /////////////////////////////////////////////////////////////////////////////
   uint8_t get_uint8_t(void)
   {
      uint8_t outVal = bdStr_[pos_];
      pos_ += 1;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint16_t get_uint16_t(ENDIAN e=LE)
   {
      uint16_t outVal = (e==LE ? READ_UINT16_LE(bdStr_.getPtr() + pos_) :
                                 READ_UINT16_BE(bdStr_.getPtr() + pos_));
      pos_ += 2;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint32_t get_uint32_t(ENDIAN e=LE)
   {
      uint32_t outVal = (e==LE ? READ_UINT32_LE(bdStr_.getPtr() + pos_) :
                                 READ_UINT32_BE(bdStr_.getPtr() + pos_));
      pos_ += 4;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   int32_t get_int32_t(ENDIAN e = LE)
   {
      int32_t outVal = (e == LE ? 
         BinaryData::StrToIntLE<int32_t>(bdStr_.getPtr() + pos_) :
         BinaryData::StrToIntBE<int32_t>(bdStr_.getPtr() + pos_));
      pos_ += 4;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint64_t get_uint64_t(ENDIAN e=LE)
   {
      uint64_t outVal = (e==LE ? READ_UINT64_LE(bdStr_.getPtr() + pos_) :
                                 READ_UINT64_BE(bdStr_.getPtr() + pos_));
      pos_ += 8;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   void get_BinaryData(BinaryData & bdTarget, uint32_t nBytes)
   {
      bdTarget.copyFrom( bdStr_.getPtr() + pos_, nBytes);
      pos_ += nBytes;
   }

   /////////////////////////////////////////////////////////////////////////////
   void get_BinaryData(uint8_t* targPtr, uint32_t nBytes)
   {
      bdStr_.copyTo(targPtr, pos_, nBytes);
      pos_ += nBytes;
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef get_BinaryDataRef(unsigned nBytes)
   {
      auto bdr = bdStr_.getSliceRef(pos_, nBytes);
      pos_ += nBytes;

      return bdr;
   }


   /////////////////////////////////////////////////////////////////////////////
   // Take the remaining buffer and shift it to the front
   // then return a pointer to where the old data ends
   //
   //                                      
   //  Before:                             pos
   //                                       |
   //                                       V
   //             [ a b c d e f g h i j k l m n o p q r s t]
   //
   //  After:      pos           return*
   //               |               |
   //               V               V
   //             [ m n o p q r s t - - - - - - - - - - - -]
   //                                 
   //
   std::pair<uint8_t*, size_t> rotateRemaining(void)
   {
      size_t nRemain = getSizeRemaining();
      //if(pos_ > nRemain+1)
         //memcpy(bdStr_.getPtr(), bdStr_.getPtr() + pos_, nRemain);
      //else
         memmove(bdStr_.getPtr(), bdStr_.getPtr() + pos_, nRemain);

      pos_ = 0;

      return std::make_pair(bdStr_.getPtr() + nRemain, getSize() - nRemain);
   }

   /////////////////////////////////////////////////////////////////////////////
   void     resetPosition(void)           { pos_ = 0; }
   size_t   getPosition(void) const       { return pos_; }
   size_t   getSize(void) const           { return bdStr_.getSize(); }
   size_t   getSizeRemaining(void) const  { return getSize() - pos_; }
   bool     isEndOfStream(void) const     { return pos_ >= getSize(); }
   uint8_t* exposeDataPtr(void)           { return bdStr_.getPtr(); }
   uint8_t const * getCurrPtr(void)       { return bdStr_.getPtr() + pos_; }

private:
   BinaryData bdStr_;
   size_t     pos_;

};

class SecureBinaryData;

class BinaryRefReader
{
public:
   /////////////////////////////////////////////////////////////////////////////
   BinaryRefReader(size_t sz=0) :
      bdRef_(),
      totalSize_(sz)
   {
      pos_.store(0, std::memory_order_relaxed);
   }

   BinaryRefReader& operator=(const BinaryRefReader& brr)
   {
      if(&brr == this)
         return *this;

      bdRef_ = brr.bdRef_;
      totalSize_ = brr.totalSize_;
      pos_.store(brr.pos_.load(std::memory_order_relaxed), std::memory_order_relaxed);

      return *this;
   }

   BinaryRefReader(const BinaryRefReader& brr)
   {
      bdRef_ = brr.bdRef_;
      totalSize_ = brr.totalSize_;
      pos_.store(brr.pos_.load(std::memory_order_relaxed), std::memory_order_relaxed);
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryRefReader(BinaryData const & toRead)  { setNewData(toRead); }
   BinaryRefReader(BinaryDataRef const & toRead)  { setNewData(toRead); }

   // Default to INF size -- leave it to the user to guarantee that he's
   // not reading past the end of rawPtr
   BinaryRefReader(uint8_t const * rawPtr, size_t nBytes=UINT32_MAX) 
   {
      setNewData(rawPtr, nBytes);
   }

   void setNewData(BinaryData const & toRead)
   {
      setNewData(toRead.getPtr(), toRead.getSize());
   }

   void setNewData(BinaryDataRef const & toRead)
   {
      setNewData(toRead.getPtr(), toRead.getSize());
   }

   void setNewData(uint8_t const * ptr, size_t nBytes=UINT32_MAX)
   {
      bdRef_ = BinaryDataRef(ptr, nBytes);
      totalSize_ = nBytes;
      pos_.store(0, std::memory_order_relaxed);
   }

   /////////////////////////////////////////////////////////////////////////////
   void advance(size_t nBytes);

   /////////////////////////////////////////////////////////////////////////////
   void rewind(uint32_t nBytes)
   {
      size_t start = pos_.load(std::memory_order_relaxed);
      pos_.fetch_sub(nBytes, std::memory_order_relaxed);
      if(pos_.load(std::memory_order_relaxed) > start)
         pos_.store(0, std::memory_order_relaxed);
   }

   /////////////////////////////////////////////////////////////////////////////
   uint64_t get_var_int(uint8_t* nRead=NULL);
   uint8_t get_uint8_t(void);
   uint16_t get_uint16_t(ENDIAN e=LE);
   uint32_t get_uint32_t(ENDIAN e=LE);
   int32_t get_int32_t(ENDIAN e = LE);
   uint64_t get_uint64_t(ENDIAN e=LE);
   int64_t get_int64_t(ENDIAN e = LE);
   double get_double(void);
   BinaryDataRef get_BinaryDataRef(uint32_t);
   BinaryRefReader fork(void) const;
   void get_BinaryData(BinaryData&, uint32_t);
   BinaryData get_BinaryData(uint32_t);
   SecureBinaryData get_SecureBinaryData(uint32_t);
   void get_BinaryData(uint8_t*, uint32_t);
   std::string get_String(uint32_t);

   /////////////////////////////////////////////////////////////////////////////
   void resetPosition(void);
   size_t getPosition(void) const;
   size_t getSize(void) const;
   size_t getSizeRemaining(void) const;
   bool isEndOfStream(void) const;
   uint8_t const* exposeDataPtr(void);
   uint8_t const* getCurrPtr(void);

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef getRawRef(void);

private:
   BinaryDataRef bdRef_;
   size_t totalSize_;

   /*
   On at least AMD Ryzen CPUs, gcc O1/2 compilation has demonstrated that reset
   and advance operations can result in out of order execution on pos_ leading to
   unexpected offset position, when pos_ is a simple size_t.

   Upgrading pos_ to either volatile or atomic<size_t> enforces the sequential
   execution of operations on pos_, fixing the issue.

   Since the only desirable additional feature is sequentiality, relaxed atomic
   operations were prefered to volatile, as they are generally cheaper at least
   on Windows (where volatiles come with acq_rel semantics by default).
   */
   std::atomic<size_t> pos_;
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// This is only intended to be used for the four datatypes:
//    uint8_t, uint16_t, uint32_t, uint64_t
// Simplicity is what makes this so useful 
template<typename DTYPE>
class BitPacker
{
public:
   BitPacker(void) : intVal_(0), bitsUsed_(0) {}


   void putBits(DTYPE val, uint32_t bitWidth)
   {
      uint8_t const SZ = sizeof(DTYPE);
      if(bitsUsed_ + bitWidth > SZ*8)
         LOGERR << "Tried to put bits beyond end of bit field";

      if(bitsUsed_==0 && bitWidth==SZ*8)
      {
         bitsUsed_ = SZ*8;
         intVal_ = val;
         return;
      }

      uint32_t shiftAmt = SZ*8 - (bitsUsed_ + bitWidth);
      DTYPE mask = (DTYPE)((1ULL<<bitWidth) - 1);
      intVal_ |= (val & mask) << shiftAmt;
      bitsUsed_ += bitWidth;
   }

   void putBit(bool val)
   {
      DTYPE bit = (val ? 1 : 0);   
      putBits(bit, 1);
   }

   uint32_t getBitsUsed(void) {return bitsUsed_;}

   BinaryData getBinaryData(void) 
               { return BinaryData::IntToStrBE<DTYPE>(intVal_); }

   // Disabling this to avoid inadvertantly using it to write out 
   // data in the wrong endianness.  (instead, always use getBinaryData
   // or writeToStream
   //DTYPE getValue(void)      { return intVal_; }
   
   void  reset(void)         { intVal_ = 0; bitsUsed_ = 0; }

private:
   DTYPE    intVal_; 
   uint32_t bitsUsed_;

};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// This is only intended to be used for the four datatypes:
//    uint8_t, uint16_t, uint32_t, uint64_t
// Simplicity is what makes this so useful 
template<typename DTYPE>
class BitUnpacker
{
public:
   BitUnpacker(void) {bitsRead_=0xffffffff;}
   BitUnpacker(DTYPE valToRead) {setValue(valToRead);}
   BitUnpacker(BinaryRefReader & brr)
   {
      BinaryData bytes = brr.get_BinaryData(sizeof(DTYPE));
      setValue( BinaryData::StrToIntBE<DTYPE>(bytes) );
   }

   void setValue(DTYPE val)   { intVal_ = val; bitsRead_ = 0; }

   DTYPE getBits(uint32_t bitWidth)
   {
      uint8_t const SZ = sizeof(DTYPE);
      if(bitsRead_==0 && bitWidth==SZ*8)
      {
         bitsRead_ = bitWidth;
         return intVal_;
      }
      uint32_t shiftAmt = SZ*8 - (bitsRead_ + bitWidth);
      DTYPE mask = (DTYPE)((1ULL<<bitWidth) - 1);
      bitsRead_ += bitWidth;
      return ((intVal_ >> shiftAmt) & mask);
   }

   bool getBit(void)
   {
      return (getBits(1) > 0);
   }

   void reset(void) { intVal_ = 0; bitsRead_ = 0; }

private:
   DTYPE    intVal_; 
   uint32_t bitsRead_;

};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BinaryWriter
{
public:
   /////////////////////////////////////////////////////////////////////////////
   // Using the argument to pre-allocate a certain amount of capacity.  Not 
   // required, but will improve performance if you can take a reasonable guess
   // about the final size of the output data
   BinaryWriter(size_t reserveSize=0) :
      theString_(0)
   {
      if(reserveSize != 0)
         theString_.reserve(reserveSize);
   }

   /////////////////////////////////////////////////////////////////////////////
   void reserve(size_t sz) { theString_.reserve(sz); }


   /////////////////////////////////////////////////////////////////////////////
   // These write data properly regardless of the architecture
   void put_uint8_t (const uint8_t&  val) { theString_.append( val ); }

   /////
   void put_uint16_t(const uint16_t& val, ENDIAN e=LE) 
   { 
      if (e == LE)
      {
         auto valPtr = (uint8_t*)&val;
         theString_.append(valPtr, 2);
      }
      else
      {
         auto&& out = WRITE_UINT16_BE(val);
         theString_.append(out.getPtr(), 2);
      }
   }

   /////
   void put_uint32_t(const uint32_t& val, ENDIAN e=LE) 
   { 
      if (e == LE)
      {
         auto valPtr = (uint8_t*)&val;
         theString_.append(valPtr, 4);
      }
      else
      {
         auto&& out = WRITE_UINT32_BE(val);
         theString_.append(out.getPtr(), 4);
      }
   }

   /////
   void put_int32_t(const int32_t& val, ENDIAN e = LE)
   {
      if (e == LE)
      {
         auto valPtr = (uint8_t*)&val;
         theString_.append(valPtr, 4);
      }
      else
      {
         auto&& out = BinaryData::IntToStrBE<int32_t>(val);
         theString_.append(out.getPtr(), 4);
      }
   }

   /////
   void put_uint64_t(const uint64_t& val, ENDIAN e=LE) 
   { 
      if (e == LE)
      {
         auto valPtr = (uint8_t*)&val;
         theString_.append(valPtr, 8);
      }
      else
      {
         auto&& out = WRITE_UINT64_BE(val);
         theString_.append(out.getPtr(), 8);
      }
   }

   ////
   void put_double(const double& val)
   {
      auto valPtr = (uint8_t*)&val;
      theString_.append(valPtr, 8);
   }

   /////////////////////////////////////////////////////////////////////////////
   uint8_t put_var_int(const uint64_t& val)
   {

      if(val < 0xfd)
      {
         put_uint8_t((uint8_t)val);
         return 1;
      }
      else if(val <= UINT16_MAX)
      {
         put_uint8_t(0xfd);
         put_uint16_t((uint16_t)val);
         return 3;
      }
      else if(val <= UINT32_MAX)
      {
         put_uint8_t(0xfe);
         put_uint32_t((uint32_t)val);
         return 5;
      }
      else 
      {
         put_uint8_t(0xff);
         put_uint64_t(val);
         return 9;
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void put_BinaryData(BinaryData const & str, size_t offset=0, uint32_t sz=0)
   {
      if(offset==0)
      {
         if(sz==0)
            theString_.append(str);
         else
            theString_.append(str.getPtr(), sz);
      }
      else
      {
         if(sz==0)
            theString_.append(str.getPtr() + offset, str.getSize() - offset);
         else
            theString_.append(str.getPtr() + offset, sz);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void put_BinaryDataRef(BinaryDataRef const & str)
   {
      theString_.append(str);
   }


   /////////////////////////////////////////////////////////////////////////////
   void put_BinaryData(uint8_t const * targPtr, uint32_t nBytes)
   {
      theString_.append(targPtr, nBytes);
   }

   /////////////////////////////////////////////////////////////////////////////
   void put_String(const std::string& str)
   {
      theString_.append((const uint8_t*)str.c_str(), str.size());
   }

   /////////////////////////////////////////////////////////////////////////////
   template<typename T>
   void put_BitPacker(BitPacker<T> & bp) { put_BinaryData(bp.getBinaryData()); }

   /////////////////////////////////////////////////////////////////////////////
   BinaryData const & getData(void) const
   {
      return theString_;
   }

   /////////////////////////////////////////////////////////////////////////////
   size_t getSize(void) const
   {
      return theString_.getSize();
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryDataRef getDataRef(void) const
   {
      return theString_.getRef();
   }

   /////////////////////////////////////////////////////////////////////////////
   std::string toString(void) const
   {
      return theString_.toBinStr();
   }

   /////////////////////////////////////////////////////////////////////////////
   std::string toHex(void) const
   {
      return theString_.toHexStr();
   }

   
   /////////////////////////////////////////////////////////////////////////////
   void reset(void)
   {
      theString_.resize(0);
   }

private:
   BinaryData theString_;


};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BinaryStreamBuffer
{

public:

   /////////////////////////////////////////////////////////////////////////////
   BinaryStreamBuffer(std::string filename="", uint32_t bufSize=DEFAULT_BUFFER_SIZE) :
      binReader_(bufSize),
      streamPtr_(NULL),
      weOwnTheStream_(false),
      bufferSize_(bufSize),
      fileBytesRemaining_(0)
   {
      if( filename.size() > 0 )
      {
         streamPtr_ = new std::ifstream;
         weOwnTheStream_ = true;
         std::ifstream* ifstreamPtr = static_cast<std::ifstream*>(streamPtr_);
         ifstreamPtr->open(OS_TranslatePath(filename.c_str()), std::ios::in | std::ios::binary);
         if( !ifstreamPtr->is_open() )
         {
            std::cerr << "Could not open file for reading!  File: " << filename.c_str() << std::endl;
            std::cerr << "Aborting!" << std::endl;
            throw std::runtime_error("failed to open file");
         }

         ifstreamPtr->seekg(0, std::ios::end);
         totalStreamSize_  = (uint32_t)ifstreamPtr->tellg();
         fileBytesRemaining_ = totalStreamSize_;
         ifstreamPtr->seekg(0, std::ios::beg);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void attachAsStreamBuffer(std::istream & is,
                             uint32_t streamSize,
                             uint32_t bufSz=DEFAULT_BUFFER_SIZE)
   {
      if(streamPtr_ != NULL && weOwnTheStream_)
      {
         static_cast<std::ifstream*>(streamPtr_)->close();
         delete streamPtr_;
      }

      streamPtr_           = &is;
      fileBytesRemaining_  = streamSize;
      totalStreamSize_     = streamSize;
      bufferSize_          = bufSz;
      binReader_.resize(bufferSize_);
   }


   
   /////////////////////////////////////////////////////////////////////////////
   // Refills the buffer from the stream, returns true if there is more data 
   // left in the stream
   bool streamPull(void)
   {
      SCOPED_TIMER("StreamPull");

      size_t prevBufSizeRemain = binReader_.getSizeRemaining();
      if(fileBytesRemaining_ == 0)
         return false;

      if( binReader_.getPosition() <= 0)
      {
         // No data to shuffle, just pull from the stream buffer
         if(fileBytesRemaining_ > binReader_.getSize())
         {
            // Enough left in the stream to fill the entire buffer
            streamPtr_->read((char*)(binReader_.exposeDataPtr()), binReader_.getSize());
            fileBytesRemaining_ -= binReader_.getSize();
         }
         else
         {
            // The buffer is bigger than the remaining stream size
            streamPtr_->read((char*)(binReader_.exposeDataPtr()), fileBytesRemaining_);
            binReader_.resize(fileBytesRemaining_);
            fileBytesRemaining_ = 0;
         }
         
      }
      else
      {
         // The buffer needs to be refilled but has leftover data at the end
         std::pair<uint8_t*, size_t> leftover = binReader_.rotateRemaining();
         uint8_t* putNewDataPtr = leftover.first;
         size_t numBytes        = leftover.second;

         if(fileBytesRemaining_ > numBytes)
         {
            // Enough data left in the stream to fill the entire buffer
            streamPtr_->read((char*)putNewDataPtr, numBytes);
            fileBytesRemaining_ -= numBytes;
         }
         else
         {
            // The buffer is bigger than the remaining stream size
            streamPtr_->read((char*)putNewDataPtr, fileBytesRemaining_);
            binReader_.resize(fileBytesRemaining_+ prevBufSizeRemain); 
            fileBytesRemaining_ = 0;
         }
      }

      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   BinaryReader& reader(void)
   {
      return binReader_;
   }

   /////////////////////////////////////////////////////////////////////////////
   size_t getFileByteLocation(void)
   {
      return totalStreamSize_ - (fileBytesRemaining_ + binReader_.getSizeRemaining());
   }


   size_t getBufferSizeRemaining(void) { return binReader_.getSizeRemaining(); }
   size_t getFileSizeRemaining(void)   { return fileBytesRemaining_; }
   size_t getBufferSize(void)          { return binReader_.getSize(); }

private:

   BinaryReader binReader_;
   std::istream* streamPtr_;
   bool     weOwnTheStream_;
   size_t   bufferSize_;
   size_t   totalStreamSize_;
   size_t   fileBytesRemaining_;

};

namespace std
{
   template<> struct hash<BinaryData>
   {
      std::size_t operator()(const BinaryData&) const;
   };

   template<> struct hash<BinaryDataRef>
   {
      std::size_t operator()(const BinaryDataRef&) const;
   };
};


#endif
