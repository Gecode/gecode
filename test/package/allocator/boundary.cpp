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

// Space-owned data must survive cloning after the creating thread has exited.
class BufferSpace : public Gecode::Space {
public:
  int* values;
  BufferSpace(void) : values(alloc<int>(128)) {
    for (int i=0; i<128; i++)
      values[i] = i;
  }
  BufferSpace(BufferSpace& s) : Gecode::Space(s), values(alloc<int>(128)) {
    for (int i=0; i<128; i++)
      values[i] = s.values[i];
  }
  Gecode::Space* copy(void) override {
    return new BufferSpace(*this);
  }
};

Gecode::Space* boundary_space(void) {
  return new BufferSpace;
}
bool boundary_valid(Gecode::Space* s) {
  BufferSpace* b = static_cast<BufferSpace*>(s);
  for (int i=0; i<128; i++)
    if (b->values[i] != i)
      return false;
  return true;
}
