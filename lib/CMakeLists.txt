add_subdirectory(ast)
add_subdirectory(context)
add_subdirectory(parser)
#add_subdirectory(sema)

set(ENACT_SRC
        ${AST_SRC}
        ${CONTEXT_SRC}
        ${PARSER_SRC})
        #${SEMA_SRC})

set(ENACT_SRC ${ENACT_SRC} PARENT_SCOPE)

add_library(enact ${ENACT_SRC})
target_include_directories(enact PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)