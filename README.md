# Target tracker

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
