#!/bin/bash
export CXXFLAGS=-static
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-g++

"${@}"
