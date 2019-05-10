add_library(protobuf_codec codec.cc)
target_link_libraries(protobuf_codec protobuf blaze z)

add_custom_command(OUTPUT query.pb.cc query.pb.h
        COMMAND protoc
        ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/query.proto - I ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS query.proto
        VERBATIM)