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
find_package(Bullet CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED) 
find_package(Stb REQUIRED)
find_package(Freetype REQUIRED)


include(FetchContent)

FetchContent_Declare(
    Utily
    GIT_REPOSITORY https://github.com/WillisMedwell/Utily.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Utily)

if(NOT MSVC)  
    target_compile_options(Engine PRIVATE -fexperimental-library -Wall -Wextra -Wpedantic)
endif()

target_precompile_headers(Engine PRIVATE include/EnginePch.hpp)

if(DEFINED EMSCRIPTEN)
    target_link_libraries(Engine PUBLIC 
        glm::glm assimp::assimp EnTT::EnTT Utily::Utily Freetype::Freetype lodepng ${BULLET_LIBRARIES} 
    )
    target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Stb_INCLUDE_DIR})
    target_compile_options(Engine PUBLIC 
        "$<$<CONFIG:Debug>:-O2;-g3;-sDEMANGLE_SUPPORT=1;-sFORCE_FILESYSTEM=1;-sASSERTIONS=1;-sSAFE_HEAP=1;-sSTACK_OVERFLOW_CHECK=2;-sNO_DISABLE_EXCEPTION_CATCHING;-Wno-unused-command-line-argument;-fno-inline-functions;-sEXIT_RUNTIME=1>"
        "$<$<CONFIG:Release>:-Oz;-sGL_FFP_ONLY;-msimd128;-mrelaxed-simd;-msse;-msse2;-msse3;-msse4.1;-Wno-unused-command-line-argument;-sFORCE_FILESYSTEM=1>"
    )
    target_link_options(Engine PUBLIC 
        "$<$<CONFIG:Debug>:-O2;-g3;-sDEMANGLE_SUPPORT=1;-sFORCE_FILESYSTEM=1;-sASSERTIONS=1;-sSAFE_HEAP=1;-sSTACK_OVERFLOW_CHECK=2;-sNO_DISABLE_EXCEPTION_CATCHING;-Wno-unused-command-line-argument;-fno-inline-functions;-sEXIT_RUNTIME=1>"
        "$<$<CONFIG:Release>:-Oz;-sGL_FFP_ONLY;-msimd128;-mrelaxed-simd;-msse;-msse2;-msse3;-msse4.1;-Wno-unused-command-line-argument;-sFORCE_FILESYSTEM=1>"
    )
    target_link_options(Engine PUBLIC -sUSE_WEBGL2=1 -sUSE_GLFW=3 -sFULL_ES3=1 -sFULL_ES2=1 -Wno-unused-command-line-argument -sALLOW_MEMORY_GROWTH)
else()
    find_package(OpenGL REQUIRED) 
    find_package(OpenAL CONFIG REQUIRED)
    find_package(GLFW3 CONFIG REQUIRED)
    find_package(GLEW REQUIRED)
    target_link_libraries(Engine PUBLIC 
        glm::glm assimp::assimp EnTT::EnTT Utily::Utily Freetype::Freetype lodepng ${BULLET_LIBRARIES} 
        OpenGL::GL OpenAL::OpenAL GLEW::GLEW glfw 
    )
    if(MSVC)
        target_compile_options(Engine PUBLIC
            "$<$<CONFIG:Debug>:/Od;/ZI;/RTCs;/MDd;/Ob0>"
            "$<$<CONFIG:Release>:/Ox;/Ot;/arch:AVX;/Oy;/Oi;>"
        )
    else()
        target_compile_options(Engine PRIVATE
            "$<$<CONFIG:Debug>:-O0;-g;-fno-omit-frame-pointer>"
            "$<$<CONFIG:Release>:-O3;-march=native>"
        )
    endif()

    target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OPENGL_INCLUDE_DIR} ${Stb_INCLUDE_DIR})
endif()
