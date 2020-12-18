
uint3 get_color(int iterations)
{
    //colour pallet from https://www.color-hex.com/color-palette/4666
    int colour_index = iterations % 5;
    switch(colour_index)
    {
        case 0: return (uint3)(178,216,216);
        case 1: return (uint3)(102,178,178); 
        case 2: return (uint3)(0,128,128); 
        case 3: return (uint3)(0,102,102); 
        case 4: return (uint3)(0,76,76); 
    }
}



__kernel void mandelbrot(__write_only image2d_t img)
{
    int Px = get_global_id(0);
    int Py = get_global_id(1);
    int2 size = get_image_dim(img);
    uint4 colour = (uint4)(255,255,255,255);

    //from https://en.wikipedia.org/wiki/Mandelbrot_set#Computer_drawings
    float x0 = ((float) Px / size.x)* 3.25 - 2.0;
    float y0 = ((float) Py / size.y) * 2.0 -1.0;

    float x = 0.0; 
    float y = 0.0;

    int iterations = 0;
    int max_iteration = 1000;

    while(x*x + y*y <= 4.0 && iterations < max_iteration)
    {
        float xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iterations++; 
    }

    colour.xyz = get_color(iterations);

    write_imageui(img, (int2)(Px,Py) , colour);
}