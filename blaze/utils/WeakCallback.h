//
// Created by xi on 19-2-9.
//

#ifndef BLAZE_WEAKCALLBACK_H
#define BLAZE_WEAKCALLBACK_H

#include <functional>
#include <memory>

#include <blaze/log/Logging.h>

namespace blaze
{

template <typename CLASS, typename... ARGS>
class WeakCallback
{
public:

    WeakCallback(const std::weak_ptr<CLASS>& object,
                 const std::function<void (CLASS*, ARGS...)>& functor) :
        object_(object),
        functor_(functor)
    {}

    // Default dtor, copy ctor and assignment are OK

    void operator()(ARGS&&... args) const
    {
        std::shared_ptr<CLASS> ptr(object_.lock());
        if (ptr)
        {
            functor_(ptr.get(), std::forward<ARGS>(args)...);
        }
        else
        {
            LOG_TRACE << "expired";
        }
    }

private:
    std::weak_ptr<CLASS> object_;
    std::function<void (CLASS*, ARGS...)> functor_;
};

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*functor)(ARGS...))
{
    return WeakCallback<CLASS, ARGS...>(object, functor);
}

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*functor)(ARGS...) const)
{
    return WeakCallback<CLASS, ARGS...>(object, functor);
}

} // namespace blaze

#endif //BLAZE_WEAKCALLBACK_H
