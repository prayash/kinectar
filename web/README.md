# Kinectar

An experiment with Kinect v1 and three.js.

The process of going from Kinect -> WebGL requires you to fire up an OpenGL viewer of the Kinect camera, recording the viewer to get Kinect's data, exporting that to a .webm format and reading it in via WebGL.

## Installation

- First, install libfreenect.
```
$ git clone git://github.com/OpenKinect/libfreenect.git
```
- Then, you need to build it from source:
```
$ cd libfreenect
$ mkdir build; cd build
$ cmake ..
$ make
$ rm ../CMakeCache.txt
$ cmake .. -G Xcode
````

## Running
- Open `libfreenect/build/libfreenect.xcodeproj` and run the freenect-glview executable OR you an also:
- Execute `$ ./bin/freenect-glview` via command line.

## Recording
- Use ffmpeg and libvpx to screengrab the glview window and encode to `webm` format in order to use with WebGL.

```
$ ffmpeg -strict -2 -f avfoundation -pixel_format bgr0 -i "1:none" -codec:v libvpx kinect.webm
```

## Firing up the WebGL application
