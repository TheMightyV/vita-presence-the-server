cmake_minimum_required(VERSION 3.2.0)
project(vita-presence-the-server)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)


add_executable(vita-presence-the-server src/main.cpp lib/inih/cpp/INIReader.cpp lib/inih/ini.c)

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(WIN32)

target_include_directories(${PROJECT_NAME} PRIVATE lib)
target_include_directories(${PROJECT_NAME} PRIVATE lib/discord-rpc/include)
target_include_directories(${PROJECT_NAME} PRIVATE lib/tacopie/includes)

add_subdirectory(lib/tacopie)
add_subdirectory(lib/discord-rpc)

target_link_libraries(${PROJECT_NAME} PRIVATE discord-rpc)
target_link_libraries(${PROJECT_NAME} PRIVATE tacopie)
#target_link_libraries(${PROJECT_NAME} PRIVATE Threads:Threads)

