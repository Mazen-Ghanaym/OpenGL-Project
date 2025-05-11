cmake_minimum_required(VERSION 3.10)
project(flappy-ball)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set optimization flags for Release build
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Set OpenGL preference to GLVND
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

# Windows-specific configuration
if(WIN32)
    # Set variables for finding dependencies
    set(OPENGL_ROOT_DIR "$ENV{PROGRAMFILES}/OpenGL" CACHE PATH "OpenGL root directory")
    set(GLUT_ROOT_DIR "$ENV{PROGRAMFILES}/freeglut" CACHE PATH "FreeGLUT root directory")
    set(OPENAL_ROOT_DIR "$ENV{PROGRAMFILES}/OpenAL" CACHE PATH "OpenAL root directory")
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenAL REQUIRED)

# Platform-specific configurations
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

# Add executable
file(GLOB_RECURSE SOURCE_FILES
    "src/*.cpp"
    "src/*.c"
)

add_executable(flappy-ball ${SOURCE_FILES})

# Link libraries
target_link_libraries(flappy-ball PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${OPENAL_LIBRARY}
)

# Enable maximum warning level
if(MSVC)
    target_compile_options(flappy-ball PRIVATE /W4)
else()
    target_compile_options(flappy-ball PRIVATE -Wall -Wextra)
endif()

# Include directories
target_include_directories(flappy-ball PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/assets
)

# Copy assets to build directory
add_custom_command(TARGET flappy-ball POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:flappy-ball>/assets
)
