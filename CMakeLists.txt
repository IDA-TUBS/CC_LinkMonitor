cmake_minimum_required(VERSION 3.0.0)
project(cc_linkmonitor VERSION 0.1.0)
set (CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

### cmake macros
include(${PROJECT_SOURCE_DIR}/cmake/dev/set_sources.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dev/generate_msvc_libraries.cmake)

### Build directory structure
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

### search for local boost
find_package(Boost 1.73.0 EXACT COMPONENTS thread log log_setup REQUIRED)

### Define library content
set(
    LIB_SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/cpp/cc_linkmonitor/messages.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/cc_linkmonitor/log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/cc_linkmonitor/mobility_management/linkMonitor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/cc_linkmonitor/mobility_management/connectionManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/cc_linkmonitor/mobility_management/mobilityClient.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/cc_linkmonitor/mobility_management/mobilityServer.cpp
)

## Define library content
add_library(cc_linkmonitor SHARED ${LIB_SRC_FILES})

target_include_directories(cc_linkmonitor
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(cc_linkmonitor PUBLIC boost_system boost_log boost_log_setup boost_thread pthread)

target_compile_definitions(cc_linkmonitor PUBLIC BOOST_LOG_DYN_LINK)

# Enable logging via -DLOG during build process
if(LOG)
    message("Logging activated")
    target_compile_definitions(cc_linkmonitor PUBLIC LOG_ON)
endif()

# Enable file log via -DFILE during build process
if(FILE)
    message("File logging activated")
    target_compile_definitions(cc_linkmonitor PUBLIC FILE_ON)
endif()

# Enable console log via -DCONSOLE during build process
if(CONSOLE)
    message("Console logging activated")
    target_compile_definitions(cc_linkmonitor PUBLIC CONSOLE_ON)
endif()

target_compile_options(cc_linkmonitor PUBLIC -g)

include(CMakePackageConfigHelpers)

### Add headers to system include dir 
set_public_headers_directory(${PROJECT_SOURCE_DIR}/include cc_linkmonitor
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers
    INSTALL
)

### Install lib
install(
    TARGETS cc_linkmonitor
    EXPORT cc_linkmonitorTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    EXPORT cc_linkmonitorTargets
    NAMESPACE cc_linkmonitor::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cc_linkmonitor
    FILE cc_linkmonitorTargets.cmake
)

### Install template implementation files
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} 
    FILES_MATCHING PATTERN "*.tpp"
)

### Add package info for cmake
configure_package_config_file( 
    "cc_linkmonitorConfig.cmake.in" 
    "cc_linkmonitorConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cc_linkmonitor
    PATH_VARS
        CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cc_linkmonitorConfigVersion.cmake
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion
)

### Install Config and ConfigVersion files
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/cc_linkmonitorConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/cc_linkmonitorConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cc_linkmonitor"
)

### Build Targets

file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/cc_linkmonitor/**/*.hpp
    ${PROJECT_SOURCE_DIR}/include/cc_linkmonitor/*.hpp
)

set(SRC_FILES 
    
  ${ALL_HEADERS}

  src/cpp/cc_linkmonitor/messages.cpp
  src/cpp/cc_linkmonitor/log.cpp

  src/cpp/cc_linkmonitor/mobility_management/linkMonitor.cpp
  src/cpp/cc_linkmonitor/mobility_management/connectionManager.cpp
  src/cpp/cc_linkmonitor/mobility_management/mobilityClient.cpp
  src/cpp/cc_linkmonitor/mobility_management/mobilityServer.cpp

)

# -------------------------------------------------------------------------- #
#                            Example Applications                            #
# -------------------------------------------------------------------------- #

# -------------------------------- Downlink -------------------------------- #
add_executable(rm_downlink examples/downlink/rm_node.cpp ${SRC_FILES})
target_link_libraries(rm_downlink cc_linkmonitor)

add_executable(client_downlink examples/downlink/client_node.cpp ${SRC_FILES})
target_link_libraries(client_downlink cc_linkmonitor)

# -------------------------------- Uplink -------------------------------- #
add_executable(rm_uplink examples/uplink/rm_node.cpp ${SRC_FILES})
target_link_libraries(rm_uplink cc_linkmonitor)

add_executable(client_uplink examples/uplink/client_node.cpp ${SRC_FILES})
target_link_libraries(client_uplink cc_linkmonitor)