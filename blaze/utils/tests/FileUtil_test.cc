//
// Created by xi on 19-1-18.
//

#include <blaze/utils/FileUtil.h>

#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include <string>

using namespace blaze;

int main()
{
    std::string result;
    int64_t size = 0;
    int err = FileUtil::ReadFile("/proc/self", 1024, &result, &size);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/proc/self", 1024, &result, nullptr);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/proc/self/cmdline", 1024, &result, &size);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/dev/null", 1024, &result, &size);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/dev/zero", 1024, &result, &size);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/notexist", 1024, &result, &size);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/dev/null", 1024, &result, &size);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);
    err = FileUtil::ReadFile("/dev/null", 1024, &result, nullptr);
    printf("%d %zd %" PRId64 "\n", err, result.size(), size);

}