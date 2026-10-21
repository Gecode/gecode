# A private source build keeps malloc/new replacement outside Gecode.
function(gecode_configure_mimalloc)
  foreach(target mimalloc mimalloc-static mimalloc-obj mimalloc-obj-target)
    if(TARGET ${target})
      message(FATAL_ERROR "GECODE_ENABLE_MIMALLOC cannot share an existing ${target} target; Gecode requires its own non-overriding mimalloc build")
    endif()
  endforeach()

  # Normal variables keep these choices local, including when Gecode is a subproject.
  # Upstream's minimum policy version predates normal-variable preservation for
  # set(CACHE); keep our local settings when it declares its cache options.
  set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
  endif()
  set(MI_OVERRIDE OFF)
  set(MI_XMALLOC OFF)
  set(MI_EXTRA_CPPDEFS "")
  set(MI_DEBUG_TSAN OFF)
  set(MI_DEBUG_UBSAN OFF)
  set(MI_OSX_INTERPOSE OFF)
  set(MI_OSX_ZONE OFF)
  set(MI_WIN_REDIRECT OFF)
  set(MI_BUILD_SHARED OFF)
  set(MI_BUILD_STATIC OFF)
  set(MI_BUILD_OBJECT ON)
  set(MI_BUILD_TESTS OFF)
  set(MI_USE_CXX DEFAULT)
  set(MI_OPT_ARCH OFF)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(MI_TLS_MODEL LOCAL_DYNAMIC)
  endif()

  include(FetchContent)
  FetchContent_Declare(gecode_mimalloc
    URL https://codeload.github.com/microsoft/mimalloc/tar.gz/refs/tags/v3.5.3
    URL_HASH SHA256=3b4a15153a59905995f7070296ed604bb5ccc00cabb8b93446931aff77224d47)
  FetchContent_MakeAvailable(gecode_mimalloc)
  # Exclude upstream convenience targets and install rules; embed its object directly.
  set_property(DIRECTORY "${gecode_mimalloc_SOURCE_DIR}" PROPERTY EXCLUDE_FROM_ALL TRUE)
  set_target_properties(mimalloc-obj PROPERTIES
    C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)

  # Prefix public and internal identifiers, including globals and TLS data.
  # Reading all platforms avoids a probe build and also covers debug-only names.
  # Leave upstream macros alone so their definitions can still be processed.
  file(GLOB_RECURSE mi_files
    "${gecode_mimalloc_SOURCE_DIR}/include/*.h"
    "${gecode_mimalloc_SOURCE_DIR}/src/*.h"
    "${gecode_mimalloc_SOURCE_DIR}/src/*.c")
  set(mi_names)
  set(mi_macros)
  foreach(mi_file IN LISTS mi_files)
    file(READ "${mi_file}" mi_text)
    string(REGEX MATCHALL "_*mi_[A-Za-z0-9_]+" mi_file_names "${mi_text}")
    list(APPEND mi_names ${mi_file_names})
    string(REGEX MATCHALL "#[ \t]*define[ \t]+_*mi_[A-Za-z0-9_]+" mi_file_macros "${mi_text}")
    foreach(mi_macro IN LISTS mi_file_macros)
      string(REGEX REPLACE "^#[ \t]*define[ \t]+" "" mi_macro "${mi_macro}")
      list(APPEND mi_macros "${mi_macro}")
    endforeach()
  endforeach()
  list(REMOVE_DUPLICATES mi_names)
  list(REMOVE_ITEM mi_names ${mi_macros})
  list(SORT mi_names)
  set(mi_rename "/* Private to Gecode's embedded mimalloc; do not install. */\n")
  foreach(mi_name IN LISTS mi_names)
    string(APPEND mi_rename "#define ${mi_name} gecode_${mi_name}\n")
  endforeach()
  file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gecode-mimalloc-rename.h"
    CONTENT "${mi_rename}" @ONLY)

  # Include the rename header before upstream's single-translation-unit build.
  file(READ "${gecode_mimalloc_SOURCE_DIR}/src/static.c" mi_static)
  if(WIN32)
    # MSVC's /INCLUDE pragmas use literal strings, which macros cannot rename.
    file(READ "${gecode_mimalloc_SOURCE_DIR}/src/prim/windows/prim.c" mi_windows)
    string(REPLACE "/INCLUDE:__mi_" "/INCLUDE:_gecode__mi_" mi_windows "${mi_windows}")
    string(REPLACE "/INCLUDE:_mi_" "/INCLUDE:gecode__mi_" mi_windows "${mi_windows}")
    file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gecode-mimalloc-windows.c"
      CONTENT "${mi_windows}" @ONLY)
    file(READ "${gecode_mimalloc_SOURCE_DIR}/src/prim/prim.c" mi_prim)
    string(REPLACE "\"windows/prim.c\"" "\"gecode-mimalloc-windows.c\"" mi_prim "${mi_prim}")
    file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gecode-mimalloc-prim.c"
      CONTENT "${mi_prim}" @ONLY)
    string(REPLACE "\"prim/prim.c\"" "\"gecode-mimalloc-prim.c\"" mi_static "${mi_static}")
  endif()
  set(mi_wrapper "${CMAKE_CURRENT_BINARY_DIR}/gecode-mimalloc.c")
  file(CONFIGURE OUTPUT "${mi_wrapper}"
    CONTENT "#include \"gecode-mimalloc-rename.h\"\n${mi_static}" @ONLY)
  set_property(TARGET mimalloc-obj PROPERTY SOURCES "${mi_wrapper}")
  target_include_directories(mimalloc-obj PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}" "${gecode_mimalloc_SOURCE_DIR}/src")
  get_directory_property(mi_use_cxx DIRECTORY "${gecode_mimalloc_SOURCE_DIR}" DEFINITION MI_USE_CXX)
  if(mi_use_cxx)
    set_source_files_properties("${mi_wrapper}" TARGET_DIRECTORY mimalloc-obj
      PROPERTIES LANGUAGE CXX)
  endif()

  get_directory_property(mi_libraries DIRECTORY "${gecode_mimalloc_SOURCE_DIR}" DEFINITION mi_libraries)
  set(GECODE_MIMALLOC_SOURCE_DIR "${gecode_mimalloc_SOURCE_DIR}" PARENT_SCOPE)
  set(GECODE_MIMALLOC_LIBRARIES "${mi_libraries}" PARENT_SCOPE)
endfunction()
