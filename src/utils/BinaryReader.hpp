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
 * @file BinaryReader.hpp
 * @brief Utility class for safe binary file reading
 * @author Heimdall Development Team
 * @date 2025-07-29
 *
 * This file defines the BinaryReader class that provides safe binary file
 * reading operations with support for different data types, endianness,
 * and error handling.
 */

#pragma once

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace heimdall
{

/**
 * @brief Endianness enumeration
 */
enum class Endianness
{
   Little,  // Little-endian (x86, x86-64)
   Big,     // Big-endian (PowerPC, some ARM)
   Native   // System native endianness
};

/**
 * @brief Utility class for safe binary file reading
 *
 * This class provides a safe and efficient way to read binary data from files
 * with support for:
 * - Different data types (8, 16, 32, 64-bit integers, floats, doubles)
 * - Endianness conversion
 * - Bounds checking
 * - Error handling
 * - Memory-mapped file support (future enhancement)
 */
class BinaryReader
{
   public:
   /**
    * @brief Constructor
    *
    * @param filePath Path to the binary file to read
    * @param endianness Endianness for reading multi-byte values
    */
   explicit BinaryReader(const std::string& filePath, Endianness endianness = Endianness::Native);

   /**
    * @brief Destructor
    */
   ~BinaryReader();

   /**
    * @brief Check if the file is open and readable
    *
    * @return true if file is open and readable
    * @return false otherwise
    */
   bool isOpen() const;

   /**
    * @brief Get the current file position
    *
    * @return Current position in bytes from start of file
    */
   std::streampos getPosition() const;

   /**
    * @brief Get the total file size
    *
    * @return File size in bytes
    */
   std::streamsize getFileSize() const;

   /**
    * @brief Seek to a specific position in the file
    *
    * @param position Position to seek to (in bytes from start)
    * @return true if seek was successful
    * @return false if seek failed
    */
   bool seek(std::streampos position);

   /**
    * @brief Seek relative to current position
    *
    * @param offset Offset from current position (can be negative)
    * @return true if seek was successful
    * @return false if seek failed
    */
   bool seekRelative(std::streamoff offset);

   /**
    * @brief Read 8-bit unsigned integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readU8(uint8_t& value);

   /**
    * @brief Read 16-bit unsigned integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readU16(uint16_t& value);

   /**
    * @brief Read 32-bit unsigned integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readU32(uint32_t& value);

   /**
    * @brief Read 64-bit unsigned integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readU64(uint64_t& value);

   /**
    * @brief Read 8-bit signed integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readS8(int8_t& value);

   /**
    * @brief Read 16-bit signed integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readS16(int16_t& value);

   /**
    * @brief Read 32-bit signed integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readS32(int32_t& value);

   /**
    * @brief Read 64-bit signed integer
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readS64(int64_t& value);

   /**
    * @brief Read 32-bit float
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readFloat(float& value);

   /**
    * @brief Read 64-bit double
    *
    * @param value Output value
    * @return true if read was successful
    * @return false if read failed
    */
   bool readDouble(double& value);

   /**
    * @brief Read raw bytes
    *
    * @param buffer Output buffer
    * @param size Number of bytes to read
    * @return true if read was successful
    * @return false if read failed
    */
   bool readBytes(void* buffer, std::streamsize size);

   /**
    * @brief Read string with specified length
    *
    * @param value Output string
    * @param length Number of characters to read
    * @return true if read was successful
    * @return false if read failed
    */
   bool readString(std::string& value, std::streamsize length);

   /**
    * @brief Read null-terminated string
    *
    * @param value Output string
    * @param maxLength Maximum number of characters to read
    * @return true if read was successful
    * @return false if read failed
    */
   bool readNullTerminatedString(std::string& value, std::streamsize maxLength = 1024);

   /**
    * @brief Read array of values
    *
    * @tparam T Type of values to read
    * @param values Output vector
    * @param count Number of values to read
    * @return true if read was successful
    * @return false if read failed
    */
   template <typename T>
   bool readArray(std::vector<T>& values, std::streamsize count)
   {
      values.resize(count);
      return readBytes(values.data(), count * sizeof(T));
   }

   /**
    * @brief Check if end of file has been reached
    *
    * @return true if at end of file
    * @return false otherwise
    */
   bool isEOF() const;

   /**
    * @brief Get the last error message
    *
    * @return Last error message
    */
   std::string getLastError() const;

   /**
    * @brief Clear the last error
    */
   void clearError();

   private:
   /**
    * @brief Convert endianness of a value
    *
    * @tparam T Type of value to convert
    * @param value Value to convert
    * @return Converted value
    */
   template <typename T>
   T convertEndianness(T value) const;

   /**
    * @brief Check if system is little-endian
    *
    * @return true if system is little-endian
    * @return false if system is big-endian
    */
   static bool                    isLittleEndian();

   std::unique_ptr<std::ifstream> file_;                  // File stream
   Endianness                     endianness_;            // Endianness for reading
   std::string                    lastError_;             // Last error message
   bool                           isLittleEndianSystem_;  // Cached system endianness
};

}  // namespace heimdall