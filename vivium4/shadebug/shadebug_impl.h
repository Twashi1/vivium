#pragma once

#include "shadebug.h"

namespace Vivium {
// TODO: export explicit instantiations of this function
template <ShadebugTokenType _TokenType, typename T>
ShadebugToken _createShadebugToken(ShadebugLexerContext& context,
                                   T const& value) {
  ShadebugToken token;
  _populateTokenDebug(context, token);

  token.type = _TokenType;

  if constexpr (token.data.inplace.size() <= sizeof(T)) {
    new (token.data.inplace.data()) T(value);
  } else {
    static_assert(false && "Unimplemented");
  }

  return token;
}

template <ShadebugTokenType _TokenType>
ShadebugToken _createShadebugToken(ShadebugLexerContext& context,
                                   std::string_view const value) {
  ShadebugToken token;
  _populateTokenDebug(context, token);

  token.type = _TokenType;

  TriviallyRelocateableString* string =
      std::bit_cast<TriviallyRelocateableString*>(
          allocate(context.allocator, sizeof(TriviallyRelocateableString),
                   alignof(TriviallyRelocateableString)));
}
}  // namespace Vivium
