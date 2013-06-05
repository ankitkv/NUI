### 3D Natural User Interface for Operating Systems

The purpose of this project is to make the operating system shell take advantage of the advent of stereoscopic 3D displays in market. Windows and window controls are projected in 3D space and the user can interact with them by touching the perceived positions of the windows in space, not the screen.

Thus the user can move and touch window controls in 3D space. The 3D display is implemented via the compiz compositor, and the input is implemented via OpenNI and OpenCV.


#### I. Hardware requirements

You will need an OpenNI-compliant 3D depth-sensor for NUI user input.
Please see: http://www.openni.org/3d-sensors/


#### II. Getting the source

To download the latest revision of the project, you will need git. Use the following command to download the NUI repository:

```
$ git clone https://github.com/ankitkv/NUI.git
```

To download the latest changes to the source code after the repository has been downloaded, use the following command from the NUI directory:

```
$ git pull
```


#### III. Building NUI

To compile NUI, you will need the libraries for OpenNI, compiz development, OpenCV, OpenGL. You will also need cmake to build the compiz plugin.

To build and install everything, use this command from the source root directory:

```
$ ./make.sh
```

Alternatively, if you wish to compile individual components, follow these steps:

Before building anaglyph, you must first build and install libnui.
From the source root directory,

```
$ cd nui/bin
$ ./make.sh
$ sudo ./make.sh install
```

Then, build and install the anaglyph compiz plugin.
From the source root directory,

```
$ cd anaglyph
$ ./make.sh
$ sudo ./make.sh install
```
