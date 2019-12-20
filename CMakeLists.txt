cmake_minimum_required(VERSION 3.15)
project(Graphics-Lab2)

set(CMAKE_CXX_STANDARD 14)

add_executable(main main.cpp obj_loader.cpp obj_loader.h CVector.h CVector.cpp CMatrix.h CMatrix.cpp)
target_link_libraries(main -lglut -lGL -lGLU -lSOIL)
