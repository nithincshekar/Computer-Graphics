Assignment #3: Ray tracing

FULL NAME: Nithin Chandrashekhar


MANDATORY FEATURES
------------------

<Under "Status" please indicate whether it has been implemented and is
functioning correctly.  If not, please explain the current status.>

Feature:                                 Status: finish? (yes/no)
-------------------------------------    -------------------------
1) Ray tracing triangles                  yes

2) Ray tracing sphere                     yes

3) Triangle Phong Shading                 yes

4) Sphere Phong Shading                   yes

5) Shadows rays                           yes

6) Still images                           yes
   
7) Extra Credit (up to 20 points)
   !!! explain your extra credit here, if applicable !!!
Implemented anti-aliasing by taking color of neighbouring pixel color and averaging it.
There is a flag called "ANTIALIAS_FLAG" in line number 58 in the .cpp file. Can modify this flag to get with/without anti-aliasing rendering.
images with anti-aliasing are named with _naa.jpg and without anti-aliasing are named as scene_name.jpg.