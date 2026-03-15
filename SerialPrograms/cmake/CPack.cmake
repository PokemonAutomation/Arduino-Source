### Packaging
# Install binary
install(
    TARGETS SerialPrograms
    RUNTIME DESTINATION bin
)

# Resources
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/Resources/
    DESTINATION share/SerialPrograms/Resources
)

# App icon
install(
    FILES ${CMAKE_SOURCE_DIR}/../IconResource/SerialPrograms.png
    DESTINATION share/icons/hicolor/256x256/apps
)

# Desktop Entry
install(
    FILES ${CMAKE_SOURCE_DIR}/../IconResource/SerialPrograms.desktop
    DESTINATION share/applications
)

# License
install(
    FILES ${CMAKE_SOURCE_DIR}/../LICENSE
    DESTINATION share/licenses/SerialPrograms
)

# Discord Partner Library
install(
    FILES ${CMAKE_SOURCE_DIR}/../3rdPartyBinaries/discord_social_sdk_linux/lib/release/libdiscord_partner_sdk.so
    DESTINATION lib
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# ONNX Runtime Library
install(
    FILES ${CMAKE_BINARY_DIR}/onnxruntime-linux-x64-1.23.0/lib/libonnxruntime.so.1.23.0
    DESTINATION lib
    RENAME libonnxruntime.so.1
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

### CPack Config
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_PACKAGE_NAME "pokemon-automation")
set(CPACK_PACKAGE_VERSION ${SerialPrograms_VERSION})
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "PokemonAutomation")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6core6, libqt6gui6, libqt6widgets6, libqt6multimedia6, libqt6serialport6, libqt6multimediawidgets6, \
     qt6-image-formats-plugins, gstreamer1.0-plugins-base, gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad, \
     gstreamer1.0-plugins-ugly, libopencv-core410, libopencv-imgproc410, libglx0, libgl1, libhdf5-310, libvtk9.3, \
     tesseract-ocr"
)

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Utilities")
set(CPACK_RPM_PACKAGE_REQUIRES
    "qt6-qtbase, qt6-qtmultimedia, qt6-qtserialport, qt6-qtimageformats, gstreamer1, gstreamer1-plugins-base, \
    gstreamer1-plugins-good, gstreamer1-plugins-bad-free, gstreamer1-plugins-ugly, gstreamer-plugin-libav, \
    opencv, mesa-libGL, libglvnd, hdf5, vtk, tesseract, leptonica, onnxruntime"
)

include(CPack)