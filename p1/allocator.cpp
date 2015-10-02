#include "allocator.h"

Pointer::Pointer()
{
    index = 0;
    allocator = NULL;
}

Pointer::Pointer(Allocator *all, int idx)
{
    allocator = all;
    index = idx;
}

void* Pointer::get() const
{
    if (allocator == NULL)
        return NULL;
    return allocator->get_pointer(index);
}

int Pointer::get_index()
{
    return index;
}

void Pointer::set_index(int idx)
{
    index = idx;
}

void Pointer::set_alloc(Allocator *all)
{
    allocator = all;
}

Allocator::Allocator(void *base, size_t size)
{
    //printf("Creating allocator...\n");
    memmory = (size_t*)base;
    max_size = size;
    real_memmory = align_plus(memmory);
    //printf("base:%ld; real_memmory:%ld\n", (size_t)base, real_memmory);
    real_size = ((size_t)base + size - (size_t)real_memmory);
    *real_memmory = real_size;
    //printf("real_size:%ld\n, *real_memmory:%ld", real_size, *(size_t*)real_memmory);
    allocated_employed = 1;
    allocated_size = 5;
    allocated = (size_t**)calloc(1, allocated_size * sizeof(*allocated));
    allocated[0] = NULL;
    //printf("Created allocator.\n");
} 

Allocator::~Allocator()
{
    //printf("Destroying allocator...\n");
    ::free(allocated);
    //printf("Destroyed allocator.\n");
}

Pointer Allocator::alloc(size_t N)
{
    //printf("Allocating memmory...\n");
    size_t *ptr = real_memmory;
    N = align_plus(N);
    N /= sizeof(size_t);
    if (N == 0)
    {
        printf("\tRequested 0 memory\nNo allocation.\n");
        return Pointer(this, 0);
     }
    while (1)
     {
        if (ptr == real_memmory + real_size / sizeof(size_t))
         {   // has gone into the end of allocated memmory
            //printf("\tNO MEMMORY\n");
            throw AllocError(AllocErrorType::NoMemory, "\n");
            return Pointer(this, 0);
        }
        if (!get_status(ptr) && 
            align_minus(*ptr) >= (N + 2) * sizeof(size_t))
        {   // enough free space for data + align + 2 headers
            //printf("\tCreating a new block...\n");

            size_t *old_end = ptr + get_block_size_t(ptr) - 1;
            size_t *new_end = ptr + N + 1;
            size_t *new_begin = ptr + N + 2;
            size_t new_size = get_block_size(ptr)
                                - (N + 2) * sizeof(size_t);
            if (new_end != old_end)
                *new_begin = new_size;
            *old_end = new_size;
            *new_end = (N + 2) * sizeof(size_t);
            *ptr = (N + 2) * sizeof(size_t);
            *ptr |= 1;
            *new_end |= 1;
            //printf("\tCreated the new block.\n");
            //printf("Allocated.\n");
            return Pointer(this, push_in_allocated(ptr));
        }
        else
        {
            //printf("Have not found free space, trying next...\n");
            //printf("ptr=%ld\nend_ptr=%ld\n", (size_t)ptr - (size_t)real_memmory, real_size);
            ptr = ptr + get_block_size_t(ptr);
            //printf("%ld\n", get_block_size_t(ptr));
            //printf("%ld\n", *ptr); 
            //exit(0);
        }
        
     }
}  

void Allocator::realloc(Pointer &p, size_t N)
{
    //printf("Reallocing memmory...\n");
    if (p.get() == NULL)
    {
        p = alloc(N);
        return;
    }
    if (N == 0)
    {
        this->free(p);
        return;
    }
    N = align_plus(N);
    N /= sizeof(size_t);
    size_t *temp = (size_t*)p.get() - 1;
    if (N <= get_block_size_t(temp) && N + 2 > get_block_size_t(temp))
    {
        return;
    }
    if (N + 2 <= get_block_size_t(temp))
    {
        size_t *old_begin = temp + get_block_size_t(temp);
        size_t *new_begin = temp + (N + 2);
        size_t *new_end = new_begin - 1;
        size_t *old_end = temp + get_block_size_t(temp) - 1;
        *new_begin = get_block_size(temp) - (N + 2) * sizeof(size_t);
        *new_end = (N + 2) * sizeof(size_t);
        *old_end = *new_begin;
        *temp = *new_end;
        if (!get_status(old_begin))
        {
            merge(new_begin, old_begin);
        }
        return;
    }
    size_t *next = temp + get_block_size_t(temp);
    if ((next == real_memmory + real_size / sizeof(size_t))
        || get_status(next) 
        || get_block_size_t(next) + get_block_size(temp) < N)
    {
        Pointer p1 = alloc(N * sizeof(size_t));
        memmove(p1.get(), p.get(), 
                get_block_size(temp) - 2 * sizeof(size_t));
        this->free(p);
        p = p1;
        return;
    } 
    else
    {
        if (N + 2 <= get_block_size_t(next) + get_block_size_t(temp) 
            && get_block_size_t(next) + get_block_size_t(temp)
            < N + 4)
        {
            size_t *old_end = next + get_block_size_t(next) - 1;
            *old_end = get_block_size(temp) + get_block_size(next);
            *temp = *old_end;
            return;
        }
        size_t *new_begin = temp + (N + 2);
        size_t *old_begin = temp + get_block_size_t(temp);
        size_t *new_end = new_begin - 1;
        size_t *old_end = old_begin + get_block_size_t(old_begin);
        size_t old_size = *old_begin;
        *new_begin = old_size + get_block_size(temp) - 
                    (N + 2) * sizeof(size_t);
        *new_end = (N + 2) * sizeof(size_t);
        *new_end |= 1;
        *old_end = *new_begin;
        *temp = *new_end;
        *temp |= 1;
    }
    //printf("Realloced memmory.\n");
}

void Allocator::free(Pointer &p)
{
    //printf("Being free memmory...\n");
    size_t *temp = (size_t*)p.get();
    if (temp == NULL)
    {
        throw AllocError(AllocErrorType::InvalidFree, "\n");
        return;
    }
    --temp;
    size_t *next = temp + get_block_size_t(temp);
    size_t *prev = (temp - 1) - get_block_size_t(temp - 1);
    *temp &= -2;
    if (!get_status(next))
        merge(temp, next);
    if (!get_status(prev))
        merge(prev, temp);
    pop_from_allocated(p);
    //printf("Memmory is free.\n");
}

void Allocator::defrag()
{
    //printf("Defrag start...\n");
    size_t *ptr = real_memmory + get_block_size_t(real_memmory);
    int cnt = 0;
    while (ptr != real_memmory + real_size / sizeof(size_t))
    {
        size_t *prev = ptr - get_block_size_t(ptr - 1);
        size_t *next = ptr + get_block_size_t(ptr);
        //printf("\t#%d:\tprev =%ld\tptr=%ld\tnext=%ld\n",
        //        cnt, (prev - real_memmory) * sizeof(size_t), 
        //        (ptr - real_memmory) * sizeof(size_t),
        //        (next - real_memmory) * sizeof(size_t));
        if (!get_status(prev))
        {
            int index = 0;
            for (size_t i = 0; i < allocated_employed; ++i)
            {
                if (allocated[i] == ptr)
                {
                    index = i;
                    break;
                }
            }
            if (index == 0)
            {
                printf("UNKNOWEN ERROR!\n");
                return;
            }
            memmove(prev, ptr, get_block_size(ptr));
            ptr = prev + get_block_size_t(prev);
            *(ptr - 1) = get_block_size(prev);
            *(ptr - 1) |= 1;
            *ptr = (size_t)next - (size_t)ptr;
            *(next - 1) = *ptr;
            if (!get_status(next))
                merge(ptr, next);
            allocated[index] = prev;
        }
        cnt++;
        ptr = ptr + get_block_size_t(ptr);
    }
    //printf("Defrag ended.\n");
}

void Allocator::print()
{
    printf("Print...\n");
    int cnt = 0;
    printf("Allocated:\n");
    for (size_t i = 0; i < allocated_employed; ++i)
    {
        if (allocated[i] != NULL)
        {
            
            printf("\t#%d: Point:\t%lu\tBlock_size:\t%lu\n", cnt++,
                    (allocated[i] - real_memmory) * sizeof(size_t),
                    *allocated[i]);
        }
    }
    printf("All memory blocks:\n");
    size_t* ptr = real_memmory;
    cnt = 0;
    while ( ptr != real_memmory + real_size / sizeof(size_t) )
    {
        printf("\t#%d: Point:\t%lu\tBlock_size:\t%lu\tStatus:%d\n",
                cnt++, (size_t)ptr - (size_t)real_memmory,
                get_block_size(ptr), get_status(ptr));
        ptr = ptr + get_block_size_t(ptr);
    }
    printf("Printed.\n");
}



void* Allocator::get_pointer(int index)
{
    if (index <= 0 || (size_t)index >= allocated_employed)
        return NULL;
    return (allocated[index] + 1);
}

size_t* Allocator::align_plus(size_t *num)
{
    return (size_t*)(((size_t)num + 7) & -8);
}

size_t Allocator::align_plus(size_t num)
{
    return (num + 7) & -8;
}


size_t* Allocator::align_minus(size_t *num)
{
    return (size_t*)((size_t)num & -8);
}

size_t Allocator::align_minus(size_t num)
{
    return num & -8;
}

int Allocator::push_in_allocated(size_t *ptr)
{
    //printf("Pushing into allocated...\n");
    for (size_t i = 1; i < allocated_employed; ++i)
    {
        if (allocated[i] == NULL)
        {
            allocated[i] = ptr;
            //printf("\tFound old free space.\n");
            return i;
        }
    }
    if (allocated_employed >= allocated_size)
    {
        allocated_size <<= 1;
        size_t **allocated_temp = (size_t**) ::realloc(allocated, 
                                        allocated_size *
                                        sizeof(size_t*));
        if (allocated_temp == NULL)
        {
            printf("No memmory.\n");
            return 0;
        }
        //printf("Realloced allocated\n");
        allocated = allocated_temp;
    }
    allocated[allocated_employed] = ptr;
    //printf("Push have been ended.\n");
    return allocated_employed++;   
}

void Allocator::pop_from_allocated(Pointer &p)
{
    allocated[p.get_index()] = 0;
    p.set_index(0);
    p.set_alloc(NULL);
}


int Allocator::get_status(size_t* p)
{
    return *p & 1;
}

void Allocator::merge(size_t *p1, size_t *p2)
{
    *p1 &= -2;
    *p2 &= -2;
    size_t *old_begin = p1 + get_block_size_t(p1);
    //size_t *old_end = old_begin - 1;
    size_t *new_end = old_begin + get_block_size_t(old_begin) - 1;
    *p1 = get_block_size(p1) + get_block_size(old_begin);
    *new_end = get_block_size(p1);
}



size_t Allocator::get_block_size_t(size_t *ptr)
{
    return (*ptr & -2) / sizeof(size_t);
}

size_t Allocator::get_block_size(size_t *ptr)
{
    return *ptr & -2;
}

