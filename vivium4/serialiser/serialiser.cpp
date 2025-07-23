#include "serialiser.h"

namespace Vivium {
	void SerialiserFileInterface::begin(std::string fileLocation, bool readMode)
	{
		auto flags = std::ios::binary | (readMode ? (std::ios::in) : (std::ios::app | std::ios::out));

		file.open(fileLocation, flags);

		if (!file.is_open()) {
			VIVIUM_LOG(LogSeverity::ERROR, "Couldnt't find file location {}", fileLocation);
		}
	}

	void SerialiserFileInterface::writeBytes(uint64_t length, void const* data)
	{
		file.write(reinterpret_cast<char const*>(data), length);
	}

	void SerialiserFileInterface::readBytes(uint64_t length, void* data)
	{
		file.read(reinterpret_cast<char*>(data), length);
	}

	void SerialiserFileInterface::end()
	{
		file.close();
	}

	void SerialiserMemoryInterface::writeBytes(uint64_t length, void const* data) {}
	void SerialiserMemoryInterface::readBytes(uint64_t length, void* data) {}
}