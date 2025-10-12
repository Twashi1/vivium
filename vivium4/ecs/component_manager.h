#pragma once

#include <concepts>

#include "../error/log.h"
#include "../serialiser/serialiser.h"
#include "defines.h"

namespace Vivium {
// TODO: stricter definition of a valid component
//	removed the old stuff because issues with serialisation
template <typename T>
concept ValidComponent = true;

/*! \brief Move a component from source to destination.
 *
 * No requirement to preserve the memory at source, can be invalidated for
 * performance.
 *
 * \tparam T The type of component to move.
 * \param source The source memory to move from.
 * \param dest The destination memory to move to.
 */
template <ValidComponent T>
void defaultMoveComponent(void* source, void* dest);

/*! \brief Copy a component from source to destination.
 *
 * Requires that the data at source is preserved for future copies.
 *
 * \tparam T The type of component to copy.
 * \param source The source memory to copy from.
 * \param dest The destination memory to copy to.
 */
template <ValidComponent T>
void defaultCopyComponent(void const* source, void* dest);

/*! \brief Moves a list of components from source to destination.
 *
 * No requirement to preserve the memory of any individual component at source.
 * The list of components is continguous in memory.
 *
 * \tparam T The type of component to move.
 * \param source The source memory to copy from.
 * \param dest The destination memory to copy to.
 * \param count The number of components to move - assumed to be the whole list
 * often.
 */
template <ValidComponent T>
void defaultReallocComponent(void* source, void* dest, uint64_t count);

/*! \brief Deconstruct a list of components.
 *
 * No requirement to zero memory or perform deconstruction on POD types.
 *
 * \tparam T The type of component to destroy.
 * \param data The list of components.
 * \param count The number of components to destroy.
 */
template <ValidComponent T>
void defaultDestroyComponent(void* data, uint64_t count);

/*! \brief Swap the memory of the components starting at a and b
 *
 * No guarantee that a and b will be different addresses or different
 * components.
 *
 * \tparam T The type of component to swap.
 * \param a The first address to swap.
 * \param b The second address to swap.
 */
template <ValidComponent T>
void defaultSwapComponent(void* a, void* b);

// TODO: should take ValidComponent as a concept
/*! \brief Write the data to the given memory interface.
 *
 * Must preserve the data at source.
 *
 * \tparam T The type of component to serialise.
 * \param src Pointer to the component to serialise.
 * \param store The memory interface to serialise to.
 */
template <typename T>
void defaultSerialiseWrite(void const* src, SerialiserMemoryInterface& store);

/*! \brief Read the data from the given memory interface.
 *
 * \tparam T The type of component to deserialise.
 * \param src Pointer to the memory to write to.
 * \param store The memory interface to deserialise from.
 */
template <typename T>
void defaultSerialiseRead(void* src, SerialiserMemoryInterface& store);

struct ComponentManager {
  typedef void (*MoveFunction)(void* src, void* dst);
  typedef void (*CopyFunction)(void const* src, void* dst);
  typedef void (*SerialiseWrite)(void const* src,
                                 SerialiserMemoryInterface& store);
  typedef void (*SerialiseRead)(void* src, SerialiserMemoryInterface& store);
  typedef void (*ReallocFunction)(void* src, void* dst, uint64_t);
  typedef void (*DestroyFunction)(void*, uint64_t);
  typedef void (*SwapFunction)(void*, void*);

  MoveFunction moveFunction = nullptr;
  CopyFunction copyFunction = nullptr;
  SerialiseWrite writeFunction = nullptr;
  SerialiseRead readFunction = nullptr;
  ReallocFunction reallocFunction = nullptr;
  DestroyFunction destroyFunction = nullptr;
  SwapFunction swapFunction = nullptr;

  uint64_t componentTypeIndex;
  uint64_t typeSize;

  // TODO: get rid of this function, make it a non-member
  uint64_t getOffset(uint64_t index) const;
};

/*! \brief Create a default component manager for the type.
 *
 * \tparam T The component type to create the manager for.
 * \param componentTypeIndex The registry's type index for that component.
 * \return The component manager.
 */
template <typename T>
ComponentManager defaultComponentManager(uint64_t componentTypeIndex);
}  // namespace Vivium