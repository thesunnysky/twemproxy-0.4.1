cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(tproxy-0.4.1 VERSION 0.4)

set(CMAKE_BUILD_TYPE "DEBUG")

message(STATUS "Host is: ${CMAKE_HOST_SYSTEM}. Build target is: ${CMAKE_SYSTEM}")
get_filename_component(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
message(STATUS "Project root directory is: ${SRC_ROOT}")

#aux_source_directory(src SRC_PROXY)

add_compile_definitions(HAVE_EPOLL)
add_compile_definitions(HAVE_STATS)
add_compile_definitions(NC_DEBUG_LOG)
add_compile_definitions(NC_VERSION_STRING="0.4.1")
add_compile_definitions(IOV_MAX=128)

set(SRC_PROXY
        src/nc.c src/nc_array.c src/nc_client.c src/nc_conf.c src/nc_connection.c
        src/nc_core.c src/nc_log.c src/nc_log.c src/nc_mbuf.c src/nc_message.c
        src/nc_proxy.c src/nc_rbtree.c src/nc_request.c src/nc_response.c
        src/nc_stats.c src/nc_server.c src/nc_signal.c src/nc_string.c src/nc_util.c
        src/event/nc_epoll.c src/event/nc_kqueue.c src/event/nc_evport.c

        src/hashkit/nc_crc16.c src/hashkit/nc_crc32.c src/hashkit/nc_fnv.c
        src/hashkit/nc_hsieh.c src/hashkit/nc_jenkins.c src/hashkit/nc_ketama.c
        src/hashkit/nc_md5.c src/hashkit/nc_modula.c src/hashkit/nc_murmur.c
        src/hashkit/nc_one_at_a_time.c src/hashkit/nc_random.c

        src/proto/nc_memcache.c src/proto/nc_redis.c
        )

add_executable(nutcracker ${SRC_PROXY})
set_target_properties(nutcracker PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SRC_ROOT}")

target_include_directories(nutcracker
        PRIVATE ${SRC_ROOT}/src
        PRIVATE ${SRC_ROOT}/src/event
        PRIVATE ${SRC_ROOT}/src/hashkit
        PRIVATE ${SRC_ROOT}/src/proto
        PRIVATE ${SRC_ROOT}/contrib/yaml-0.1.4/include
        )

target_link_libraries(nutcracker
        PRIVATE pthread
        PRIVATE m
        PRIVATE ${SRC_ROOT}/contrib/yaml-0.1.4/src/.libs/libyaml.a
        )

link_directories(src src/event src/hashkit src/proto contrib/yaml-0.1.4)