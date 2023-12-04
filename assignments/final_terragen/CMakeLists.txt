#Transformations example

file(
 GLOB_RECURSE FINAL_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE FINAL_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this final_terragen's asset folder to bin when it is built
add_custom_target(copyAssetsA8 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${FINAL_INC} DESTINATION include/final_terragen)
add_executable(final_terragen ${FINAL_SRC} ${FINAL_INC} ${FINAL_ASSETS} "constants.h")
target_link_libraries(final_terragen PUBLIC core IMGUI)
target_include_directories(final_terragen PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when final_terragen is built
add_dependencies(final_terragen copyAssetsA8)