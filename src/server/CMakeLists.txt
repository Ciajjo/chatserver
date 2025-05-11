project(server)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB DBSRC ${PROJECT_SOURCE_DIR}/db/*.cpp)
file(GLOB MODELSRC ${PROJECT_SOURCE_DIR}/model/*.cpp)
file(GLOB REDISSRC ${PROJECT_SOURCE_DIR}/redis/*.cpp)

add_library(hjk SHARED ${MODELSRC} ${DBSRC})

add_executable(chatserver ${SRC} ${DBSRC} ${MODELSRC} ${REDISSRC})
target_link_libraries(chatserver muduo_net muduo_base mysqlclient hiredis pthread)