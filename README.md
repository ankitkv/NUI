### 3D Natural User Interface for Operating Systems

The purpose of this project is to make the operating system shell take advantage of the advent of stereoscopic 3D displays in market. Windows and window controls are projected in 3D space and the user can interact with them by touching the perceived positions of the windows in space, not the screen.

Thus the user can move and touch window controls in 3D space. The 3D display is implemented via the compiz compositor, and the input is implemented via OpenNI and OpenCV.

This project was initially created as a final year project for my computer engineering bachelor's degree, with Siddharth Kulkarni, Ronit Kulkarni and Humayun Mulla.


#### I. Hardware requirements

You will need an OpenNI-compliant 3D depth-sensor for NUI user input.
Please see: http://www.openni.org/3d-sensors/


#### II. Getting the source

To download the latest revision of the project, you will need git. Use the following command to download the NUI repository:

```
$ git clone https://github.com/ankitkv/NUI.git
```

To download the latest changes to the source code after the repository has been cloned, use the following command from the NUI directory:

```
$ git pull
```


#### III. Building NUI

To compile NUI, you will need the libraries for OpenNI, compiz development, OpenCV, OpenGL. You will also need cmake to build the compiz plugin.

Required environment variables:
 - OPENNI2_INCLUDE = your OpenNI Include directory path
 - OPENNI2_REDIST = your OpenNI Redist directory path
 - OPENNI2_DRIVERS_PATH = your OpenNI Drivers directory path (generally $OPENNI2_REDIST/OpenNI2/Drivers)

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

Similarly, build the gesture plugin.

```
$ cd gesture
$ ./make.sh
$ sudo ./make.sh install
```


#### IV. Using NUI

Place your depth sensor to the right of your monitor, looking at the monitor perpendicularly. Run the calibration tool:

```
$ cd nui/bin
$ ./calib
```

In the calib window, ensure that your monitor appears on the very left. The screen should touch the left edge of the window.
Now, when you take a finger near the screen, it should be indicated with a red dot.

 * Touch the upper left corner of the screen, press Q
 * Touch the upper right corner of the screen, press W
 * Touch the lower left corner of the screen, press A
 * Touch the lower right corner of the screen, press S

To cancel calibration, press D.

Once calibrated, the mouse pointer will follow your finger when placed in front of the screen.
Ensure accurate calibration, and press ESC to exit the calibration tool. The calibration data is saved in ~/.nuipoints.xml

Load the anaglyph (Anaglyph) and gesture (3D Gestures) plugins in compiz.
Press Shift+Alt+A to start anaglyph.
Press Shift+Alt+G to start gesture detection. You will need your depth sensor connected for this to work.

You can now see windows in 3D, hold them, move them around, touch them to click on them, all in 3D space :)
