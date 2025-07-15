add_library(${LIB_NAME})

set(HEADER_PATH ${PROJECT_SOURCE_DIR}/include/${LIB_NAME})

target_include_directories(${LIB_NAME}
    PRIVATE
        ${HEADER_PATH}
        ${PROJECT_SOURCE_DIR}/src
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_sources(${LIB_NAME}
    PUBLIC
        ${HEADER_PATH}/${LIB_NAME}.h
    PRIVATE
        ${LIB_NAME}.cpp
        byte/read.h
        byte/read.cpp
        byte/write.h
        byte/write.cpp
        byte/length.h
        byte/length.cpp
        sml.h
        sml.cpp
        traits.h
)

target_link_libraries(${LIB_NAME}
    PRIVATE
        bit_manip
)