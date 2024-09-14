# Target tracker
This code uses feature detection and matching to track an objects. It starts by loading a target image and then detects features in both the target and each frame of the video. Then match these features to find the target in the video, filters out bad matches, and computes the average position of the good matches.

## Overview
1. Load target image
2. Extract features using SIFT
3. Find target in image by identifying matching keypoints from the reference image
4. Average of keypoints is center (filter outliers)
5. Update then becomes your reference (skipping this as its not stable) 
6. Reduce and update ROI for feature detection once target is found
7. Repeat 3-6 

![](assets/results.gif)

## To run:
1. Install [docker](https://docs.docker.com/engine/install/)

2. Clone repo

3. Build:
```
$ docker build -t track_target .
```

4. Run:
```
$ xhost +local:docker
$ docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix track_target
```
