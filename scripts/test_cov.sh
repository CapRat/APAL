#!/bin/

cmake -E env CXXFLAGS="-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage" C_FLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs -ftest-coverage" cmake ..