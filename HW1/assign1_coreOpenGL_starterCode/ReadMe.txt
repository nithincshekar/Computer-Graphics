HW1: Terrain (DEM-based) rendering
Student Name: Nithin Chandrashekhar

The main implementation is in hw1.cpp file.
The points, wireframes and triangles can be rendered by pressing the following keys.
p - points
w - wireframes
t - triangles
o - wireframes over tirangles
r - reset to the initial rendering
a - toggle animation.

Animation
For animating, I am rotating over all 3 axes. That is, in the idleFunc, I am incrementing theta by 1 with respect to x, y and z axis.

Mouse and Keyboard events
Mouse left and middle button can be pressed and dragged to modify the rendering(with or without pressing the control and shift keys).