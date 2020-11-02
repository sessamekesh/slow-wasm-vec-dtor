cmake_minimum_required (VERSION 3.10)

project ("SlowVecDtorExperiment")

set(CMAKE_CXX_STANDARD 17)

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_EXE_LINKER_FLAGS "-s WASM=1 -s ENVIRONMENT=web,worker")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -s USE_WEBGL2=1 -s USE_GLFW=3 -s FETCH=1 -s INITIAL_MEMORY=268435456")
endif()

# Add source to this project's executable.
add_executable (vec_benchmark main.cc)