cmake_minimum_required(VERSION 2.6)
project(ranking)

#############################################
# OpenMP support
option(USE_OPENMP "use openmp as multi-thread implement" ON)

# Compiler variables
set(BUILD_SHARED_LIBS ON)
set(BUILD_DEPENDENCE_DIR ${PROJECT_SOURCE_DIR}/../deps/lib)
#set(BUILD_BIN_DIR ${PROJECT_SOURCE_DIR}/../bin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-w -O3 -W -Wall")
#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-w -g -W -Wall -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11") #-std=c++0x
set(CMAKE_INSTALL_PREFIX $HOME/${PROJECT_NAME})
set(CMAKE_SHARED_LINKER_FLAGS "-fopenmp -L${BUILD_DEPENDENCE_DIR} -L/usr/local/libmemcached/lib/ -L/usr/local/lib -L/home/w/lib64/")
set(CMAKE_EXE_LINKER_FLAGS "-fopenmp -L${BUILD_DEPENDENCE_DIR} -L/home/w/lib64/ -L/usr/local/lib")

#-fuse-ld=gold -Wl,--no-gnu-unique,--no-undefined
# Compiler options
#add_definitions(-DOMP)
#add_definitions(-DPROF)
#add_definitions(-D__DEBUG_XFEA_FEA_OUTPUT_ONLINE_)

#add_subdirectory(Tools)
#############################################


#############################################
# Compile file option
# Get whole source files that need to compile
aux_source_directory(. SOURCE_FILES)
aux_source_directory(Feature Feature_SOURCE_FILES)
aux_source_directory(Framework Framework_SOURCE_FILES)
#aux_source_directory(HiRedis HiRedis_SOURCE_FILES)
aux_source_directory(Model Model_SOURCE_FILES)
aux_source_directory(Strategy Strategy_SOURCE_FILES)
aux_source_directory(Condition Condition_SOURCE_FILES)
aux_source_directory(Utilities Utilities_SOURCE_FILES)
aux_source_directory(Extractor Extractor_SOURCE_FILES)
aux_source_directory(JavaTools JavaTools_SOURCE_FILES)
aux_source_directory(Interfaces Interfaces_SOURCE_FILES)

set(SOURCE_FILES ${Utilities_SOURCE_FILES} ${Feature_SOURCE_FILES} ${Framework_SOURCE_FILES}
${Model_SOURCE_FILES} ${Strategy_SOURCE_FILES} ${Condition_SOURCE_FILES}
${PROTO_SRC} ${Extractor_SOURCE_FILES}
${JavaTools_SOURCE_FILES} ${Interfaces_SOURCE_FILES})
aux_source_directory(Test Test_SOURCE_FILES)
aux_source_directory(Proxy Proxy_SOURCE_FILES)

#list(REMOVE_ITEM SOURCE_FILES_SE2 "Utilities/ObjectFactoryImp.cpp" "Utilities/Service.cpp")

# Compile application to executable program
add_library(mtai SHARED ${SOURCE_FILES})
add_library(proxy SHARED ${Proxy_SOURCE_FILES})
add_executable(test ${Test_SOURCE_FILES})

aux_source_directory(TestOcpx TestOcpx_SOURCE_FILES)
add_executable(testocpx ${TestOcpx_SOURCE_FILES})

add_library(protobuf STATIC IMPORTED)
add_library(bisheng STATIC IMPORTED)
set_property(TARGET protobuf PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../deps/lib/libprotobuf.a)
set_property(TARGET bisheng PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../deps/lib/libbisheng.a)
add_library(log4cpp STATIC IMPORTED)
set_property(TARGET log4cpp PROPERTY IMPORTED_LOCATION
${PROJECT_SOURCE_DIR}/../deps/lib/liblog4cpp.a)
#############################################


#############################################
# Dependence library setting
SET (WHOLELIB -Wl,--whole-archive
${PROJECT_SOURCE_DIR}/../deps/lib/libbisheng.a -Wl,--no-whole-archive)

target_link_libraries(proxy
dl
)

target_link_libraries(mtai
pthread
#hiredis
protobuf
bisheng
jsoncpp
log4cpp
boost_system
dl
#${WHOLELIB}
#bidfeed-sharelib
)
target_link_libraries(test
#${PROJECT_NAME}
pthread
protobuf
dl
#boost_system
#${WHOLELIB}
)
target_link_libraries(testocpx
protobuf
dl
pthread
)


# Properties setting
#set_target_properties(${PROJECT_NAME} PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY "${BUILD_BIN_DIR}"
#ARCHIVE_OUTPUT_DIRECTORY "${BUILD_BIN_DIR}"
#)

# Include path
include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/../deps/include
${PROJECT_SOURCE_DIR}/../deps/include/bisheng
${PROJECT_SOURCE_DIR}/Interfaces
${PROJECT_SOURCE_DIR}/Framework
${PROJECT_SOURCE_DIR}/Utilities
${PROJECT_SOURCE_DIR}/HiRedis
/usr/local/libmemcached/include/
/home/w/include
/usr/include
/usr/local/include/
)
#############################################

#############################################
# install
#install(TARGETS ${PROJECT_NAME} DESTINATION /usr/sbin/)
#install(DIRECTORY ${PROJECT_SOURCE_DIR}/initd/ DESTINATION /etc 
#PATTERN "*"
#PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ)
#install(FILES ${PROJECT_SOURCE_DIR}/initd/log DESTINATION /var/log/oeld/)
#install(FILES ${PROJECT_SOURCE_DIR}/initd/log DESTINATION /var/log/oelsd/)
#############################################
