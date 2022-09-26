llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support )
file (GLOB SOURCES
   *.cpp
)
add_executable(svfgraph ${SOURCES} ICFGWrapper.cpp ICFGWrapper.h)

target_link_libraries(svfgraph ${SVF_LIB} ${llvm_libs})

target_link_libraries(svfgraph ${Z3_DIR}/bin/libz3.a )
set_target_properties( svfgraph PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

include_directories(.)