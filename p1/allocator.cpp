#include "allocator.h"

Pointer::Pointer()
{
    pointer = NULL;
}

Pointer::Pointer(void *obj)
{
    pointer = obj;
}

void* Pointer::get() const
{
    return pointer;
}

void Pointer::set(void *obj)
{
    pointer = obj;
}

