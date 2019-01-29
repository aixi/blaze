//
// Created by xi on 19-1-17.
//

#ifndef BLAZE_SINGLETON_H
#define BLAZE_SINGLETON_H

#include <assert.h>
#include <mutex>
#include <blaze/utils/Types.h>

namespace blaze
{

// thread safe singleton

namespace detail
{

template <typename T>
struct has_no_destroy
{
    // it could not detect inherited function

    template <typename C>
    static char test(decltype(&C::no_destroy));

    template <typename C>
    static int32_t test(...);
    const static bool value = (sizeof(test<T>(0)) == 1);
};

} // namespace detail

template <typename T>
class Singleton : public noncopyable
{
public:

    static T& Instance()
    {
        std::call_once(once_, Init);
        assert(value_ != nullptr);
        return *value_;
    }

    Singleton() = delete;
    ~Singleton() = delete;

private:

    static void Destroy()
    {
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        UnusedVariable(dummy);
        delete value_;
        value_ = nullptr;
    }

    static void Init()
    {
        // using template partial specialization to make use of another kind of ctor

        value_ = new T();
        if (!detail::has_no_destroy<T>::value)
        {
            ::atexit(Destroy);
        }
    }

private:
    static std::once_flag once_;
    static T* value_;
};

template <typename T>
std::once_flag Singleton<T>::once_;

template <typename T>
T* Singleton<T>::value_ = nullptr;

}

#endif //BLAZE_SINGLETON_H
