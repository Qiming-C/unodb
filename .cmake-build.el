((cmake-build-cmake-profiles
  (release "-DCMAKE_BUILD_TYPE=Release")
  (release-asan "-DCMAKE_BUILD_TYPE=Release -DSANITIZE=ON")

  (debug "-DCMAKE_BUILD_TYPE=Debug")
  (debug-asan "-DCMAKE_BUILD_TYPE=Debug -DSANITIZE=ON")
  (debug-tsan "-DCMAKE_BUILD_TYPE=Debug -DSANITIZE_THREAD=ON")

  (gcc-release "-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9")
  (gcc-release-asan "-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 -DSANITIZE=ON")
  (gcc-debug "-DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9")
  (gcc-debug-asan "-DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 -DSANITIZE=ON")
  (gcc-debug-tsan "-DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 -DSANITIZE_THREAD=ON")

  (llvm-release "-DCMAKE_PREFIX_PATH=/usr/local/opt/llvm -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++")
  (llvm-debug "-DCMAKE_PREFIX_PATH=/usr/local/opt/llvm -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++")
  (llvm-debug-asan "-DCMAKE_PREFIX_PATH=/usr/local/opt/llvm -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ -DSANITIZE=ON")
  (llvm-debug-tsan "-DCMAKE_PREFIX_PATH=/usr/local/opt/llvm -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ -DSANITIZE_THREAD=ON"))

 (cmake-build-run-configs
  (test
   (:build "")
   (:run "" "make" "test"))))
