#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "zlib" for configuration "Release"
set_property(TARGET zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(zlib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libzlib.a"
  )

list(APPEND _cmake_import_check_targets zlib )
list(APPEND _cmake_import_check_files_for_zlib "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libzlib.a" )

# Import target "libjpeg-turbo" for configuration "Release"
set_property(TARGET libjpeg-turbo APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libjpeg-turbo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibjpeg-turbo.a"
  )

list(APPEND _cmake_import_check_targets libjpeg-turbo )
list(APPEND _cmake_import_check_files_for_libjpeg-turbo "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibjpeg-turbo.a" )

# Import target "libtiff" for configuration "Release"
set_property(TARGET libtiff APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libtiff PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibtiff.a"
  )

list(APPEND _cmake_import_check_targets libtiff )
list(APPEND _cmake_import_check_files_for_libtiff "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibtiff.a" )

# Import target "libwebp" for configuration "Release"
set_property(TARGET libwebp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libwebp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibwebp.a"
  )

list(APPEND _cmake_import_check_targets libwebp )
list(APPEND _cmake_import_check_files_for_libwebp "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibwebp.a" )

# Import target "libopenjp2" for configuration "Release"
set_property(TARGET libopenjp2 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libopenjp2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibopenjp2.a"
  )

list(APPEND _cmake_import_check_targets libopenjp2 )
list(APPEND _cmake_import_check_files_for_libopenjp2 "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibopenjp2.a" )

# Import target "libpng" for configuration "Release"
set_property(TARGET libpng APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libpng PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibpng.a"
  )

list(APPEND _cmake_import_check_targets libpng )
list(APPEND _cmake_import_check_files_for_libpng "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibpng.a" )

# Import target "libprotobuf" for configuration "Release"
set_property(TARGET libprotobuf APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libprotobuf PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibprotobuf.a"
  )

list(APPEND _cmake_import_check_targets libprotobuf )
list(APPEND _cmake_import_check_files_for_libprotobuf "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/liblibprotobuf.a" )

# Import target "tegra_hal" for configuration "Release"
set_property(TARGET tegra_hal APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(tegra_hal PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libtegra_hal.a"
  )

list(APPEND _cmake_import_check_targets tegra_hal )
list(APPEND _cmake_import_check_files_for_tegra_hal "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libtegra_hal.a" )

# Import target "ittnotify" for configuration "Release"
set_property(TARGET ittnotify APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ittnotify PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libittnotify.a"
  )

list(APPEND _cmake_import_check_targets ittnotify )
list(APPEND _cmake_import_check_files_for_ittnotify "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libittnotify.a" )

# Import target "ade" for configuration "Release"
set_property(TARGET ade APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ade PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libade.a"
  )

list(APPEND _cmake_import_check_targets ade )
list(APPEND _cmake_import_check_files_for_ade "${_IMPORT_PREFIX}/lib/opencv4/3rdparty/libade.a" )

# Import target "opencv_world" for configuration "Release"
set_property(TARGET opencv_world APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_world PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopencv_world.a"
  )

list(APPEND _cmake_import_check_targets opencv_world )
list(APPEND _cmake_import_check_files_for_opencv_world "${_IMPORT_PREFIX}/lib/libopencv_world.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
