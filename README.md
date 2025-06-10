# SmoothLife
## What is it?
An implementation of the cellular automata defined in the paper ["Generalization of Conway's "Game of Life" to a continuous - "SmoothLife"](https://arxiv.org/pdf/1111.1567) and assisted by [Tsoding streams](https://www.youtube.com/watch?v=L68_BBiuHUw&pp=ygUTVHNvZGluZyBzbW9vdGggbGlmZQ%3D%3D)! Essentially the it shares the same operations as the original Game of Life but instead of each cell being either alive or dead, each cell takes a value between being alive and dead. This value is represented as a floating point number between 0 and 1 with 0 being dead and 1 being alive.
## Pre requisites
  - MakeFile
  - SFML 2.6.1
## Tutorial
There are 2 supported modes of compilation (Threaded and unthreaded). The threaded mode will compile the version of the code that implements multithreading while the unthreaded mode will compile the single threaded version.
### To Compile Unthreaded
`make main`
### To Compile Threaded
`make threaded`
# **_IMPORTANT_**
The current unthreaded implementation saves the frames produced to a bunch of PNG(s) in the output_frames folder for to create timelapses. To stop this comment out lines 83 to 85 in the main.cpp. This does not apply to the threaded version.
