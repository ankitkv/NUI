find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (anaglyph PLUGINDEPS composite opengl)
