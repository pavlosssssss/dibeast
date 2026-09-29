# Everything in libdibeast is hidden unless marked DIBEAST_EXPORT. Layers are OBJECT libraries so their
# objects land in the shared library itself; dibeast_EXPORTS makes DIBEAST_EXPORT mean "export" there.
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
add_compile_definitions(dibeast_EXPORTS)

set(DIBEAST_GENERATED_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)

add_library(dibeast_common INTERFACE)
target_include_directories(dibeast_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${DIBEAST_GENERATED_INCLUDE_DIR}
)
target_link_libraries(dibeast_common INTERFACE Boost::asio)

add_library(dibeast_data OBJECT
    data/AppStateStore.cpp
)
target_link_libraries(dibeast_data PUBLIC dibeast_common)

add_library(dibeast_app OBJECT
    application/AppDispatcher.cpp
    application/AppRegistry.cpp
    application/CorsPolicy.cpp
    application/DefaultAppController.cpp
    application/DefaultProcessLauncher.cpp
    application/DeviceController.cpp
    application/DialXml.cpp
    application/Interfaces.cpp
    application/Xml.cpp
)
target_link_libraries(dibeast_app PUBLIC dibeast_data Boost::asio Boost::process Boost::url)

add_library(dibeast_service OBJECT
    service/rest/HttpServer.cpp
    service/rest/HttpSession.cpp
    service/rest/Response.cpp
    service/rest/Router.cpp
    service/rest/Routes.cpp
    service/ssdp/SsdpService.cpp
)
target_link_libraries(dibeast_service PUBLIC dibeast_app Boost::beast Boost::url)

add_library(dibeast SHARED
    DialApp.cpp
)
target_link_libraries(dibeast PRIVATE dibeast_service dibeast_app dibeast_data)
target_include_directories(dibeast PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${DIBEAST_GENERATED_INCLUDE_DIR}
)
# The version script is the final say on the ABI: only dibeast:: symbols are global, tagged DIBEAST_0.
set(DIBEAST_VERSION_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/dibeast.map)
target_link_options(dibeast PRIVATE
    -Wl,--exclude-libs,ALL
    -Wl,--no-undefined
    -Wl,--version-script=${DIBEAST_VERSION_SCRIPT}
)
set_target_properties(dibeast PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    LINK_DEPENDS ${DIBEAST_VERSION_SCRIPT}
)

include(GenerateExportHeader)
generate_export_header(dibeast
    BASE_NAME DIBEAST
    EXPORT_FILE_NAME ${DIBEAST_GENERATED_INCLUDE_DIR}/dibeast/Export.h
)
