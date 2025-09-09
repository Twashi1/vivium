#include "serialiser.h"

namespace Vivium {
void SerialiserFileInterface::begin(std::string fileLocation, bool readMode) {
  auto flags = std::ios::binary |
               (readMode ? (std::ios::in) : (std::ios::trunc | std::ios::out));

  file.open(fileLocation, flags);

  if (!file.is_open()) {
    VIVIUM_LOG(LogSeverity::ERROR, "Couldnt't find file location {}",
               fileLocation);
  }

  countBytes = 0;
}

void SerialiserFileInterface::writeBytes(uint64_t length, void const* data) {
  file.write(reinterpret_cast<char const*>(data), length);
  countBytes += length;
}

void SerialiserFileInterface::readBytes(uint64_t length, void* data) {
  file.read(reinterpret_cast<char*>(data), length);
  countBytes += length;
}

void SerialiserFileInterface::end() { file.close(); }

void SerialiserMemoryInterface::begin(uint64_t capacity) {
  destination = new uint8_t[capacity];
  maxSize = capacity;
  offset = 0;
}

void SerialiserMemoryInterface::writeBytes(uint64_t length, void const* data) {
  if (offset + length > maxSize) {
    // Need to re-alloc
    void* newDestination = new uint8_t[maxSize + (maxSize / 2) + length];
    memcpy(newDestination, destination, offset);

    delete[] destination;
    destination = newDestination;
  }

  memcpy(reinterpret_cast<uint8_t*>(destination) + offset, data, length);
  offset += length;
}

void SerialiserMemoryInterface::readBytes(uint64_t length, void* data) {
  VIVIUM_ASSERT(offset + length <= maxSize, "Exceeded maximum read");

  memcpy(data, reinterpret_cast<uint8_t*>(destination) + offset, length);
  offset += length;
}

void SerialiserMemoryInterface::end() { delete[] destination; }
}  // namespace Vivium