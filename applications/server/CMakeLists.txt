set(SOURCES
    main.cpp
    Server.cpp Server.h
    sv_msg.cpp sv_msg.h
    SvMap.cpp SvMap.h
    SvMovable.cpp SvMovable.h
    image.cpp image.h
    WeatherType.h
    AStar.cpp AStar.h
)

set(TARGET a5-server)

# executable target
add_executable(${TARGET} ${SOURCES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# output in source
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# define base path for mesh/image lookups
target_compile_definitions(${TARGET}
    PRIVATE -DEXECUTABLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

# TODO
## print max round ms
target_compile_definitions(${TARGET}
        PRIVATE -DSERVER_PRINT_MAX
)

# handle dependencies

# Boost
find_package(Boost REQUIRED COMPONENTS system program_options)
target_include_directories(
    ${TARGET}
    PRIVATE ${Boost_INCLUDE_DIRS}
)

# OpenGL
find_package(OpenGL REQUIRED)

target_include_directories(${TARGET}
    PRIVATE ${OPENGL_INCLUDE_DIRS}
)
target_link_libraries(${TARGET}
    PRIVATE ${OPENGL_LIBRARIES}
)

# common libs
target_link_libraries(${TARGET}
    PRIVATE cppgl nlohmann_json networking ${Boost_LIBRARIES}
)

if(WIN32)
    # post build dll copy
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../common/cppgl/external/thirdparty/win/bin/x64" $<TARGET_FILE_DIR:${TARGET}>)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:cppgl> $<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_NAME:cppgl>)
endif()
