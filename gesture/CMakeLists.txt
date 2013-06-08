find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(gesture PLUGINDEPS composite opengl)
