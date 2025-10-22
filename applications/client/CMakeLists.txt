if (NOT UNIX AND NOT WIN32)
    message(FATAL_ERROR "Unsupported platform!")
endif()

# irrKlang
set(IRRKLANG "${CMAKE_CURRENT_SOURCE_DIR}/../../common/irrKlang-64bit-1.6.0")
if(UNIX)
    set(IRRKLANG_LIBRARIES
        "${IRRKLANG}/bin/linux-gcc-64/libIrrKlang.so"
    #    "${IRRKLANG}/bin/linux-gcc-64/ikpFlac.so"
    #    "${IRRKLANG}/bin/linux-gcc-64/ikpMP3.so"
)
endif()
if(WIN32)
    set(IRRKLANG_LIBRARIES
        "${IRRKLANG}/bin/winx64-visualStudio/irrKlang.dll"
        #    "${IRRKLANG}/bin/winx64-visualStudio/ikpFlac.dll"
        #    "${IRRKLANG}/bin/winx64-visualStudio/ikpMP3.dll"
    )
endif()
set(IRRKLANG_INCLUDE_DIRS "${IRRKLANG}/include")

file(GLOB IRRKLANG_HEADERS "${IRRKLANG_INCLUDE_DIRS}/*.h")

set(SOURCES
        "${IRRKLANG_HEADERS}"
        main.cpp
        particles.cpp particles.h
        rendering/buffer.cpp rendering/buffer.h
        rendering/pass.cpp rendering/pass.h
        rendering/util.cpp rendering/util.h
        map/block.h map/block.cpp
        map/chunk.h map/chunk.cpp
        model/model.h model/model.cpp
        map/map.h map/map.cpp
        gui/IconFont.h
        audio/test/SoundTest.cpp audio/test/SoundTest.h
        audio/test/CircularSoundTest.cpp audio/test/CircularSoundTest.h
        audio/SoundUtil.cpp audio/SoundUtil.h
        gui/Gui.cpp gui/Gui.h
        model/movable_model.cpp model/movable_model.h
        model/player.cpp model/player.h
        input/InputBinding.h
        input/InputManager.h
        networking/ambassador.cpp networking/ambassador.h
        animation/animation.h
        animation/animator.h
        animation/animdata.h
        animation/assimp_glm_helpers.h
        animation/bone.h
        animation/mesh.h
        animation/model_animation.h
        model/animated_model.cpp model/animated_model.h
        model/enemy.cpp model/enemy.h
        networking/cl_msg.cpp networking/cl_msg.h
        model/animal.cpp model/animal.h
        model/cloud.cpp model/cloud.h
        rain.cpp rain.h
        point_light.cpp point_light.h
        audio/SoundManager.cpp audio/SoundManager.h
        gui/GuiUtils.cpp gui/GuiUtils.h
        lightning.cpp lightning.h
        model/item.cpp model/item.h model/screen_model.cpp model/screen_model.h
        flare.cpp flare.h
        model/screen_model_test.cpp model/screen_model_test.h
        sky.cpp sky.h
        model/RemotePlayer.cpp model/RemotePlayer.h)

set(TARGET a5-client)

add_executable(${TARGET} ${SOURCES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# output in source
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# define base path for mesh/image lookups
target_compile_definitions(${TARGET}
        PRIVATE -DEXECUTABLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

# TODO
# disable sleep of network thread by defining macro AMBASSADOR_NO_SLEEP
#target_compile_definitions(${TARGET}
#        PRIVATE -DAMBASSADOR_NO_SLEEP
#)

# handle dependencies

# Boost
find_package(Boost REQUIRED COMPONENTS system program_options)
target_include_directories(
        ${TARGET}
        PRIVATE ${Boost_INCLUDE_DIRS}
)

# irrKlang
target_include_directories(${TARGET}
        PRIVATE ${IRRKLANG_INCLUDE_DIRS}
)
target_link_libraries(${TARGET}
        PRIVATE "${IRRKLANG_LIBRARIES}"
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
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/cppgl/external/thirdparty/win/bin/x64"
            $<TARGET_FILE_DIR:${TARGET}>)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:cppgl>
            $<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_NAME:cppgl>)

    # irrKlang
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${IRRKLANG}/bin/winx64-visualStudio"
            $<TARGET_FILE_DIR:${TARGET}>)
endif()
