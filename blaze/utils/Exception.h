//
// Created by xi on 19-1-21.
//

#ifndef BLAZE_EXCEPTION_H
#define BLAZE_EXCEPTION_H

#include <string>
#include <exception>

namespace blaze
{

class Exception : public std::exception
{

public:
    explicit Exception(const char* what);

    explicit Exception(const std::string& what);

    virtual ~Exception() noexcept;

    const char* what() const noexcept override;

    const char* StackTrace() const noexcept;


private:

    void FillStackTrace();

private:
    std::string message_;
    std::string stack_;
};

}

#endif //BLAZE_EXCEPTION_H
