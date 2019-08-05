//
// Created by xi on 19-2-9.
//

#include <blaze/concurrent/ThreadGuard.h>

using namespace blaze;

ThreadGuard::ThreadGuard( DtorAction action, std::thread&& thread) :
    action_(action),
    thread_(std::move(thread))
{}

// Calling function detach() or join() of a unjoinable thread is UB
// Use ThreadGuard to ensure safely destruct of thread object
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
