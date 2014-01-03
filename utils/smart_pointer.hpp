#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <pthread.h>

namespace e7
{
    namespace utils
    {
        class object;
        template <typename TYPE> class smart_pointer;
    }
}

class e7::utils::object
{
     template <typename TYPE> friend class smart_pointer;

public:
    explicit object(void)
        : __ref_count__(0), __fastmutex__(NULL)
    {
        __fastmutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__fastmutex__, NULL);
    }
    virtual ~object(void)
    {}

public:
    static void *operator new(std::size_t size)
    {
        return calloc(1, size);
    }
    static void operator delete(void *p)
    {
        free(p);

        return;
    }
    static void *operator new[](std::size_t size, std::size_t obj_size)
    {
        intptr_t p = reinterpret_cast<intptr_t>(calloc(1, size));
        if (size > obj_size) {
            p |= 1;
        }

        return reinterpret_cast<void *>(p);
    }
    static void operator delete[](void *p)
    {
        intptr_t addr = reinterpret_cast<intptr_t>(p);
        addr &= ((~0) - 1);
        free(reinterpret_cast<void *>(addr));

        return;
    }

private:
    // just for smart_pointer
    void __ref_increase__(void)
    {
        ++__ref_count__;
    }

    intptr_t __get_ref_count__(void) const
    {
        return __ref_count__;
    }

    void __ref_decrease__(void)
    {
        --__ref_count__;
    }

private:
    intptr_t __ref_count__;
    pthread_mutex_t *__fastmutex__;
};

template <typename TYPE> class e7::utils::smart_pointer
{
public:
    smart_pointer(void)
        : __is_array__(false), __obj__(NULL), __fastmutex__(NULL)
    {}
    smart_pointer(object *obj)
        : __is_array__(false), __obj__(NULL), __fastmutex__(NULL)
    {
        object *p = NULL;

        if (NULL == obj) {
            return;
        }

        p = reinterpret_cast<object *>(
            (reinterpret_cast<intptr_t>(obj) & ((~0) - 1))
        );
        try {
            __obj__ = dynamic_cast<TYPE *>(p);
        } catch (...) { // std::bad_cast
            assert(0);
        }
        this->__fastmutex__ = __obj__->__fastmutex__;
        __is_array__ = !(obj == __obj__);

        assert(0 == pthread_mutex_lock(this->__fastmutex__));
        __obj__->__ref_increase__();
        assert(0 == pthread_mutex_unlock(this->__fastmutex__));

        return;
    }

    explicit smart_pointer(smart_pointer const &other)
        : __is_array__(false), __obj__(NULL), __fastmutex__(NULL)
    {
        *this = other;
    }

    smart_pointer const &operator =(smart_pointer const &other)
    {
        __release__();

        if (other.not_null()) {
            this->__is_array__ = other.__is_array__;
            this->__obj__ = other.__obj__;
            this->__fastmutex__ = other.__fastmutex__;

            assert(0 == pthread_mutex_lock(this->__fastmutex__));
            this->__obj__->__ref_increase__();
            assert(0 == pthread_mutex_unlock(this->__fastmutex__));
        }

        return *this;
    }

    virtual ~smart_pointer(void)
    {
        __release__();
    }

public:
    intptr_t operator ==(void *p) const
    {
        return p == __obj__;
    }
    intptr_t operator !=(void *p) const
    {
        return p != __obj__;
    }
    intptr_t operator ==(smart_pointer const &other) const
    {
        return other.__obj__ == this->__obj__;
    }
    intptr_t operator !=(smart_pointer const &other) const
    {
        return other.__obj__ != this->__obj__;
    }
    intptr_t is_null(void) const
    {
        return NULL == __obj__;
    }
    intptr_t not_null(void) const
    {
        return NULL != __obj__;
    }
    TYPE *operator ->(void)
    {
        return __obj__;
    }
    TYPE &operator *(void)
    {
        return *__obj__;
    }

private:
    void __release__(void)
    {
        intptr_t ref_count = 0;

        if (NULL == this->__obj__) {
            return;
        }

        assert(0 == pthread_mutex_lock(this->__fastmutex__));
        __obj__->__ref_decrease__();
        ref_count = __obj__->__get_ref_count__();
        if (0 == ref_count) {
            if (__is_array__) {
                delete[] __obj__;
            } else {
                delete __obj__;
            }
        }
        __obj__ = NULL;
        assert(0 == pthread_mutex_unlock(this->__fastmutex__));

        if (0 == ref_count) {
            pthread_mutex_destroy(this->__fastmutex__);
            free(this->__fastmutex__);
        }
        this->__fastmutex__ = NULL;

        return;
    }

private:
    intptr_t __is_array__;
    TYPE *__obj__;
    pthread_mutex_t *__fastmutex__;
};
