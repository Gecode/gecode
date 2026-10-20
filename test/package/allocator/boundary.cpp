#include "boundary.hh"

void* boundary_alloc(size_t n) {
  return Gecode::heap.ralloc(n);
}
void* boundary_realloc(void* p, size_t n) {
  return Gecode::heap.rrealloc(p,n);
}
void boundary_free(void* p) {
  Gecode::heap.rfree(p);
}
