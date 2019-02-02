//
// Created by xi on 19-1-14.
//

#ifndef BLAZE_CALLBACKS_H
#define BLAZE_CALLBACKS_H

#include <functional>

namespace blaze
{

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace net
{

using TimerCallback = std::function<void()>;

} // namespace net
} // namespace blaze

#endif //BLAZE_CALLBACKS_H
