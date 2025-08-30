#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "spirv-cross-c-shared" for configuration "Release"
set_property(TARGET spirv-cross-c-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(spirv-cross-c-shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libspirv-cross-c-shared.so.0.67.0"
  IMPORTED_SONAME_RELEASE "libspirv-cross-c-shared.so.0"
  )

list(APPEND _cmake_import_check_targets spirv-cross-c-shared )
list(APPEND _cmake_import_check_files_for_spirv-cross-c-shared "${_IMPORT_PREFIX}/lib/libspirv-cross-c-shared.so.0.67.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
