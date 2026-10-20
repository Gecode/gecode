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
  set_target_properties(mimalloc-obj PROPERTIES C_VISIBILITY_PRESET hidden)
  get_directory_property(mi_libraries DIRECTORY "${gecode_mimalloc_SOURCE_DIR}" DEFINITION mi_libraries)
  set(GECODE_MIMALLOC_SOURCE_DIR "${gecode_mimalloc_SOURCE_DIR}" PARENT_SCOPE)
  set(GECODE_MIMALLOC_LIBRARIES "${mi_libraries}" PARENT_SCOPE)
endfunction()
