# If on Windows, build gridrec.cc
if(TOMOPY_CXX_GRIDREC)
# Locate sources for this project headers are included for IDEs
    file(GLOB gridrec_tomo_sources
        ${CMAKE_CURRENT_LIST_DIR}/*.cc
        ${CMAKE_CURRENT_LIST_DIR}/filters.c)
    file(GLOB gridrec_tomo_headers
        ${CMAKE_CURRENT_LIST_DIR}/../../include/gridrec/*.hh
        ${CMAKE_CURRENT_LIST_DIR}/../../include/gridrec/filters.h)

else(TOMOPY_CXX_GRIDREC)

    file(GLOB gridrec_tomo_sources
        ${CMAKE_CURRENT_LIST_DIR}/*.c)  
    file(GLOB gridrec_tomo_headers
        ${CMAKE_CURRENT_LIST_DIR}/../../include/gridrec/*.h)   

endif(TOMOPY_CXX_GRIDREC)

# core sources
set(GRIDREC_SOURCES ${gridrec_tomo_headers} ${gridrec_tomo_sources})

# create library
add_library(libtomopy-gridrec SHARED ${GRIDREC_SOURCES})

# link library
target_link_libraries(libtomopy-gridrec PUBLIC ${EXTERNAL_LIBRARIES}
    PRIVATE ${EXTERNAL_PRIVATE_LIBRARIES})

# where to place the library
set(OUTPUT_PATH ${CMAKE_BINARY_DIR})

list(APPEND TARGET_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../include/gridrec")

# target properties
set_target_properties(libtomopy-gridrec
    PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY    ${OUTPUT_PATH}/tomopy/sharedlibs
    LIBRARY_OUTPUT_DIRECTORY    ${OUTPUT_PATH}/tomopy/sharedlibs
    ARCHIVE_OUTPUT_DIRECTORY    ${OUTPUT_PATH}/tomopy/sharedlibs
    INCLUDE_DIRECTORIES         "${TARGET_INCLUDE_DIRECTORIES}"
    C_STANDARD                  ${CMAKE_C_STANDARD}
    C_STANDARD_REQUIRED         ${CMAKE_C_STANDARD_REQUIRED}
    CXX_STANDARD                ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED       ${CMAKE_CXX_STANDARD_REQUIRED}
    ${CUDA_PROPERTIES})

target_compile_definitions(libtomopy-gridrec PUBLIC
    ${${PROJECT_NAME}_DEFINITIONS})

target_compile_options(libtomopy-gridrec PUBLIC
    $<$<COMPILE_LANGUAGE:C>:${${PROJECT_NAME}_C_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${${PROJECT_NAME}_CXX_FLAGS}>
    $<$<COMPILE_LANGUAGE:CUDA>:${${PROJECT_NAME}_CUDA_FLAGS}>)

install(TARGETS libtomopy-gridrec
    DESTINATION ${PYTHON_SITE_DIR}/tomopy/sharedlibs
    COMPONENT python)
