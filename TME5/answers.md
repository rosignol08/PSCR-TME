# TME5 Correction: Parallelization of a Ray Tracer

## Baseline Sequential

### Question 1
Extracted TME5.zip from Moodle, added to repo, committed and pushed.

Configured project with CMake as previous TMEs. No dependencies, all handmade.

### Question 2

Ran `build/TME5` and generated `spheres.bmp`.

Platform: 
PC Ryzen 3 3300U 4 core 4 threads.
Temps/baseline choisi :
Total time 2918ms.

## With Manual Threads

### Question 3
Implemented `void renderThreadPerPixel(const Scene& scene, Image& img)` in Renderer.

Avec 200 * 200 :

Total time 1331ms.

Avec Ryzen 3 3100 4 core 8 threads.
./TME5 -mThreadPerPixel -W 200 -H 200 -n 8
Ray tracer starting with output 'spheres.bmp', resolution 200x200, spheres 250, mode ThreadPerPixel
Total time 1535ms.

### Question 4
Implemented `void renderThreadPerRow(const Scene& scene, Image& img)` in Renderer.

mode ThreadPerRow
en 200 * 200

Total time 268ms.

Avec Ryzen 3 3100 4 core 8 threads.
./TME5 -mThreadPerRow -W 200 -H 200 -n 8
Ray tracer starting with output 'spheres.bmp', resolution 200x200, spheres 250, mode ThreadPerRow
Total time 9ms.


### Question 5
Implemented `void renderThreadManual(const Scene& scene, Image& img, int nbthread)` in Renderer.

mode ThreadManual 200 * 200

Total time 208ms.

Avec Ryzen 3 3100 4 core 8 threads.

./TME5 -mThreadManual -W 200 -H 200 -n 8
Ray tracer starting with output 'spheres.bmp', resolution 200x200, spheres 250, mode ThreadManual, threads 8
Total time 6ms.

## With Thread Pool

### Question 6
Queue class: blocking by default, can switch to non-blocking.

### Question 7
Pool class: constructor with queue size, start, stop.
Job abstract class with virtual run().

### Question 8
PixelJob: derives from Job, captures ?TODO?

renderPoolPixel: 

Mode "-m PoolPixel" with -n.

mode PoolPixel, threads 4

Total time 234ms.

Avec Ryzen 3 3100 4 core 8 threads.

./TME5 -mPoolPixel -W 200 -H 200 -n 8
Ray tracer starting with output 'spheres.bmp', resolution 200x200, spheres 250, mode PoolPixel, threads 8
Total time 45ms.


### Question 9
LineJob: derives from Job, captures TODO

renderPoolRow: 

mode PoolRow, threads 4
Total time 208ms.

Mode "-m PoolRow -n nbthread".

Avec Ryzen 3 3100 4 core 8 threads.

./TME5 -mPoolRow -W 200 -H 200 -n 8
Ray tracer starting with output 'spheres.bmp', resolution 200x200, spheres 250, mode PoolRow, threads 8
Total time 6ms.


### Question 10
Best:
4 thread car 4 cores sur ma machine sinon c'est plus lent legèrement.

## Bonus

### Question 11

pool supportant soumission de lambda.
