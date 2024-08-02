#include "group.h"

namespace Vivium {
	bool GroupMetadata::ownedID(uint8_t id) { return ownedComponents.test(id); }
	bool GroupMetadata::containsID(uint8_t id) { return affectedComponents.test(id); }

	// Perfect match
	bool GroupMetadata::ownsSignature(Signature const& signature) { return (signature & ownedComponents) == ownedComponents; }
	// TODO: function should be inverted, suggests "signature" is subset of us, but tests for us being a subset of "signature"
	bool GroupMetadata::containsSignature(Signature const& signature) { return (signature & affectedComponents) == affectedComponents; }
}