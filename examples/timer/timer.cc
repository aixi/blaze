//
// Created by xi on 2019/8/21.
//
#include <iostream>
#include <blaze/utils/Types.h>
#include <blaze/net/EventLoop.h>

using namespace blaze;
using namespace blaze::net;

// Print 5 integers per second

class Printer
{
public:
    Printer(EventLoop* loop) :
        loop_(loop),
        count_(0)
    {
        loop_->RunAfter(1, [this](){Print();});
    }

    ~Printer()
    {
        std::cout << "Final count is " << count_ << std::endl;
    }

    void Print()
    {
        if (count_ < 5)
        {
            std::cout << count_ << '\n';
            ++count_;
            loop_->RunAfter(1, [this](){Print();});
        }
        else
        {
            loop_->Quit();
        }
    }
    DISABLE_COPY_AND_ASSIGN(Printer);
private:
    EventLoop* loop_;
    int count_;
};

int main()
{
    EventLoop loop;
    Printer printer(&loop);
    loop.Loop();
    return 0;
}