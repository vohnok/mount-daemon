project(example_auto_mount_daemon)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} mount_daemon_lib)