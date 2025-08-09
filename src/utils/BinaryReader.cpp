/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file BinaryReader.cpp
 * @brief Implementation of BinaryReader utility class
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file implements the BinaryReader class that provides safe binary file
 * reading operations with support for different data types and endianness.
 */

#include "BinaryReader.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace heimdall
{

namespace
{

/**
 * @brief Byte swap function for endianness conversion
 *
 * @tparam T Type to swap
 * @param value Value to swap
 * @return Swapped value
 */
template <typename T>
T byteSwap(T value)
{
   uint8_t srcBytes[sizeof(T)];
   uint8_t dstBytes[sizeof(T)];
   
   // Copy value to byte array using memcpy (avoids type punning)
   std::memcpy(srcBytes, &value, sizeof(T));
   
   // Reverse byte order
   for (size_t i = 0; i < sizeof(T); ++i)
   {
      dstBytes[i] = srcBytes[sizeof(T) - 1 - i];
   }
   
   // Copy byte array back to value using memcpy
   T result;
   std::memcpy(&result, dstBytes, sizeof(T));
   return result;
}

}  // anonymous namespace

BinaryReader::BinaryReader(const std::string& filePath, Endianness endianness)
   : file_(new std::ifstream(filePath, std::ios::binary)),
     endianness_(endianness),
     lastError_(),
     isLittleEndianSystem_(isLittleEndian())
{
   if (!file_->is_open())
   {
      lastError_ = "Failed to open file: " + filePath;
   }
}

BinaryReader::~BinaryReader() = default;

bool BinaryReader::isOpen() const
{
   return file_ && file_->is_open();
}

std::streampos BinaryReader::getPosition() const
{
   if (!isOpen())
   {
      return std::streampos(-1);
   }
   return file_->tellg();
}

std::streamsize BinaryReader::getFileSize() const
{
   if (!isOpen())
   {
      return -1;
   }

   std::streampos currentPos = file_->tellg();
   file_->seekg(0, std::ios::end);
   std::streamsize size = file_->tellg();
   file_->seekg(currentPos);
   return size;
}

bool BinaryReader::seek(std::streampos position)
{
   if (!isOpen())
   {
      lastError_ = "File is not open";
      return false;
   }

   file_->seekg(position);
   if (file_->fail())
   {
      lastError_ = "Failed to seek to position";
      return false;
   }
   return true;
}

bool BinaryReader::seekRelative(std::streamoff offset)
{
   if (!isOpen())
   {
      lastError_ = "File is not open";
      return false;
   }

   file_->seekg(offset, std::ios::cur);
   if (file_->fail())
   {
      lastError_ = "Failed to seek relative to current position";
      return false;
   }
   return true;
}

bool BinaryReader::readU8(uint8_t& value)
{
   return readBytes(&value, sizeof(value));
}

bool BinaryReader::readU16(uint16_t& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readU32(uint32_t& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readU64(uint64_t& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readS8(int8_t& value)
{
   return readBytes(&value, sizeof(value));
}

bool BinaryReader::readS16(int16_t& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readS32(int32_t& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readS64(int64_t& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readFloat(float& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readDouble(double& value)
{
   if (!readBytes(&value, sizeof(value)))
   {
      return false;
   }

   if (endianness_ != Endianness::Native)
   {
      bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
      if (shouldSwap)
      {
         value = byteSwap(value);
      }
   }
   return true;
}

bool BinaryReader::readBytes(void* buffer, std::streamsize size)
{
   if (!isOpen())
   {
      lastError_ = "File is not open";
      return false;
   }

   if (!buffer)
   {
      lastError_ = "Invalid buffer pointer";
      return false;
   }

   file_->read(static_cast<char*>(buffer), size);
   if (file_->fail())
   {
      lastError_ = "Failed to read bytes from file";
      return false;
   }

   return true;
}

bool BinaryReader::readString(std::string& value, std::streamsize length)
{
   if (length < 0)
   {
      lastError_ = "Invalid string length";
      return false;
   }

   value.resize(static_cast<size_t>(length));
   return readBytes(&value[0], length);
}

bool BinaryReader::readNullTerminatedString(std::string& value, std::streamsize maxLength)
{
   if (!isOpen())
   {
      lastError_ = "File is not open";
      return false;
   }

   value.clear();
   char            ch;
   std::streamsize count = 0;

   while (count < maxLength)
   {
      if (!readBytes(&ch, 1))
      {
         break;
      }

      if (ch == '\0')
      {
         break;
      }

      value += ch;
      ++count;
   }

   return true;
}

bool BinaryReader::isEOF() const
{
   if (!isOpen())
   {
      return true;
   }
   return file_->eof();
}

std::string BinaryReader::getLastError() const
{
   return lastError_;
}

void BinaryReader::clearError()
{
   lastError_.clear();
}

template <typename T>
T BinaryReader::convertEndianness(T value) const
{
   if (endianness_ == Endianness::Native)
   {
      return value;
   }

   bool shouldSwap = (endianness_ == Endianness::Little) != isLittleEndianSystem_;
   if (shouldSwap)
   {
      return byteSwap(value);
   }
   return value;
}

bool BinaryReader::isLittleEndian()
{
   uint32_t testValue = 0x01020304;
   uint8_t  bytes[4];
   
   // Copy value to byte array using memcpy (avoids type punning)
   std::memcpy(bytes, &testValue, sizeof(testValue));
   
   return bytes[0] == 4;
}

}  // namespace heimdall