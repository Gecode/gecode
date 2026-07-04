### Try to find MPFR
# Once done this will define
#  MPFR_FOUND         - System has MPFR
#  MPFR_INCLUDE_DIRS  - The MPFR include directories
#  MPFR_LIBRARIES     - The libraries needed to use MPFR
#  MPFR::MPFR         - Imported target for MPFR
#  GMP::GMP           - Imported target for GMP

set(_MPFR_ROOT_HINTS)
foreach(_MPFR_ROOT_VAR IN ITEMS MPFR_ROOT GMP_ROOT)
  if(DEFINED ${_MPFR_ROOT_VAR})
    list(APPEND _MPFR_ROOT_HINTS "${${_MPFR_ROOT_VAR}}")
  endif()
endforeach()
foreach(_MPFR_ROOT_ENV IN ITEMS MPFR_ROOT GMP_ROOT MPFRDIR GMPDIR)
  if(DEFINED ENV{${_MPFR_ROOT_ENV}})
    list(APPEND _MPFR_ROOT_HINTS "$ENV{${_MPFR_ROOT_ENV}}")
  endif()
endforeach()

find_path(MPFR_INCLUDE_DIR NAMES mpfr.h
          HINTS ${_MPFR_ROOT_HINTS}
          PATH_SUFFIXES include
          PATHS ${INCLUDE_INSTALL_DIR})

find_library(MPFR_LIBRARY NAMES mpfr
             HINTS ${_MPFR_ROOT_HINTS}
             PATH_SUFFIXES lib lib64
             PATHS ${LIB_INSTALL_DIR})

find_path(GMP_INCLUDE_DIR NAMES gmp.h
          HINTS ${_MPFR_ROOT_HINTS}
          PATH_SUFFIXES include
          PATHS ${INCLUDE_INSTALL_DIR})

find_library(GMP_LIBRARY NAMES gmp
             HINTS ${_MPFR_ROOT_HINTS}
             PATH_SUFFIXES lib lib64
             PATHS ${LIB_INSTALL_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MPFR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(MPFR
  FOUND_VAR MPFR_FOUND
  REQUIRED_VARS MPFR_LIBRARY MPFR_INCLUDE_DIR GMP_LIBRARY GMP_INCLUDE_DIR
  FAIL_MESSAGE "Could NOT find MPFR, use MPFR_ROOT to hint its location"
)

mark_as_advanced(MPFR_INCLUDE_DIR MPFR_LIBRARY GMP_INCLUDE_DIR GMP_LIBRARY)

if(MPFR_FOUND AND NOT TARGET GMP::GMP)
  add_library(GMP::GMP UNKNOWN IMPORTED)
  set_target_properties(GMP::GMP PROPERTIES
    IMPORTED_LOCATION "${GMP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}")
endif()

if(MPFR_FOUND AND NOT TARGET MPFR::MPFR)
  add_library(MPFR::MPFR UNKNOWN IMPORTED)
  set_target_properties(MPFR::MPFR PROPERTIES
    IMPORTED_LOCATION "${MPFR_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MPFR_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES GMP::GMP)
endif()

set(MPFR_INCLUDE ${MPFR_INCLUDE_DIR})
set(MPFR_INCLUDE_DIRS ${MPFR_INCLUDE_DIR})
set(MPFR_INCLUDES ${MPFR_INCLUDE_DIR})
set(MPFR_LIBRARIES ${MPFR_LIBRARY} ${GMP_LIBRARY})
set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
set(GMP_LIBRARIES ${GMP_LIBRARY})

unset(_MPFR_ROOT_ENV)
unset(_MPFR_ROOT_HINTS)
unset(_MPFR_ROOT_VAR)
