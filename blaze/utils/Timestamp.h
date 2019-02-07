//
// Created by xi on 19-1-17.
//

#ifndef BLAZE_TIMESTAMP_H
#define BLAZE_TIMESTAMP_H

#include <stdint.h>
#include <string>
#include <algorithm>
#include <blaze/utils/Types.h>

namespace blaze
{

class Timestamp
{
public:

    const static int kMicroSecondsPerSecond = 1000 * 1000;

    // construct an invalid Timestamp

    Timestamp() :
        micro_seconds_since_epoch_(0)
    {}

    explicit Timestamp(int64_t micro_seconds_since_epoch) :
        micro_seconds_since_epoch_(micro_seconds_since_epoch)
    {}

    void Swap(Timestamp& other)
    {
        std::swap(micro_seconds_since_epoch_, other.micro_seconds_since_epoch_);
    }

    static Timestamp Now();

    static Timestamp Invalid()
    {
        return Timestamp();
    }

    bool IsValid() const
    {
        return micro_seconds_since_epoch_ > 0;
    }

    // default copy ctor / assignment dtor are OK

    std::string ToString() const;

    std::string ToFormattedString(bool show_microseconds = true) const;

    // internal usage

    int64_t MicrosecondsSinceEpoch() const
    {
        return micro_seconds_since_epoch_;
    }

    time_t SecondsSinceEpoch() const
    {
        return static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
    };

    static Timestamp FromUnixTime(time_t t)
    {
        return FromUnixTime(t, 0);
    }

    static Timestamp FromUnixTime(time_t t, int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }

private:

    int64_t micro_seconds_since_epoch_;
};

// pass by value is OK, sizeof(Timestamp) = 8

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.MicrosecondsSinceEpoch() < rhs.MicrosecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.MicrosecondsSinceEpoch() == rhs.MicrosecondsSinceEpoch();
}

// @return (high - low) in seconds

inline double TimeDifference(Timestamp low, Timestamp high)
{
    int64_t diff = high.MicrosecondsSinceEpoch() - low.MicrosecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}


inline Timestamp AddTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.MicrosecondsSinceEpoch() + delta);
}

}

#endif //BLAZE_TIMESTAMP_H
