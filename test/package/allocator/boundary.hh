#ifndef GECODE_TEST_PACKAGE_ALLOCATOR_BOUNDARY_HH
#define GECODE_TEST_PACKAGE_ALLOCATOR_BOUNDARY_HH

#include <gecode/kernel.hh>

#if defined(_WIN32) && !defined(ALLOCATOR_BOUNDARY_STATIC)
#ifdef ALLOCATOR_BOUNDARY_BUILD
#define ALLOCATOR_BOUNDARY_EXPORT __declspec(dllexport)
#else
#define ALLOCATOR_BOUNDARY_EXPORT __declspec(dllimport)
#endif
#else
#define ALLOCATOR_BOUNDARY_EXPORT
#endif

ALLOCATOR_BOUNDARY_EXPORT void* boundary_alloc(size_t n);
ALLOCATOR_BOUNDARY_EXPORT void* boundary_realloc(void* p, size_t n);
ALLOCATOR_BOUNDARY_EXPORT void boundary_free(void* p);
ALLOCATOR_BOUNDARY_EXPORT Gecode::Space* boundary_space(void);
ALLOCATOR_BOUNDARY_EXPORT bool boundary_valid(Gecode::Space* s);

#endif
