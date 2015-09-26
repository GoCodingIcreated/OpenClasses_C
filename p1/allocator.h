#include <stdexcept>
#include <string>
#include <list>
#include <string.h>
#include <iostream>
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
    Pointer(void *obj);
    void *get() const;
    void set(void *obj);
private:
    void *pointer;
};

class Allocator {
public:
    Allocator(void *base, size_t size) 
    {
        memmory = base;
        max_size = size;
        memset(base, 0, size);
        pointers.push_back(make_pair(Pointer(base), 1));
        pointers.push_back(make_pair(Pointer((char*)base + size - 1), 1));
        //cout << "ALLOCATOR CREATED\n";
    }
    
    Pointer alloc(size_t N)
    {
        auto it = pointers.begin();
        auto it_next = it;
        it_next++;
        for ( ; 
              it_next != pointers.end(); 
              ++it, it_next++)
        {
            if ( (char*)it_next->first.get() - 
                ((char*)it->first.get() + it->second) >= N )
            {
                Pointer p((char*)it->first.get() + it->second);
                pointers.insert(it_next, make_pair(p, N));
                //cout << "POINTER ALLOCED\n";
                return p;
            }
        }
        throw AllocError(AllocErrorType::NoMemory, "ehhhhh...\n");
        //cout << "CAN NOT ALLOC MEMMORY\n";
        return Pointer(0);
    }
    void realloc(Pointer &p, size_t N) 
    {
        
        auto it = pointers.begin();
        for ( ; it->first.get() != p.get() && it != pointers.end(); ++it );
        if ( it == pointers.end() )
        {
            //cout << "INVALID POINTER IN REALLOC\n";
            throw AllocError(AllocErrorType::InvalidFree, "uhh...\n");
            return;
        }
        auto it_next = it;
        it_next++;
        if ( (char*)it_next->first.get() - (char*)it->first.get() > N )
        {
            //cout << "REALLOCED ON PLACE\n";
            it->second = N;
        }
        else
        {
            Pointer p_temp = alloc(N);
            memmove(p_temp.get(), it->first.get(), it->second);
            free(p);
            p = p_temp;
            //cout << "REALLOCED ON OTHER PLACE\n";
        }
    }
    void free(Pointer &p)
    {
        
        auto it = pointers.begin();
        for ( ; it->first.get() != p.get() && it != pointers.end(); ++it );
        if  ( it == pointers.end() )
        {
            throw AllocError(AllocErrorType::InvalidFree, "ahh..\n");
            //cout << "INVALID POINTER IN FREE\n";
            return;
        }
        pointers.erase(it);
        //cout << "OK FREE\n";
        
    }

    void defrag() 
    {
        
        auto it = pointers.begin();
        it++;
        auto it_next = it;
        it_next++;
        for ( ; (it_next) != pointers.end(); ++it, ++it_next )
        {
            free(it->first);
            auto it_temp = alloc(it->second);
            memmove(it_temp.get(), it->first.get(), it->second);
            //cout << "OTHER ONE POINTER MOVED\n";
        }
        //cout << "END OF DEFRAG\n";
        
    }
    void print()
    {
        for ( auto it = pointers.begin(); it != pointers.end(); ++it)
        {
            printf("%ld %d\n", (char*)it->first.get() - (char*)memmory, it->second);
        }
    }
    std::string dump() { return ""; }
private:
    void *memmory;
    size_t max_size;
    std::list<std::pair<Pointer, int> > pointers;
};


/*
int main()
{
    size_t size = 10000;
    void *p = malloc(size);
    Allocator allocator(p, size);
    Pointer p1 = allocator.alloc(10);
    Pointer p2 = allocator.alloc(20);
    allocator.print();
    allocator.realloc(p1, 30);
    allocator.print();
    return 0;

}
*/
