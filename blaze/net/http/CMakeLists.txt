add_library(blaze_http STATIC
        HttpContext.cc
        HttpResponse.cc
        HttpRequest.cc
        HttpServer.cc)
target_link_libraries(blaze_http blaze)