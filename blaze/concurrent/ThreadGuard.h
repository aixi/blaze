//
// Created by xi on 19-2-9.
//

#ifndef BLAZE_THREADGUARD_H
#define BLAZE_THREADGUARD_H

#include <thread>

#include <blaze/utils/noncopyable.h>

namespace blaze
{

class ThreadGuard : public noncopyable
{
public:
    enum class DtorAction
    {
        join,
        detach
    };

    ThreadGuard(DtorAction action, std::thread&& thread);

    ~ThreadGuard();

    ThreadGuard(ThreadGuard&&) = default;

    ThreadGuard& operator=(ThreadGuard&&) = default;

    std::thread& get()
    {
        return thread_;
    }

private:
    DtorAction action_;
    std::thread thread_;
};

}

#endif //BLAZE_THREADGUARD_H
