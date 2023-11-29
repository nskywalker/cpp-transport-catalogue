cmake_minimum_required(VERSION 3.23)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

add_executable(transport_catalogue
        geo.h
        main.cpp
        transport_catalogue.cpp domain.cpp domain.h json.cpp json.h json_reader.cpp json_reader.h map_renderer.cpp
        map_renderer.h request_handler.cpp request_handler.h svg.cpp svg.h geo.cpp json_builder.cpp json_builder.h graph.h ranges.h router.h transport_router.cpp transport_router.h)
