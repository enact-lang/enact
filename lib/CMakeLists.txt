add_subdirectory(analyser)
add_subdirectory(ast)
add_subdirectory(bytecode)
add_subdirectory(compiler)
add_subdirectory(context)
add_subdirectory(memory)
add_subdirectory(parser)
add_subdirectory(type)
add_subdirectory(value)
add_subdirectory(vm)

set(ENACT_SRC
        ${ANALYSER_SRC}
        ${AST_SRC}
        ${BYTECODE_SRC}
        ${COMPILER_SRC}
        ${CONTEXT_SRC}
        ${MEMORY_SRC}
        ${PARSER_SRC}
        ${TYPE_SRC}
        ${VALUE_SRC}
        ${VM_SRC}

        ${CMAKE_CURRENT_SOURCE_DIR}/AstPrinter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AstPrinter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/InsertionOrderMap.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Natives.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Natives.h
        ${CMAKE_CURRENT_SOURCE_DIR}/trivialStructs.h)
set(ENACT_SRC ${ENACT_SRC} PARENT_SCOPE)

add_library(enact ${ENACT_SRC})