project(Test)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

file(COPY ${CMAKE_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(Test ${TEST_SOURCES} ${TEST_HEADERS})

target_precompile_headers(Test PRIVATE include/TestPch.hpp ${CMAKE_CURRENT_INCLUDE_DIR}/Engine.hpp)

if(DEFINED EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS='_main' --closure 1") 
        
    # Asset embedding.
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
    file(GLOB_RECURSE ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
    set(PRELOAD_FILES "")
    foreach(FILE ${ASSET_FILES})
        file(RELATIVE_PATH REL_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
        set(PRELOAD_FILES "${PRELOAD_FILES} --preload-file ${REL_FILE}")
    endforeach()
    message(STATUS "PRELOAD FILES: \"${PRELOAD_FILES}\"")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PRELOAD_FILES}")
else()
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

enable_testing()
find_package(GTest CONFIG REQUIRED)
target_link_libraries(Test PRIVATE Engine GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(AllTestsInMain main)

target_include_directories(Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
