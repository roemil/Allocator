## Custom allocators
A header-only lib for custom allocators.

### Type of allocators
* Arena Allocator
* Block Allocator
* Boundary Tag Allocator (Support different placement policies)

### Boundary Tag Allocator
Allocator that allocates a region of memory for you. When that region is freed this region is merged (coalesced) with any neighbouring blocks (if they are also free). This allocator support different polices to find available memory. Implemented policies are first fit and best fit.

### Arena Allocator
An allocator that is useful for allocating multiple objects with the same lifetime.

### Block Allocator
An allocator that is useful when you want to allocate and deallocate object of same time very often.

## Examples
For examples, see test suites.

## Note
Not all allocators have implemented the allocator_traits to a fully extent. It is on going...
