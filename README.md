# Target tracker
This code uses feature detection and matching to track an objects. It starts by loading a target image and then detects features in both the target and each frame of the video. Then match these features to find the target in the video, filters out bad matches, and computes the average position of the good matches.

Still a work in progress**
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
