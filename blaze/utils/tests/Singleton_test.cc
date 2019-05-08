//
// Created by xi on 19-1-16.
//

#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Singleton.h>

using namespace blaze;

class Test : public noncopyable
{
public:

    Test()
    {
        std::stringstream ss;
        ss << "thread_id=" << std::this_thread::get_id() << " constructing " << this << '\n';
        std::cout << ss.str();
    }

    ~Test()
    {
        std::stringstream ss;
        ss << "thread_id=" << std::this_thread::get_id() << " constructing " << this << ' ' << name_ << '\n';
        std::cout << ss.str();
    }

    const std::string& Name() const
    {
        return name_;
    }

    void SetName(const std::string& name)
    {
        name_ = name;
    }

private:
    std::string name_;
};

class TestNoDestroy : public noncopyable
{
public:

    // Tag member for Singleton<T>

    void no_destory();

    TestNoDestroy()
    {
        std::stringstream ss;
        ss << "thread_id=" << std::this_thread::get_id() << " constructing TestNoDestroy " << this << '\n';
        std::cout << ss.str();
    }

    ~TestNoDestroy()
    {
        std::stringstream ss;
        ss << "thread_id=" << std::this_thread::get_id() << " constructing TestNoDestroy " << this << '\n';
        std::cout << ss.str();
    }
};

void thread_func()
{
    std::stringstream ss;
    ss << "thread_id=" << std::this_thread::get_id() << ", "
        << &Singleton<Test>::GetInstance() << ' ' << Singleton<Test>::GetInstance().Name() << '\n';
    std::cout << ss.str();
    Singleton<Test>::GetInstance().SetName(" only once changed");
}

int main()
{
    Singleton<Test>::GetInstance().SetName("only once");
    std::thread thread1(thread_func);
    thread1.join();
    std::stringstream ss;
    ss << "thread_id=" << std::this_thread::get_id() << ", "
       << &Singleton<Test>::GetInstance() << Singleton<Test>::GetInstance().Name() << '\n';
    std::cout << ss.str();
    Singleton<TestNoDestroy>::GetInstance();
    printf("with valgrind, you should see %zd-byte memory leak.\n", sizeof(TestNoDestroy));
}
