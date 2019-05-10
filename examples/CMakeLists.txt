add_executable(blaze_simple simple.cc)
target_link_libraries(blaze_simple blaze)

add_subdirectory(chat)
add_subdirectory(daytime)
add_subdirectory(echo)
add_subdirectory(pingpong)
add_subdirectory(protobuf)