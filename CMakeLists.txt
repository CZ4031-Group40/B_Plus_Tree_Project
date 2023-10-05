cmake_minimum_required(VERSION 3.26)
project(B_Plus_Tree_Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(B_Plus_Tree_Project src/main.cpp
        src/Storage.h
        src/Storage.cpp
        src/BPlusTree.cpp
        src/BPlusTree.h
        src/DataStruct.h)


add_executable(exp1and2 src/experiment1and2.cpp
        src/Storage.h
        src/Storage.cpp
        src/BPlusTree.cpp
        src/BPlusTree.h
        src/DataStruct.h)
