add_executable(pingpong_client client.cc)
target_link_libraries(pingpong_client blaze)

add_executable(pingpong_server server.cc)
target_link_libraries(pingpong_server blaze)

#add_executable(pingpong_bench examples/pingpong/bench.cc)
#target_link_libraries(pingpong_bench blaze)