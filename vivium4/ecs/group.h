#pragma once

#include "defines.h"

namespace Vivium {
	template <typename T>
	struct Owned { using type = T; };
	template <typename T>
	struct Partial { using type = T; };

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

	struct GroupMetadata {
		uint64_t groupSize;
		Signature ownedComponents;
		Signature partialComponents;
		Signature affectedComponents; // ownedComponents | partialComponents

		template <OwnershipTag... WrappedTypes>
		void create() {
			// Setup signatures
			([] {
				uint8_t id = TypeGenerator<WrappedTypes::type>::getIdentifier();
				
				affectedComponents.set(id);

				if constexpr (IsOwnedTag<WrappedTypes>::value) {
					ownedComponents.set(id);
				}
				else {
					partialComponents.set(id);
				}
			} (), ...);
		}

		bool ownedID(uint8_t id);
		bool containsID(uint8_t id);

		// Perfect match
		bool ownsSignature(Signature const& signature);
		// TODO: function should be inverted, suggests "signature" is subset of us, but tests for us being a subset of "signature"
		bool containsSignature(Signature const& signature);

		template <typename T>
		bool contains() { return affectedComponents.test(TypeGenerator<T>::getIdentifier()); }
		template <typename... Ts>
		bool any() { return (contains<Ts>() || ...); }
		template <typename... Ts>
		bool all() { return (contains<Ts>() && ...); }
	};
}