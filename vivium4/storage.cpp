#include "storage.h"

namespace Vivium {
	namespace Allocator {
		namespace Static {
			Pool::Pool()
				// Default block capacity
				: Pool(4096)
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
					reinterpret_cast<uint8_t*>(std::malloc(blockCapacity)), 
					bytes
				);

				return blocks.back().data;
			}

			void Pool::free()
			{
				for (Block block : blocks) {
					std::free(block.data);
				}
			}

			Pool::Block::Block(uint8_t* data, uint64_t offset)
				: data(data), offset(offset)
			{}
		}

		namespace Dynamic {

		}
	}
}