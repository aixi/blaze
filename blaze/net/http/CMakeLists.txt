add_library(blaze_http STATIC
        HttpContext.cc
        HttpResponse.cc
        HttpRequest.cc
        HttpServer.cc)
target_link_libraries(blaze_http blaze)

set(HEADERS
        HttpServer.h
        HttpRequest.h
        HttpResponse.h
        HttpContext.h)

install(FILES ${HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include/net/http)

add_subdirectory(tests)