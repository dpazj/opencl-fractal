
//complex type from http://www.davidespataro.it/complex-number-opencl/
typedef float2 cl_complex;


inline cl_complex cl_complex_multiply(const cl_complex* a, const cl_complex* b)
{
    cl_complex res;
    res.x = a->x * b->x - a->y * b->y;
    res.y = a->x * b->y + a->y * b->x;
    return res;
}

inline cl_complex cl_complex_pow(const cl_complex* base, int exp)
{
    cl_complex res;
    res.x = base->x;
    res.y = base->y;

    for(int i = 0; i < exp-1; i++)
    {
        res = cl_complex_multiply(&res, base);
    }
    return res;
}

inline cl_complex cl_complex_divide(const cl_complex* a, const cl_complex* b)
{
    const float div = (b->x*b->x + b->y*b->y);
    return (cl_complex) ((a->x*b->x + a->y*b->y)/div , (a->y*b->x - a->x*b->y)/div);
}


// //Newton fractal from https://en.wikipedia.org/wiki/Newton_fractal



cl_complex roots(int i)
{
   // i = i % 3;
    switch(i)
    {
        case 0: return (cl_complex) (1, 0);
        case 1: return (cl_complex) (-0.5, sqrt(3.0f)/2 );
        case 2: return (cl_complex) (-0.5, -sqrt(3.0f) /2);  
    }
}


// //z^3 - 1
inline cl_complex func(const cl_complex* z)
{
    return cl_complex_pow(z, 3) - ((cl_complex) (1.0f, 0.0f));
}

// // 3*z^2
inline cl_complex derivative(const cl_complex* z)
{
    return 3 * cl_complex_multiply(z, z);
}

float abs_complex(const cl_complex* a)
{
    return sqrt(a->x * a->x + a->y*a->y);
}

__kernel void newton(__write_only image2d_t img, const float xscale, const float yscale, const float xoffset, const float yoffset)
{
    uint3 p[3]; //for some reason initializing inline gives strange colours {(uint3)(178,216,216), (uint3)(102,178,178), (uint3)(0,128,128), (uint3)(0,102,102), (uint3)(0,76,76)};
    p[0] = (uint3)(178,216,216);
    p[1] = (uint3)(102,178,178);
    p[2] = (uint3)(0,128,128);

    int Px = get_global_id(0);
    int Py = get_global_id(1);
    int2 size = get_image_dim(img);
    uint4 colour = (uint4)(0,0,0,255);


    float x0 = ((float) Px / size.x)* (3.5 * xscale) - 2.0 + xoffset;
    float y0 = ((float) Py / size.y) * (2 * yscale) - 1 + yoffset;

    cl_complex z;
    z = (cl_complex) (x0, y0);


    cl_complex test;
    test.x = 2;
    test.y = 2;
    test = cl_complex_pow(&test, 7);

    //printf("%f %f\n", test.x, test.y);

    int maxIteration = 1000;
    
    float tolerance = 0.000001;
    for(int iteration = 0; iteration < maxIteration; iteration++)
    {
        cl_complex f = func(&z);
        cl_complex fx = derivative(&z);
        z -= cl_complex_divide(&f, &fx);

        for(int i=0; i < 3; i++)
        {
            cl_complex difference = z - roots(i);
            if(fabs(difference.x) < tolerance && fabs(difference.y) < tolerance)
            {
                colour.xyz = p[i%3];
            }
        }
        
    }

    write_imageui(img, (int2)(Px,Py) , colour);
}