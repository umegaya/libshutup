#pragma once

#include <cstdlib> 
#include <stdexcept>

namespace shutup {
//system mempool using std::malloc, free, realloc.
class IMempool {
public:
	virtual ~IMempool() {}
	virtual void *malloc(size_t sz) = 0;
	virtual void free(void *p) = 0;
	virtual void *realloc(void *p, size_t sz) = 0;
	virtual bool operator == (const IMempool &m) { return true; }
};
class SystemMempool : public IMempool {
	void *malloc(size_t sz) {
		return std::malloc(sz);
	}
	void free(void *p) {
		std::free(p);
	}
	void *realloc(void *p, size_t sz) {
		return std::realloc(p, sz);
	}
};

//generate STL allocator from instance of M, which provides
//alloc/free/realloc. heavily based on https://blogs.msdn.microsoft.com/vcblog/2008/08/28/the-mallocator/
template <typename T> 
class allocator {
public:
    // common part for almost all allocator.
    typedef T * pointer;
    typedef const T * const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    T * address(T& r) const {
        return &r;
    }
    const T * address(const T& s) const {
        return &s;
    }

    size_t max_size() const {
        // The following has been carefully written to be independent of
        // the definition of size_t and to avoid signed/unsigned warnings.
        return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
    }

    template <typename U> struct rebind {
        typedef allocator<U> other;
    };

    bool operator!=(const allocator& other) const {
        return !(*this == other);
    }

    void construct(T * const p, const T& t) const {
        void * const pv = static_cast<void *>(p);
        new (pv) T(t);
    }

    void destroy(T * const p) const; // Defined below.

    // Returns true if and only if storage allocated from *this
    // can be deallocated from other, and vice versa.
    // Always returns true for stateless allocators.
    bool operator==(const allocator& other) const {
        return m_->operator==(*other.m_);
    }

    // Default constructor, copy constructor, rebinding constructor, and destructor.
    // Empty for stateless allocators.
    allocator(IMempool *m = nullptr) : m_(m == nullptr ? new SystemMempool() : m) {}
    allocator(const allocator &a) : m_(&(a.pool())) {}
    template <typename U> allocator(const allocator<U> &a) : m_(&(a.pool())) {}
    ~allocator() {}

    // accessor of underlaying mempool
   	inline IMempool &pool() const { return *m_; }

    // The following will be different for each allocator.
    T * allocate(const size_t n) const {
        // The return value of allocate(0) is unspecified.
        // this module returns NULL in order to avoid depending
        // on malloc(0)’s implementation-defined behavior
        // (the implementation can define malloc(0) to return NULL,
        // in which case the bad_alloc check below would fire).
        // All allocators can return NULL in this case.
        if (n == 0) {
            return nullptr;
        }

        // All allocators should contain an integer overflow check.
        // The Standardization Committee recommends that std::length_error
        // be thrown in the case of integer overflow.
        if (n > max_size()) {
            throw std::overflow_error("allocator<T>::allocate() – Integer overflow.");
        }

        void * const pv = m_->malloc(n * sizeof(T));
 
        // Allocators should throw std::bad_alloc in the case of memory allocation failure.
        if (pv == nullptr) {
            throw std::bad_alloc();
        }
        return static_cast<T *>(pv);
    }

    void deallocate(T * const p, const size_t n) const {
        // allocator wraps free().
        m_->free(p);
    }

    // The following will be the same for all allocators that ignore hints.
    template <typename U> T * allocate(const size_t n, const U * /* const hint */) const {
    	//TODO: realloc can be used?
        return allocate(n);
    }

private: //prohibit copy
    allocator& operator=(const allocator&);
    IMempool *m_;
};

template <typename T> void allocator<T>::destroy(T * const p) const {
    p->~T();
}
}

