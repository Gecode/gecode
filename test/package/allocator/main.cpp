#include "boundary.hh"
#include <cstdlib>
#include <cstring>
#include <thread>
#ifdef CHECK_HOST_MIMALLOC
#include <mimalloc.h>
#endif

#if GECODE_EXPECT_MIMALLOC && !defined(GECODE_MIMALLOC)
#error "Expected mimalloc-enabled installed headers"
#elif !GECODE_EXPECT_MIMALLOC && defined(GECODE_MIMALLOC)
#error "Expected system-allocator installed headers"
#endif

int main(void) {
  void* p = Gecode::heap.ralloc(256);
#ifdef CHECK_HOST_MIMALLOC
  void* host = mi_malloc(256);
  void* system = std::malloc(256);
  bool isolated = !mi_is_in_heap_region(p) && mi_is_in_heap_region(host);
#if GECODE_HOST_MIMALLOC_OVERRIDE
  isolated = isolated && mi_is_in_heap_region(system);
#else
  isolated = isolated && !mi_is_in_heap_region(system);
#endif
  mi_free(host);
  std::free(system);
  if (!isolated)
    return 1;
#endif
  std::memset(p,0x5a,256);
  std::thread resize([&] { p = boundary_realloc(p,16384); });
  resize.join();
  for (int i=0; i<256; i++)
    if (static_cast<unsigned char*>(p)[i] != 0x5a)
      return 2;
#ifdef CHECK_HOST_MIMALLOC
  if (mi_is_in_heap_region(p))
    return 3;
#endif
  std::thread release([&] { boundary_free(p); });
  release.join();
  std::thread allocate([&] { p = boundary_alloc(1024); });
  allocate.join();
  p = Gecode::heap.rrealloc(p,4096);
  Gecode::heap.rfree(p);

  return 0;
}
