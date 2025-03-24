cmake_minimum_required(VERSION 2.8...3.5)

project(Assignment)

add_executable(main.out main.cpp
        loadTGA.h)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries( main.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )
