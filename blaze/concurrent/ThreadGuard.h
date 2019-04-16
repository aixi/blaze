//
// Created by xi on 19-2-9.
//

#ifndef BLAZE_THREADGUARD_H
#define BLAZE_THREADGUARD_H

#include <thread>
#include <blaze/utils/Types.h>

namespace blaze
{

class ThreadGuard
{
public:
    enum class DtorAction
    {
        join,
        detach
    };

    // thread is movable, but not copyable, so as atomic

    ThreadGuard(DtorAction action, std::thread&& thread);

    ~ThreadGuard();

    std::thread& get()
    {
        return thread_;
    }

    // non-copyable
    DISABLE_COPY_AND_ASSIGN(ThreadGuard);
    // movable
    DISABLE_MOVE_AND_ASSIGN(ThreadGuard);

private:
    DtorAction action_;
    std::thread thread_;
};

}

#endif //BLAZE_THREADGUARD_H
