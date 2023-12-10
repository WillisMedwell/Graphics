project(Engine)


file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

# For visual studio to recognise it...
file(COPY ${CMAKE_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

find_package(glm CONFIG REQUIRED)


if(DEFINED EMSCRIPTEN)
    add_definitions(-DBUILD_SHARED_LIBS=OFF)
    add_definitions(-DASSIMP_BUILD_TESTS=OFF)
    add_definitions(-DASSIMP_BUILD_TESTS=OFF)
    add_definitions(-DASSIMP_BUILD_GLTF_IMPORTER=ON)
    add_definitions(-DASSIMP_BUILD_DRACO=ON)
    
    add_subdirectory(lib/Assimp)

    target_include_directories(Engine PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        lib/Assimp/include
        ${CMAKE_BINARY_DIR}/assimp/include # config file
    )


else()
    find_package(assimp CONFIG REQUIRED)
    target_include_directories(Engine PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Engine PUBLIC glm::glm assimp::assimp)

target_compile_options(Engine PRIVATE -fexperimental-library)