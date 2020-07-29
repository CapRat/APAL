#!/bin/
mkdir build
cd build
cmake -E env CXXFLAGS="-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage" C_FLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs -ftest-coverage -lgcov" cmake ..
ctest .
gcovr --fail-under-line 80 -s  --exclude-directories "test|examples"  -r /mnt/c/Users/Benjamin/Source/Repos/xplug/  .