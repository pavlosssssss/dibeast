add_executable(phony_app
    phony/main.cpp
    phony/AdditionalDataClient.cpp
)
target_include_directories(phony_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(phony_app PRIVATE dibeast_common Boost::beast Boost::url)

add_library(dibeast_apps INTERFACE)
target_include_directories(dibeast_apps INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dibeast_apps INTERFACE dibeast)
target_compile_definitions(dibeast_apps INTERFACE DIBEAST_PHONY_EXECUTABLE="$<TARGET_FILE:phony_app>")
