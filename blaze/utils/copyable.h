//
// Created by xi on 19-1-29.
//

#ifndef BLAZE_COPYABLE_H
#define BLAZE_COPYABLE_H

namespace blaze
{

// Any derived class of copyable should be value type.

class copyable
{
public:
    copyable() = default;
    ~copyable() = default;
};

}

#endif //BLAZE_COPYABLE_H
