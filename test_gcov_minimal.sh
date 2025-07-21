#!/bin/bash
set -e
set -x

dir="build/src/CMakeFiles/heimdall-core.dir/common"
cd "$dir"
gcov -o . MetadataExtractor.cpp.gcda /home/tbakker/code/heimdall/src/common/MetadataExtractor.cpp
ls -l *.gcov 