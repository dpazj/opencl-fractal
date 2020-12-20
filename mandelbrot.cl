
__kernel void mandelbrot(__write_only image2d_t img, const float xscale, const float yscale, const float xoffset, const float yoffset)
{
    uint3 p[5]; //for some reason initializing inline gives strange colours {(uint3)(178,216,216), (uint3)(102,178,178), (uint3)(0,128,128), (uint3)(0,102,102), (uint3)(0,76,76)};
    p[0] = (uint3)(178,216,216);
    p[1] = (uint3)(102,178,178);
    p[2] = (uint3)(0,128,128);
    p[3] = (uint3)(0,102,102);
    p[4] = (uint3)(0,76,76);


    int Px = get_global_id(0);
    int Py = get_global_id(1);
    int2 size = get_image_dim(img);
    uint4 colour = (uint4)(255,255,255,255);

    //from https://en.wikipedia.org/wiki/Mandelbrot_set#Computer_drawings
    float x0 = ((float) Px / size.x)* (3.5 * xscale) - 2.0 + xoffset;
    float y0 = ((float) Py / size.y) * (2 * yscale) - 1 + yoffset;

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

    colour.xyz = p[iterations%5];

    write_imageui(img, (int2)(Px,Py) , colour);
}