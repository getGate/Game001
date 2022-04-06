
SET(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SUBDIRLIST(TARGET_DIRS ${TARGET_DIR})
foreach(dir ${TARGET_DIRS})
    if(EXISTS "${TARGET_DIR}/${dir}/main.cpp")
        message("find target " ${dir})
        add_executable(${dir} ${dir}/main.cpp)
    endif(EXISTS "${TARGET_DIR}/${dir}/main.cpp")
endforeach()