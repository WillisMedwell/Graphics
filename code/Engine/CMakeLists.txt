project(Engine)


file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

# For visual studio to recognise it...
file(COPY ${CMAKE_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

find_package(glm CONFIG REQUIRED)



if(DEFINED EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ASSIMP_BUILD_GLTF_IMPORTER OFF)
    set(ASSIMP_BUILD_DRACO OFF)
    set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
    set(ASSIMP_BUILD_FBX_IMPORTER ON)

    add_subdirectory(lib/Assimp)
    target_include_directories(Engine PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        lib/Assimp/include
        ${CMAKE_BINARY_DIR}/assimp/include # config file
    )
    target_link_libraries(Engine PUBLIC glm::glm assimp::assimp)
    target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
else()
    find_package(OpenGL REQUIRED) 
    find_package(assimp CONFIG REQUIRED)
    find_package(GLFW3 CONFIG REQUIRED)
    find_package(GLEW REQUIRED)

    target_include_directories(Engine PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(Engine PUBLIC glm::glm assimp::assimp OpenGL::GL GLEW::GLEW glfw)
    target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OPENGL_INCLUDE_DIR})
endif()




target_compile_options(Engine PRIVATE -fexperimental-library)