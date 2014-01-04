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
        : __ref_count__(0), __count_mutex__(NULL)
    {
        __count_mutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__count_mutex__, NULL);
    }
    explicit object(object const &other)
        : __ref_count__(0), __count_mutex__(NULL)
    {
        __count_mutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__count_mutex__, NULL);
    }
    virtual ~object(void)
    {}

private:
    // you should use std::vector as a single object
    void *operator new[](size_t size);
    void operator delete[](void *p);

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
    pthread_mutex_t *__count_mutex__;
};

template <typename TYPE> class e7::utils::smart_pointer
{
public:
    static smart_pointer null_pointer;

public:
    smart_pointer(object *obj = NULL)
        : __obj__(NULL), __count_mutex__(NULL)
    {
        if (NULL == obj) {
            return;
        }

        try {
            __obj__ = dynamic_cast<TYPE *>(obj);
        } catch (...) { // std::bad_cast
            assert(0);
        }
        this->__count_mutex__ = __obj__->__count_mutex__;

        assert(0 == pthread_mutex_lock(this->__count_mutex__));
        __obj__->__ref_increase__();
        assert(0 == pthread_mutex_unlock(this->__count_mutex__));

        return;
    }

    explicit smart_pointer(smart_pointer const &other)
        : __obj__(NULL), __count_mutex__(NULL)
    {
        *this = other;
    }

    smart_pointer const &operator =(smart_pointer const &other)
    {
        __release__();

        if (other.not_null()) {
            this->__obj__ = other.__obj__;
            this->__count_mutex__ = other.__count_mutex__;

            assert(0 == pthread_mutex_lock(this->__count_mutex__));
            this->__obj__->__ref_increase__();
            assert(0 == pthread_mutex_unlock(this->__count_mutex__));
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
    void release(void)
    {
        __release__();

        return;
    }

    void copy(smart_pointer const &other)
    {
        __release__();
        if (other.not_null()) {
            __obj__ = new TYPE(*static_cast<smart_pointer>(other));
            __count_mutex__ = __obj__->__count_mutex__;

            assert(0 == pthread_mutex_lock(__count_mutex__));
            __obj__->__ref_increase__();
            assert(0 == pthread_mutex_unlock(__count_mutex__));
        }

        return;
    }

private:
    void __release__(void)
    {
        intptr_t ref_count = 0;

        if (NULL == this->__obj__) {
            return;
        }

        assert(0 == pthread_mutex_lock(this->__count_mutex__));
        __obj__->__ref_decrease__();
        ref_count = __obj__->__get_ref_count__();
        if (0 == ref_count) {
            delete __obj__;
        }
        __obj__ = NULL;
        assert(0 == pthread_mutex_unlock(this->__count_mutex__));

        if (0 == ref_count) {
            pthread_mutex_destroy(this->__count_mutex__);
            free(this->__count_mutex__);
        }
        this->__count_mutex__ = NULL;

        return;
    }

private:
    TYPE *__obj__;
    pthread_mutex_t *__count_mutex__;
};

template <typename TYPE> typename e7::utils::smart_pointer<TYPE>
e7::utils::smart_pointer<TYPE>::null_pointer;
