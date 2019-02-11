#include <iostream>
//#include <iterator>
#include <new>
#include <limits>

template<class T>
class Allocator {
public:
    T* address(T& x) const noexcept {
        return &x;
    }

    const T* adress(const T& x) const noexcept {
        return &x;
    }

    T* allocate(size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p) {
        ::operator delete(static_cast<void*>(p));
    }

    size_t max_size() const noexcept {
        return std::numeric_limits<size_t>::max() / sizeof(T);
    }

    template<typename... Args>
    void constuct(T* p, Args&&... args) {
        ::new ((void*) p) T(std::forward<Args>(args)...);
    }

    void destroy(T* p) {
        p->~T();
    }
};

template<class T>
class Iterator
    :   public std::iterator<std::random_access_iterator_tag, T>
{
private:
    T* p_;
public:
    Iterator() {}
    ~Iterator() {}
    Iterator(T* p)
        :   p_(p)
    {}
    
};

template<class T, class Alloc = std::allocator<T>>
class Vector {
public:
    using iterator = Iterator<T>;
private:
    Alloc alloc_;
};

int main() {
    Allocator<int> alloc;
    std::cout << alloc.max_size() << '\n';
    return 0;
}
