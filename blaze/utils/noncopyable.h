//
// Created by xi on 19-1-8.
//

#ifndef BLAZE_NONCOPYABLE_H
#define BLAZE_NONCOPYABLE_H

namespace blaze
{

class noncopyable
{
public:
    noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

}

#endif //BLAZE_NONCOPYABLE_H
