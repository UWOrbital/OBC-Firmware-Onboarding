cd build
cmake .. -DCMAKE_BUILD_TYPE=Test -DCMAKE_POLICY_VERSION_MINIMUM=3.5 
cmake --build .
ctest --verbose