//
// Created by xi on 19-1-21.
//

#include <stdio.h>
#include <blaze/utils/Exception.h>

struct Bar
{
    void test()
    {
        throw blaze::Exception("oops");
    }
};

void foo()
{
    Bar b;
    b.test();
}

int main()
{
    try 
    {
        foo();
    }
    catch (const blaze::Exception& e)
    {
        printf("reason: %s\n", e.what());
        printf("stack trace: %s\n", e.StackTrace());
    }
}