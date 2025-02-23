option(example_auto_mount_daemon "Build example_auto_mount_daemon." ON)
message(STATUS "Build example_auto_mount_daemon '${example_auto_mount_daemon}'")

if(example_auto_mount_daemon)
  add_subdirectory( auto_mount_daemon )
endif()