#include "boundary_tag_allocator.h"

namespace Allocator {
void coalesce_once(detail::Block *p) {
  if (!p) {
    return;
  }
  if (!p->next) {
    return;
  }
  p->size_ += p->next->size_;
  p->next = p->next->next;
  if (p->next) {
    p->next->prev = p;
  }
}

} // namespace Allocator