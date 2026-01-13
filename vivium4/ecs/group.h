#pragma once

#include "defines.h"

namespace Vivium {
template <typename T>
struct Owned {
  using type = T;
};
template <typename T>
struct Partial {
  using type = T;
};

template <typename T>
struct IsValidOwnershipTag : std::false_type {};
template <typename T>
struct IsValidOwnershipTag<Owned<T>> : std::true_type {};
template <typename T>
struct IsValidOwnershipTag<Partial<T>> : std::true_type {};

template <typename T>
struct IsOwnedTag : std::false_type {};
template <typename T>
struct IsOwnedTag<Owned<T>> : std::true_type {};

template <typename T>
struct IsPartialTag : std::false_type {};
template <typename T>
struct IsPartialTag<Partial<T>> : std::true_type {};

template <typename T>
concept OwnershipTag = IsValidOwnershipTag<T>::value;
template <typename T>
concept PartialTag = IsPartialTag<T>::value;
template <typename T>
concept OwnedTag = IsOwnedTag<T>::value;

struct Registry;

struct GroupMetadata {
  uint64_t groupSize;
  Signature ownedComponents;
  Signature partialComponents;
  Signature affectedComponents;  // ownedComponents | partialComponents
  Registry* registry;

  // TODO: interdependency issue now
  /*! \brief Construct group metadata from a set of annotated types. */
  template <OwnershipTag... WrappedTypes>
  void create();

  /*! \brief Return if this group owns a type index. */
  bool ownedID(uint8_t id);
  /*! \brief Return if this group affects a type index. */
  bool containsID(uint8_t id);

  /*! \brief Check that the given signature contains exactly the owned
   * components. */
  bool ownsSignature(Signature const& signature);
  // TODO: function should be inverted, suggests "signature" is subset of us,
  // but tests for us being a subset of "signature"
  /*! \brief Check that this signature contains exactly the affected types of
   * this group. */
  bool containsSignature(Signature const& signature);

  /*! \brief Returns if group affects the passed type. */
  template <typename T>
  bool contains();
  /*! \brief Return if any of the passed types are contained. */
  template <typename... Ts>
  bool any();
  /*! \brief Return if all of the passed types are contained. */
  template <typename... Ts>
  bool all();
};

template <typename T, OwnershipTag... Components>
constexpr inline bool _isOwnedType =
    (std::is_same_v<T, typename Components::type> || ...);

// https://internalpointers.com/post/writing-custom-iterators-modern-cpp
template <OwnershipTag... Components>
struct ViewElement {
  uint64_t index;
  Entity entity;

  Registry* registry;

  template <typename T>
  T& get();

  template <typename T>
  T const& get() const;
};

template <OwnershipTag... WrappedTypes>
struct View {
  Registry* registry;
  // TODO: if the array re-allocates, this will fail
  //  because the owned entity array changes location
  //  if this doesn't own group metadata, it also relies on that
  Entity* ownedEntityArray;
  GroupMetadata* groupMetadata;

  struct ViewIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = void;
    using value_type = ViewElement<WrappedTypes...>;
    using pointer = value_type*;
    using reference = value_type&;

    Registry* registry;
    Entity* ownedEntityArray;
    GroupMetadata* groupMetadata;

    value_type current;

    ViewIterator(Registry* registry, Entity* ownedEntityArray,
                 GroupMetadata* groupMetadata, uint64_t startIndex,
                 Entity entity);

    reference operator*();
    pointer operator->();

    ViewIterator& operator++();
    ViewIterator operator++(int);

    bool operator==(ViewIterator const& other);
    bool operator!=(ViewIterator const& other);
  };

  ViewIterator begin();
  ViewIterator end();
};
}  // namespace Vivium
