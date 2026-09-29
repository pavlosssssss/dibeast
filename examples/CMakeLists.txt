add_executable(dibeast_server
    dial_server/main.cpp
    dial_server/ForkExecLauncher.cpp
)
target_link_libraries(dibeast_server PRIVATE dibeast dibeast_apps)
add_dependencies(dibeast_server phony_app)
