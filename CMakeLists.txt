cmake_minimum_required(VERSION 2.8)

project(sig)

SET(headers miniaudio.h)

IF(MSVC)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /we4013 /we4020 /wd4996")
    SET(libs)
ELSEIF(UNIX)
	set(CMAKE_BUILD_TYPE Debug)
    #set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c90 -march=native")
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-march=native")
    SET(libs m)
ENDIF()

set(exe_name sig-ref)
add_executable(${exe_name} sig-ref.c)
target_link_libraries(${exe_name} ${libs})

set(exe_name sig2)
add_executable(${exe_name} sig2.c miniaudio.h cds_sync.h)
target_link_libraries(${exe_name} ${libs})
