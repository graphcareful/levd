cmake_minimum_required (VERSION 2.6)
project (kraken_linux)
# The version number.
set (Levd_VERSION_MAJOR 0)
set (Levd_VERSION_MINOR 4)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-fmax-errors=2")
set(KRAKEN_LIB_LIBRARIES
  :libsensors.so
  :libyaml-cpp.so
  :libglog.so.0
  :libusb-1.0.so.0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  ${PROJECT_SOURCE_DIR}/version.h.in
  ${PROJECT_BINARY_DIR}/version.h)

set(KRAKEN_LIB_SOURCES
  ${PROJECT_SOURCE_DIR}/leviathan_service.cpp
  ${PROJECT_SOURCE_DIR}/leviathan_config.cpp
  ${PROJECT_SOURCE_DIR}/usb_descriptor_utils.cpp
  ${PROJECT_SOURCE_DIR}/kraken_driver.cpp
  ${PROJECT_SOURCE_DIR}/main.cpp)

add_executable (kraken ${KRAKEN_LIB_SOURCES})
target_link_libraries(kraken ${KRAKEN_LIB_LIBRARIES})

install(
  TARGETS kraken
  RUNTIME DESTINATION /usr/bin/
  )
# TODO: Find a shorter way to install!
install(
  DIRECTORY config/
  DESTINATION /usr/bin
  FILES_MATCHING PATTERN "kraken-start.sh"
  PERMISSIONS WORLD_READ WORLD_WRITE WORLD_EXECUTE
  GROUP_EXECUTE GROUP_READ
  OWNER_EXECUTE OWNER_READ)
install(
  DIRECTORY config/
  DESTINATION /etc/leviathan
  FILES_MATCHING PATTERN "levd.cfg")
install(
  DIRECTORY config/
  DESTINATION /etc/systemd/system
  FILES_MATCHING PATTERN "levd.service")
