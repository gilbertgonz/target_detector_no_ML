# Target detection using no ML
This code uses feature detection and matching to detect a target object. Old concept, but still pretty cool to see run in almost real-time.

## Overview
1. Load target/reference image (one can also implement human input for ROI)
2. Extract features using SIFT
3. Find target in image by identifying matching keypoints from the reference image
4. Average of keypoints is center (filter outliers)
5. Update reference image (skipping this as its not stable, see explaination below) 
6. Reduce and update ROI for feature detection once target is found
7. Repeat 3-6 

* idea for future: implement KF track and smooth detections

## Results
![](assets/results.gif)

### Reference image:
![](assets/target1.png)

* Tried to dynamically update the reference image since the reference/target can change over time, but this proved to be unstable in fast-moving scenarios. Might revisit later.

* Also tried dynamically adjusting the bounding box based on coordinates of feature detections, but proved to be unstable in scenarios with too few matches which made the ROI too small for detections.
  * **However**, in scenarios where the camera was intentionally following the target, like in sports replays, etc., results were more promising, see below for results after some tweaking with parameters:

![](assets/result2.gif)

As you can see on the bottom right, the "reference" image is being updated dynamically, instead of just referencing a single image like the previous example was doing.

## To run:
1. Install [docker](https://docs.docker.com/engine/install/)

2. Clone repo

3. Build:
```
$ docker build -t target_detector .
```

4. Run:
```
$ xhost +local:docker
$ docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix target_detector
```

## Dataset:
https://github.com/VisDrone/VisDrone-Dataset?tab=readme-ov-file
