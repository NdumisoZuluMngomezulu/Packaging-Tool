cmake_minimum_required(VERSION 3.16)
project(mytool VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "File checksum and entropy analyzer")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)       # export only MYTOOL_API symbols
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ---- core shared library: libmytool.so.1 -----------------------------------
add_library(mytool SHARED src/sha256.cpp src/crc32.cpp src/analyze.cpp)
add_library(mytool::mytool ALIAS mytool)
target_include_directories(mytool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(mytool PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(mytool PROPERTIES
    VERSION   ${PROJECT_VERSION}   # libmytool.so.1.0.0
    SOVERSION 1)                   # libmytool.so.1  (the ABI number)

# ---- CLI frontend ----------------------------------------------------------
add_executable(mytool-cli cli/main.cpp)
target_link_libraries(mytool-cli PRIVATE mytool::mytool)
target_compile_options(mytool-cli PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(mytool-cli PROPERTIES OUTPUT_NAME mytool)

# ---- tests -----------------------------------------------------------------
enable_testing()
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE mytool::mytool)
add_test(NAME core COMMAND test_core)

# ---- install ---------------------------------------------------------------
install(TARGETS mytool EXPORT mytoolTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS mytool-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/mytool DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES man/mytool.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

set(MYTOOL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/mytool)
install(EXPORT mytoolTargets NAMESPACE mytool:: DESTINATION ${MYTOOL_CMAKEDIR})
configure_package_config_file(cmake/mytoolConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/mytoolConfig.cmake
    INSTALL_DESTINATION ${MYTOOL_CMAKEDIR})
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/mytoolConfigVersion.cmake
    COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mytoolConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/mytoolConfigVersion.cmake
        DESTINATION ${MYTOOL_CMAKEDIR})

configure_file(cmake/mytool.pc.in ${CMAKE_CURRENT_BINARY_DIR}/mytool.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mytool.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
