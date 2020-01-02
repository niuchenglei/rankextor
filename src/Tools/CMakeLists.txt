file(GLOB_RECURSE examples_srcs "${PROJECT_SOURCE_DIR}/Tools/*.cpp")

foreach(source_file ${examples_srcs})
  # get file name
  get_filename_component(name ${source_file} NAME_WE)
    
  # get folder name
  get_filename_component(path ${source_file} PATH)
  get_filename_component(folder ${path} NAME_WE)

  add_executable(${name} ${source_file})

  
  include_directories(
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/../deps/include
      ${PROJECT_SOURCE_DIR}/Interfaces
      ${PROJECT_SOURCE_DIR}/Framework
      ${PROJECT_SOURCE_DIR}/Utilities
      ${PROJECT_SOURCE_DIR}/HiRedis
      /home/w/include/
      /usr/include
      /usr/local/include/
  )
  #target_link_libraries(${name} ${Caffe_LINK})

  # set back RUNTIME_OUTPUT_DIRECTORY
  #set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/examples/${folder}")

  # install
  #install(TARGETS ${name} DESTINATION bin)

endforeach()
