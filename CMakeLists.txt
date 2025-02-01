cmake_minimum_required(VERSION 3.22)

project(project VERSION 0.1.0 
  LANGUAGES ASM C
  DESCRIPTION "A template project for C libraries"
)

option(BUILD_TESTING "Testing build" ON)

# Set C standard and flags
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerStandards)
include(Functions)

# Get all source files except main.c
get_all_sources(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.c$")

# Main Library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Change include directories to use generator expressions for installation
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
set_compiler_options(${PROJECT_NAME})

# Add installation rules for the main library
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install header files, maintaining directory structure
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Create and install CMake configuration files
include(CMakePackageConfigHelpers)

# Generate the config file that makes your package findable
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

# Generate version file for compatibility checking
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# Install the export targets
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE Anvil::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

if (BUILD_TESTING)
# Find Python
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Create shared library for testing
  add_library(${PROJECT_NAME}_test SHARED ${SOURCES})
  target_include_directories(${PROJECT_NAME}_test PUBLIC 
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  target_compile_definitions(${PROJECT_NAME}_test PRIVATE 
    BUILDING_TESTS
    _DEBUG
  )

  set_target_properties(${PROJECT_NAME}_test PROPERTIES
      C_VISIBILITY_PRESET default
      VISIBILITY_INLINES_HIDDEN OFF
  )

  set_compiler_options(${PROJECT_NAME}_test)
  target_compile_options(${PROJECT_NAME}_test PRIVATE
    -O0
    -g3
  )
endif()

# Create symlink for compile_commands.json in project root
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
)
