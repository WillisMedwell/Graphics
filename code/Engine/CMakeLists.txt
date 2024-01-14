project(Engine)

file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

# For visual studio to recognise it...
file(COPY ${CMAKE_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

find_package(glm CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    Utily
    GIT_REPOSITORY https://github.com/WillisMedwell/Utily.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Utily)

if(DEFINED EMSCRIPTEN)
    target_include_directories(Engine PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(Engine PUBLIC 
        glm::glm assimp::assimp EnTT::EnTT Utily::Utily ${BULLET_LIBRARIES}
    )
    target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
else()
    target_include_directories(Engine PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    find_package(OpenGL REQUIRED) 
    find_package(GLFW3 CONFIG REQUIRED)
    find_package(GLEW REQUIRED)
    target_link_libraries(Engine PUBLIC 
        glm::glm assimp::assimp EnTT::EnTT Utily::Utily ${BULLET_LIBRARIES} 
        OpenGL::GL GLEW::GLEW glfw
    )
    target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OPENGL_INCLUDE_DIR})
endif()

if(NOT MSVC)  
    target_compile_options(Engine PRIVATE -fexperimental-library -Wall -Wextra -Wpedantic)
endif()