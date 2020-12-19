# opencl-fractal
A simple open cl program to create fractals: 
  * Mandelbrot
  * Newton
  * More to come???

#Build and run
```
make 
./fract
```

#Usage
```
Usage: ./fract -d (run with default settings) -p <fractal pattern name> -xs <x scale> --xo <x offset> --f <out filename> -w <img width> -h <img height>
```
##example
```
./fract -p newton -xs 1.5 -ys 1.5 -h 5000 -w 5000 -f file.png
```

# mandelbrot example
![](/mandelbrot.png)

# newton example
![](/newton.png)
