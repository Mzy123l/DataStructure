cmake_minimum_required(VERSION 3.16)
project(DataStructure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 优化标志
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# 头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ===== Google Test via FetchContent =====
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)
# 避免 gtest 安装和警告
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ===== 测试目标 =====
function(add_ds_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE GTest::gtest GTest::gtest_main)
    target_compile_options(${name} PRIVATE -O3)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_ds_test(test_avl test/test_AVLTree.cpp)
add_ds_test(test_rb test/test_RBTree.cpp)
add_ds_test(test_bpt test/test_BPlusTree.cpp)
add_ds_test(test_trie test/test_Trie.cpp)
add_ds_test(test_sll test/test_singlyLinkedList.cpp)
add_ds_test(test_dll test/test_doublyLinkedList.cpp)
add_ds_test(test_uf test/test_UnionFind.cpp)

# ===== 性能基准测试 =====
add_executable(benchmark test/benchmark.cpp)
target_link_libraries(benchmark PRIVATE GTest::gtest GTest::gtest_main)
target_compile_options(benchmark PRIVATE -O3)
add_test(NAME benchmark COMMAND benchmark)
