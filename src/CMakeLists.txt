project(mount_daemon_lib)

SET(mount_daemon_lib_HEADERS
  mount_daemon.h
)

SET (mount_daemon_lib_SOURCES
  mount_daemon.cpp
)

ADD_LIBRARY(${PROJECT_NAME} ${mount_daemon_lib_SOURCES} ${mount_daemon_lib_HEADERS} )
# ADD_LIBRARY(core::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

find_package(PkgConfig)
pkg_check_modules(LIBUDEV libudev IMPORTED_TARGET)

target_link_libraries(${PROJECT_NAME} PkgConfig::LIBUDEV)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

SET_TARGET_PROPERTIES(${PROJECT_NAME}
  PROPERTIES
  LINKER_LANGUAGE CXX
  COMPILE_FLAGS "-std=c++17"
)