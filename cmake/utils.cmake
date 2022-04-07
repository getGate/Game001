MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        string(FIND ${child} Unity find_res)
        IF(IS_DIRECTORY ${curdir}/${child} AND ${find_res} EQUAL -1)
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

MACRO(ADDGAME game sources resources)
    message("game: " ${game})
    SET(THE_GAME_DIR "${GAME_BIN_DIR}/bin")
    message("set bin path: " "${THE_GAME_DIR}")

    add_executable(${game} sources)
    
    set_target_properties( ${game}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${THE_GAME_DIR}
    )

    if(DEFINE resources)
        add_custom_command(TARGET ${game}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${resources} ${THE_GAME_DIR}
        )
    endif(DEFINE resources)
    
ENDMACRO()

MACRO(ADDGAMEDIR TARGET_DIR GAMES)

    foreach(theGame ${GAMES})
        if(EXISTS "${TARGET_DIR}/${theGame}/main.cpp")
            message(STATUS "game: " ${theGame})
            add_executable(${theGame} ${theGame}/main.cpp)

            set(theBinDir "${GAME_BIN_DIR}/${theGame}")

            message("game bin path: " "${theBinDir}")
            set_target_properties( ${theGame}
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${theBinDir}
            )

            # copy data-files if needed
            set(theDataSrc "${TARGET_DIR}/${theGame}/data")
            if(EXISTS "${theDataSrc}")
            add_custom_command(TARGET ${theGame}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${theDataSrc} ${theBinDir}
                DEPENDS ${theDataSrc}
            )
            endif(EXISTS "${theDataSrc}")
            
        endif(EXISTS "${TARGET_DIR}/${theGame}/main.cpp")
    endforeach()
ENDMACRO()