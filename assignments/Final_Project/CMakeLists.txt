#Transformations example

file(
 GLOB_RECURSE FINAL_PROJECT_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE FINAL_PROJECT_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment6_proceduralGeometry's asset folder to bin when it is built
add_custom_target(copyAssetsA8 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${FINAL_PROJECT_INC} DESTINATION include/Final_Project)
add_executable(Final_Project ${FINAL_PROJECT_SRC} ${FINAL_PROJECT_INC} ${FINAL_PROJECT_ASSETS}  )
target_link_libraries(Final_Project PUBLIC core IMGUI)
target_include_directories(Final_Project PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment6_proceduralGeometry is built
add_dependencies(Final_Project copyAssetsA8)