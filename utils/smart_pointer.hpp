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
    static smart_pointer null_pointer;

public:
    smart_pointer(object *obj = NULL, intptr_t array = false)
        : __is_array__(array), __obj__(NULL), __fastmutex__(NULL)
    {
        if (NULL == obj) {
            return;
        }

        try {
            __obj__ = dynamic_cast<TYPE *>(obj);
        } catch (...) { // std::bad_cast
            assert(0);
        }
        this->__fastmutex__ = __obj__->__fastmutex__;

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
    TYPE &operator [](std::size_t index)
    {
        if (!__is_array__) {
            return __obj__[0];
        } else {
            return __obj__[index];
        }
    }
    void release(void)
    {
        __release__();

        return;
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

template <typename TYPE> typename e7::utils::smart_pointer<TYPE>
e7::utils::smart_pointer<TYPE>::null_pointer;
