cmake_minimum_required(VERSION 2.8.3)
project(wpics513-text-chatroulette)

include_directories("${PROJECT_BINARY_DIR}")
 
add_executable(test_client test_client.c)
add_executable(tcr_server tcr_server.c)
