project(client)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(client ${SRC})
target_link_libraries(client hjk muduo_net muduo_base mysqlclient pthread)