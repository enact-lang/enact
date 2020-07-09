add_subdirectory(ast)
add_subdirectory(context)
add_subdirectory(parser)

set(ENACT_SRC
        ${AST_SRC}
        ${CONTEXT_SRC}
        ${PARSER_SRC}

        ${CMAKE_CURRENT_SOURCE_DIR}/AstSerialise.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AstSerialise.h
        ${CMAKE_CURRENT_SOURCE_DIR}/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/InsertionOrderMap.h
        ${CMAKE_CURRENT_SOURCE_DIR}/trivialStructs.h)
set(ENACT_SRC ${ENACT_SRC} PARENT_SCOPE)

add_library(enact ${ENACT_SRC})