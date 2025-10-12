#pragma once

// Based heavily off https://skypjack.github.io/2020-03-14-ecs-baf-part-8/
// TODO: C++26 should fix this? hopefully?

#include <cstdint>
#include <string_view>

namespace Vivium {
// TODO: pretty collision resistant, but just in case can we perform any sanity
// check? FNV-1a 64-bit hash (constexpr) (public domain)
consteval uint64_t fnv1a_hash(std::string_view str) {
  uint64_t hash = 14695981039346656037ull;
  for (char c : str) {
    hash ^= static_cast<uint64_t>(c);
    hash *= 1099511628211ull;
  }
  return hash;
}

// TODO: if these compilers ever change how they represent types, we have to do
// more work
template <typename T>
consteval std::string_view type_name() {
#if defined(__clang__)
  std::string_view p = __PRETTY_FUNCTION__;
  constexpr std::string_view prefix =
      "std::string_view Vivium::type_name() [T = ";
  constexpr std::string_view suffix = "]";
#elif defined(__GNUC__)
  std::string_view p = __PRETTY_FUNCTION__;
  constexpr std::string_view prefix =
      "consteval std::string_view Vivium::type_name() [with T = ";
  constexpr std::string_view suffix = "]";
#elif defined(_MSC_VER)
  std::string_view p = __FUNCSIG__;
  constexpr std::string_view prefix =
      "class std::basic_string_view<char,struct std::char_traits<char> > "
      "__cdecl Vivium::type_name<";
  constexpr std::string_view suffix = ">(void)";
#else
#error Unsupported compiler
#endif

  p.remove_prefix(prefix.size());
  p.remove_suffix(suffix.size());
  return p;
}

// Stable type id generation
//  might not work well across library boundaries
template <typename T>
consteval uint64_t type_id() {
  return fnv1a_hash(type_name<T>());
}
}  // namespace Vivium