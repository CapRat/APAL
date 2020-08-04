#!/bin/sh
mkdir build
cd build
cmake -E env CXXFLAGS="-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage" C_FLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs -ftest-coverage -lgcov" cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
ctest .
gcovr --fail-under-line 80 -s -j4  -r $(pwd)/../../  $gcovr --fail-under-line 80 -s -e '.*/deps/.*$' -e '.*(CMake|(T|t)tl|string_view).*$'  --exclude-directories "test/|examples/|deps/"   -r $(pwd)/..  $(pwd)