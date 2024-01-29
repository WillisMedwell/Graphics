project(Demos)

file(GLOB_RECURSE DEMOS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE DEMOS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

file(COPY ${CMAKE_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(Demos ${DEMOS_SOURCES} ${DEMOS_HEADERS})

if(DEFINED EMSCRIPTEN)        
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS='_main' -s EXPORT_NAME=\"'WasmModuleDemos'\"") 
    # Move index.html.
    file(COPY ${CMAKE_SOURCE_DIR}/index.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    
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
    target_compile_options(Demos PRIVATE -march=native)
endif()

target_link_libraries(Demos PUBLIC Engine)

target_include_directories(Demos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
