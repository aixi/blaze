//
// Created by xi on 19-1-18.
//
#include <stdio.h>
#include <vector>
#include <blaze/utils/Timestamp.h>

using namespace blaze;

void PassByConstReference(const Timestamp& x)
{
    //printf("%s\n", x.ToString().c_str());
}

void PassByValue(Timestamp x)
{
    //printf("%s\n", x.ToString().c_str());
}

void Benchmark()
{
    const int kNumbers = 1000'000'000;
    Timestamp start = Timestamp::Now();
    for (int i = 0; i < kNumbers; ++i)
    {
        PassByConstReference(Timestamp::Now());
    }
    Timestamp end = Timestamp::Now();
    printf("PassByConstReference time: %f\n", TimeDifference(start, end));

    Timestamp start2 = Timestamp::Now();
    for (int i = 0; i < kNumbers; ++i)
    {
        PassByValue(Timestamp::Now());
    }
    Timestamp end2 = Timestamp::Now();
    printf("PassByValue time: %f\n", TimeDifference(start2, end2));
}

int main()
{
    Benchmark();
    return 0;
}