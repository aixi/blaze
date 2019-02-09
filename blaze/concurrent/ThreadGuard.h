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

    ThreadGuard(std::thread&& thread, DtorAction action);

    ~ThreadGuard();

    std::thread& thread()
    {
        return thread_;
    }

private:
    std::thread thread_;
    DtorAction action_;
};

}

#endif //BLAZE_THREADGUARD_H
