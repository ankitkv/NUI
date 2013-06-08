find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(gesture PLUGINDEPS composite opengl INCDIRS "../nui/libnui" $ENV{OPENNI2_INCLUDE} LIBDIRS "/usr/local/lib" $ENV{OPENNI2_REDIST}/OpenNI2/Drivers LIBRARIES nui OpenNI2 OniFile PS1080 opencv_calib3d opencv_core opencv_imgproc)
