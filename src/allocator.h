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
    virtual bool operator != (const IMempool &m) { return false; }
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
//alloc/free/realloc. heavily based on http://stackoverflow.com/questions/36517825/is-stephen-lavavejs-mallocator-the-same-in-c11/36521845#36521845
template<class T>
struct allocator {
    using value_type = T;
    using pointer = T*;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    
    allocator(IMempool *m) : m_(m) {}
    allocator()=delete;
    allocator(allocator const &m) {
        m_ = m.m_;
    }
    template<class U>
    allocator(allocator<U> const &m) noexcept {
        m_ = m.pool_p();
    }
    allocator& operator=(allocator const &m) {
        m_ = m.m_;
        return *this;
    }
    template<class U>
    allocator& operator=(allocator<U> const &m) noexcept {
        m_ = m.pool_p();
        return *this;
    }
    
    pointer allocate(std::size_t n) {
        if (std::size_t(-1) / sizeof(T) < n)
            throw std::bad_array_new_length(); // or something else
        if (!n) return nullptr; // zero means null, not throw
        if(auto*r= static_cast<pointer>(m_->malloc(n * sizeof(T))))
            return r;
        throw std::bad_alloc();
    }
    void deallocate(pointer p, std::size_t n) {
        m_->free(p);
    }
    template<class U>
    bool operator==(allocator<U> const& rhs) const {
        return m_->operator==(*rhs.m_);
    }
    template<class U>
    bool operator!=(allocator<U> const& rhs) const {
        return m_->operator!=(*rhs.m_);
    }
public:
    void free(void *p) {
        deallocate((pointer)p, sizeof(T));
    }
    void *malloc(size_t n) {
        return allocate(n);
    }
    void *realloc(void *p, size_t n) {
        return m_->realloc(p, n);
    }
    IMempool *pool_p() const { return (IMempool *)m_; }
protected:
    IMempool *m_;
};
}

