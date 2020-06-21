add_executable(enact-cli ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(enact-cli enact)

set_target_properties(enact-cli PROPERTIES OUTPUT_NAME enact)