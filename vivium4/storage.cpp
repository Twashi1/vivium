#include "storage.h"

namespace Vivium {
	namespace Storage {
		namespace Static {
			Pool::Pool()
				// Default block capacity
				: Pool(0x4000)
			{}

			Pool::Pool(uint64_t blockCapacity)
				: blockCapacity(blockCapacity)
			{}

			void* Pool::allocate(uint64_t bytes)
			{
				if (!blocks.empty()) {
					Block* last_block = &blocks.back();

					if (last_block->offset + bytes <= blockCapacity) {
						uint8_t* ptr = last_block->data + last_block->offset;

						last_block->offset += bytes;

						return ptr;
					}
				}

				blocks.emplace_back(
					new uint8_t[std::max(blockCapacity, bytes)], 
					bytes
				);

				return blocks.back().data;
			}

			void Pool::free()
			{
				for (Block block : blocks) {
					delete[] block.data;
				}
			}

			Pool::Block::Block(uint8_t* data, uint64_t offset)
				: data(data), offset(offset)
			{}
			
			Transient::Transient(uint64_t totalCapacity)
				: offset(0)
			{
				data = new uint8_t[totalCapacity];
			}

			void* Transient::allocate(uint64_t bytes)
			{
				void* allocation = data + offset;

				offset += bytes;

				return allocation;
			}
			
			void Transient::free()
			{
				delete[] data;
			}
			
			Inplace::Inplace(void* location)
				: location(location)
			{}

			void* Inplace::allocate(uint64_t)
			{
				return location;
			}
			
			void Inplace::free() {}
		}

		namespace Dynamic {

		}
	}
}