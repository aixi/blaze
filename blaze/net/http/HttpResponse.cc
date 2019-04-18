//
// Created by xi on 19-4-17.
//

#include <stdio.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/http/HttpResponse.h>

using namespace blaze;
using namespace blaze::net;

void HttpResponse::AppendToBuffer(Buffer* output) const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP1.1 %d", status_code_);
}