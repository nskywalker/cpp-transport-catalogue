cmake_minimum_required(VERSION 3.18)
project(transport_catalogue CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

set (CATALOGUE_FILES geo.h
        main.cpp
        transport_catalogue.cpp domain.cpp domain.h json.cpp json.h json_reader.cpp json_reader.h map_renderer.cpp
        map_renderer.h request_handler.cpp request_handler.h svg.cpp svg.h geo.cpp json_builder.cpp json_builder.h
        graph.h ranges.h router.h transport_router.cpp transport_router.h serialization.cpp serialization.h)

add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${CATALOGUE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(${PROJECT_NAME} "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
