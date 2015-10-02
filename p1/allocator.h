#ifndef ALLOCATOR
#define ALLOCATOR

#include <stdexcept>
#include <string>
#include <list>
#include <string.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {
public:
    Pointer();
    Pointer(Allocator*, int);
    void *get() const;
    int get_index();
    void set_index(int);
    void set_alloc(Allocator*);
private:
    Allocator* allocator;
    int index;
};

class Allocator {
public:
    Allocator(void *base, size_t size);
    ~Allocator();
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag();
    
    void print();
    void* get_pointer(int);
    std::string dump() { return ""; }
    
private:
    size_t* align_plus(size_t *);
    size_t* align_minus(size_t*);
    size_t align_plus(size_t);
    size_t align_minus(size_t);
    int push_in_allocated(size_t *);
    void pop_from_allocated(Pointer &p);
    size_t *memmory;
    size_t *real_memmory;
    size_t real_size;
    size_t max_size;
    
    size_t get_block_size(size_t *ptr);
    size_t get_block_size_t(size_t *ptr);
    int get_status(size_t*);
    void merge(size_t*, size_t*);
    size_t allocated_size;
    size_t allocated_employed;
    size_t** allocated;
};
#endif
