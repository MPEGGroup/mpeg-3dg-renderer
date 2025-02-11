
# mpeg-pcc-renderer 

MPEG 3DG renderer is official MPEG/3DG/WG7 software used to render a static and dynamic point clouds and mesh objects.

## Dependencies

Our project uses:
  - OpenGL  
  - [GLFW](https://github.com/glfw/glfw)
  - [nanoflann](https://github.com/jlblancoc/nanoflann)
  - [Stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
  - [OpenGL Mathematics(GLM)](https://github.com/g-truc/glm.git)
  - [Tiny wavefront .obj loader(TinyObjLoader)](https://github.com/tinyobjloader/tinyobjloader)
  - [Open Multi-Processing](https://www.openmp.org/)
  - program-options-lite 

The requiered sub-modules are cloned from the CMakeFile by the building command lines and stored in the external folder (GLFW and GLM).  

On various systems, the OpenMP library must be installed manually. The following command can be used to install this dependency: `apt install libomp-dev` or `brew install libomp-dev libomp5`.

## Compiling instructions

This software supports both Windows, MacOs and Linux Operating Systems. Compilation can be performed as follows:
* Linux:
  * to build use the command `build.sh`.
  * to clean all object use the command `clear.sh`.
* Windows:
  * to build use the CMAKE command or `build.sh` or open MSVC project ./build/Release/PccAppRenderer.sln
  * to clean all object use the command `clear.sh`.
* MacOs:
  * to build use the CMAKE command or `build.sh` or open Xcode.
  * to clean all object use the command `clear.sh`.

Cmake commands can be used to build software:

```
mkdir build 
cd ./build/
cmake .. 
cmake --build . --config Release 
```

The command `clear.sh all` can be used to remove all dependencies.

## 3D mesh objects supports

The 3D mesh objects could be load and display in the renderer (obj and ply objects). 

## Command line parameters

The mpeg-pcc-renderer input parameters are listed below: 

```
-------------------------------------------------------------------------------------------
|                                   Point Cloud Renderer                                  |
|        Copyright 2016-2025 - InterDigital CE Patent Holding - All Rights Reserved       |
|                                                                                         |
| This software is being made available under the BSD License, available in the           |
| root folder. This software may be subject to InterDigital and other third party and     |
| contributor rights, including patent rights, and no such rights are granted under       |
| this license.                                                                           |
-------------------------------------------------------------------------------------------
PccAppRenderer configuration: input parameters must be:
        --version=...                   Print version.
        --help=...                      Print help.
        --config=...                    Parse configuration file.
  -f,   --PlyFile=""                    Ply input filename.
  -d,   --PlyDir=""                     Ply input directory.
        --SrcFile=""                    Source Ply filename (used for
                                        comparison).
        --SrcDir=""                     Source Ply directory (used for
                                        comparison).
  -b,   --binary=0                      Create temp binary files.
  -o,   --RgbFile=""                    Output RGB 8bits filename (specify
                                        prefix file name).
  -x,   --camera=""                     Camera path filename.
  -y,   --viewpoint=""                  Viewpoint filename.
        --spline=0                      Interpolate the camera path by splines.
  -n,   --frameNumber=1                 Fraumber.
  -i,   --frameIndex=0                  Frame index.
        --fps=30                        Frames per second.
  -a,   --align=0                       Align (0:X, 1:-X, 2:Y, 3:-Y 4:Z, 5:-Z).
        --width=800                     Window width.
        --height=600                    Window height.
        --posx=-1                       Window position X.
        --posy=-1                       Window position Y.
        --size=1                        Point size.
        --blendMode=0                   Blended point mode (0:Gaussian,
                                        1:Linear).
        --alphaFalloff=1                Blend alpha falloff.
        --type=0                        Point type:
                                          Point cloud: 0: cube,
                                                       1: circle,
                                                       2: point
                                                       3: blended point
                                          Mesh:        0: surface,
                                                       1: surface+wireframe,
                                                       2: wireframe,
                                                       3: point
        --cameraPathIndex=-1            Camera path index:
                                          Preload camera path composed of three parts:
                                            - 3 seconds from a fixed position (left or right) aligned with the bounding box of the sequence (orthographic rendering)
                                            - 3 seconds from a fixed position (front) aligned with the bounding box of the sequence (orthographic rendering)
                                            - 4 seconds of dynamic path around the models (perspective rendering).
                                          Mode:
                                            0: left  + front + front zoom in
                                            1: right + front + front zoom in
                                            2: left  + front + front zoom in right curved
                                            3: right + front + front zoom in left curved
                                            4: left  + front + near semicircle to right
                                            5: right + front + near semicircle to left
                                            6: left  + front + near circle to right
                                            7: right + front + near circle to left
                                            8: left  + front + upward spiral to the right
                                            9: right + front + upward spiral to the left
                                           10: Left curved zoom for longdress, soldier, mitch, thomas, levi sequences
                                           11: Zoom in for basketball sequence
                                           12: Zoom in for dancer sequence
                                           13: Zoom in for football sequence

        --monitor=0                     Monitor to display the window.
        --backgroundIndex=-1            Window background index.
        --backgroundColor='128 128 128' Background color:"R G B".
        --floor=0                       Adds grey floor under objects.
        --floorColor='95 95 95'         Floor color:"R G B".
        --depthMap=0                    Display depth map.
  -p,   --play=0                        Play the sequences.
        --playBackward=0                Play sequence forward and backward.
  -r,   --rotate=0                      Auto-rotate (speed in [0;4]).
        --overlay=1                     Display overlay.
        --orthographic=0                Orthographic projection.
        --synchronize=0                 Synchronize multi-windows.
        --box=-1                        Bounding box size.
        --dropdups=2                    Drop same coordinate points (0:No,
                                        1:drop, 2:average).
  -c,   --center=0                      Center the object in the bounding box.
  -s,   --scale=0                       Scale mode:    0: disable,
                                                       1: scale according to the object bounding box.
                                                       2: scale according to the
                                        sequence bounding box.
        --scenePath=""                  3D background scene path (obj object).
        --sceneScale=1                  3D background scene scale.
        --scenePos='0 0 0'              3D background scene position: "X Y Z".
        --sceneRot='0 0 0'              3D background scene rotation: "X Y Z".
        --visible=1                     Open user interface.
        --lighting=0                    Enable lighting (only for mesh
                                        objects).
        --softwareRenderer=0            Pure software rendererer without
                                        OpenGL/GLFW.   Note: this mode disables
                                        GUI and screen renderering and only
                                        allows to create offscreen RGB videos.




```

## GUI

The GLFW window could be used with the next shortcuts: 

```

Azerty keyboard shortcuts (Querty):
 + General:                                      + Playback control:
  - h           : Help.                           - Space       : Play/Stop.
  - q|Escape (a): Exit.                           - Up/Down     : First/Last frame.
  - d           : Draw overlay.                   - Left/Right  : Previous/Next frame.
  - t           : Draw logs.                      - w        (z): Save video.
  - s           : Synchronize windows.
                                                 + Record camera path and view point:
 + Configure interface and rendering:             - y           : Save viewpoint.
  - F11         : Fullscreen.                     - u           : Start/Stop path recording.
  - x           : Axes.                           - i           : Insert fixed position.
  - b           : Bounding box and camera rig.    - k           : Insert position.
  - v           : Rotate circles.                 - l           : Load a preloaded path.
  - o           : Ortho/Perspective rendering.    - g           : Camera path rendering.
  - f           : Display duplicate points.       - m        (:): Segment/Spline camera path.
  - r           : Change rendering mode.          - p           : Draw camera path.
  - c           : Change background color.
  - n           : Change point cloud color.      + 3D background scene:
  - j           : lighting.                       - Mouse+Shift: Change position and rotation.
                                                  - Shift+s/x   : +/- scale.
 + Alignment mode:                                - Shift+d/c   : +/- X position.
  - 1-2         : Faces of the bounding box.      - Shift+f/v   : +/- Y position.
  - 3-4         : Horizontal octagon vertices.    - Shift+g/b   : +/- Z position.
  - 5-6         : Horizontal rotation.            - Shift+h/n   : +/- X rotation.
  - 7-8         : +/- auto-rotate speed.          - Shift+j/,(m): +/- Y rotation.
  - 9-0         : Zoom.                           - Shift+k/;(,): +/- Z rotation.
                                                  - Shift+w  (x): Save coordinates.
 + Multi-color mode:
  - Tab         : Switch between model colors:   + Distance mode between main and source ply:
                  main, closest, interpolate      - z        (w): Display distance.
                  or fixed color [0;N].           - a        (q): Switch.
  - Ctrl+Tab    : Main color.

 + Blended points rendering mode:
  - Page Up/Down: Increase/decrease alpha falloff
  - End         : Change blending mode

Mouse:
  - Left button : Rotate.             - Scroll|Center+Ctrl: Zoom.
  - Right button: Translate.          - Scroll+Alt        : Point size.
  

```

## Examples

The next section presents some command lines to perform rendering: 

``` 
./PccAppRenderer -f longdress_vox10_%04d.ply  -n 32 -i 1051 
./PccAppRenderer -f /basketball_player_%08d.obj  -n 32 -i 1 
./PccAppRenderer -d ./ply/ -n 32 
./PccAppRenderer -d ./ply/ -n 32 --play=1 --playBackward=1 -o video
``` 

## Create video

The camera path parameters could be used to generate videos. The next command line shows an example of this functionality:

``` 
./PccAppRenderer.exe \
  -f ../longdress_fr%04d.obj \
  -n 6 
  -i 1051 \
  --camera=cfg/plane.txt\
  --spline=1 \
  --RgbFile=test 
``` 

The generated videos are RGB48. To facilitate the creation of the rendered video and the usage of these sequences, the two following scripts have been updated:
-	`./scripts/renderer.sh`
-	`./scripts/convert_video.sh`

The first script starts the rendering software in a video recording mode that saves the rendered screen to a raw video file.

The second script converts the created raw videos into more usable video formats: 
-	0: rgb 16bits i444
-	1: yuv 10bits p420
-	2: HEVC/X265 Lossless
-	3: HEVC/X265 QP18
-	4: Graphics interchange format (GIF)
-	5: zipped yuv 10bits p420

The format of the generated videos could be configured with the script parameter: `--videotype=[0;5]`.

This second script is automatically executed by the `./scripts/renderer.sh` script to convert the created videos. The default value of the renderer software and of these scripts have been fixed in alignment with the conditions defined in the mesh CFP (background color, floor color,…).

The `./scripts/renderer.sh` script can be used to generate and to convert the rendered video:  

```
./scripts/renderer.sh: Render PLY/OBJ sequences based on camera path and create video

    Usage:
       ./scripts/renderer.sh -i <INPUT> [-o <OUTPUT>] [-h]

    Parameters:
       -h|--help             : help message.
       -i|--input=*          : input dir
       -o|--output=*         : suffix of the name of the created video
       -w|--width=*          : width of the created video
       -h|--height=*         : height of the created video
       -r|--rotation=*       : rotation
       -f|--frame=*          : frame number
       -v|--viewpoint=*      : viewpoint file
       -x|--camera=*         : camera path file
       -t|--cameraPathIndex=*: camera path index
       -t|--videoType=*      : video type:
                               - 0: rgb 16bits
                               - 1: yuv 10bits
                               - 2: HEVC/X265 Lossless
                               - 3: HEVC/X265 QP18
                               - 4: Graphics interchange format (GIF)
                               - 5: zipped yuv 10bits
       -pb|--playbackward=*  : playbackward
       --floor=*             : draw floor box.
       --spline=*            : interpolate camera path by b-splines.
       --lighting=*          : enable lighting.
       --softwareRenderer=*  : use software rendering process
       --force               : recreate video if file already exists.
       --novideo             : open renderer user interface.
       -b|--binary=*         : open binary version of the pointcloud files
       --threads=*           : ffmpeg threads.
       --ffmpeg=*            : ffmpeg path.

    Examples:
      - ./scripts/renderer.sh -i input.rgb
      - ./scripts/renderer.sh -i input.rgb -o output.yuv

```

For example, the following command line reads the 300 first objects of the longdress sequence, plays the sequence by moving the camera following the camera path defined in the ./cfg/circle.txt file and creates a yuv420p10le video: video_1920x1080_10bit_p420.yuv: 

```
./scripts/renderer.sh \
    -i ./londress/ \
    -f 300 \
    -x ./cfg/plane_short.txt \
    -o video \
    --videotype=1
```

To facilitate the usage of the camera paths, preloaded camera path mode has been added in the mpeg renderer.

This mode could be used by adding the input parameter: –cameraPathIndex=index, and automatically build a 300 frames camera path based on the bounding box of the sequences. 

For the CFP sequences, the following parameters could be used to create the camera paths corresponding to the orientation of the sequences and to the motions:

| Sequences         | Camera path indexes  |
|-------------------|----------------------|
| longdress         |     10               |
| soldier           |     10               |
| mitch             |     10               |
| thomas            |     10               |
| levi              |     10               |
| basketball_player |     11               |
| dancer            |     12               |
| football          |     13               |

The camera path index could be used as follow:

```
./scripts/renderer.sh \
    -i ./londress/ \
    -f 0 \
    --cameraPathIndex=10 \
    -o video \
    --videotype=1
```

The next figure shows examples of video sequences generated with the previous settings:

![Alt Text](https://drive.google.com/uc?export=view&id=16TtRXjNVb3nmFpCsRjX_YV0504ol0i6I) ![Alt Text](https://drive.google.com/uc?export=view&id=1VOenxc6uQ07uvZUoM0sBNzGlJayy7d93)

## Notes

Please, don't hesitate to report me any issues with the renderer by the GitLab Issue Tracker or directly by [email](mailto:jricard@global.tencent.com).

## Reference

Some additional informations could be found in the [./doc/InterDigital_PointCloud_Renderer_v6.0.pdf](http://mpegx.int-evry.fr/software/MPEG/PCC/mpeg-pcc-renderer/blob/master/doc/InterDigital_PointCloud_Renderer_v6.0.pdf). 


## License
The copyright in this software is being made available under the BSD License, included below. This software may be subject to InterDigital and other third party and contributor rights, including patent rights, and no such rights are granted under this license.

Copyright (c) 2016-2025, InterDigital All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of ISO/IEC nor the names of the Project where this contribution had been made may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

