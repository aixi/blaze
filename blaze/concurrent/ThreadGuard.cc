//
// Created by xi on 19-2-9.
//

#include <blaze/concurrent/ThreadGuard.h>

using namespace blaze;

ThreadGuard::ThreadGuard(std::thread&& thread, DtorAction action) :
    thread_(std::move(thread)),
    action_(action)
{}

ThreadGuard::~ThreadGuard()
{
    if (thread_.joinable())
    {
        if (action_ == DtorAction::join)
        {
            thread_.join();
        }
        else
        {
            thread_.detach();
        }
    }
}
