find . -type f -not -name 'build.sh' -exec rm -f {} +
cmake -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug ..