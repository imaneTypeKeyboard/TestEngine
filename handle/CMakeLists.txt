#set(CMAKE_CXX_FLAGS "-fpermissive -p -O2")
#set(CMAKE_CXX_FLAGS "-fpermissive -p -g -pthread -DCC_TARGET_PLATFORM=linux")
set(CMAKE_CXX_FLAGS "-fpermissive -g -Wall")
#SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)
file(GLOB_RECURSE sources_server_share ../framework/*.cpp ../framework/*.h)
#file(GLOB_RECURSE sources_remove_SERVER_SHARE ../pitayaserver/Lua*)

file(GLOB sources *.cc *.cpp *.h)

set(gameserver_stat_srcs
  ${sources}
  ${sources_server_share}
)

#list(REMOVE_ITEM gameserver_STAT_SRCS ${sources_remove_SERVER_SHARE})

message(status " This is the server_share srcs:" ${sources_server_share})
#message(status " This is the all srcs:" ${gameserver_STAT_SRCS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../framework
  ${CMAKE_CURRENT_SOURCE_DIR}
)
LINK_DIRECTORIES(
  /usr/local/lib
)
add_executable(TestEngine ${gameserver_stat_srcs})
target_link_libraries(TestEngine rt)
target_link_libraries(TestEngine protobuf)
