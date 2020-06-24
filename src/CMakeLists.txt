add_executable(enact-cli
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/parser/Lexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/parser/Parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/parser/Typename.cpp)
#target_link_libraries(enact-cli enact)

set_target_properties(enact-cli PROPERTIES OUTPUT_NAME enact)