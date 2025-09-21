#pragma once

#if defined(__GNUC__) || defined(__GNUG__)
#include <cxxabi.h>

#include <cstdlib>
#include <memory>
#elif defined(_MSC_VER)
#else
static_assert("Failed to detect compiler")
#endif

template <typename T>
std::string prettyTypeName() {
#if defined(__GNUC__) || defined(__GNUG__)
  int status = 0;

  char* demangled =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);

  std::string result =
      (status == 0 && demangled != nullptr) ? demangled : typeid(T).name();

  return result;
#elif defined(_MSC_VER)
  return typeid(T).name()
#else
  return typeid(T).name()
#endif
}
