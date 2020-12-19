#define CL_TARGET_OPENCL_VERSION 220

#include <CL/cl.h>
#include "lodepng.h"

#include <vector> 
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>

//https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

std::vector<std::string> VALID_PATTERNS = {"mandelbrot", "newton"};

 
int main(int argc, char **argv) {

    size_t img_height = 1080;
    size_t img_width = 1920;
    size_t TS = 10; //just being lazy : ) 

    float xoffset = 0;
    float yoffset = 0;
    float xscale = 1;
    float yscale = 1;

    std::string frac_pattern = "mandelbrot";
    std::string filename = "fract.png";

    //CMD LINE ARGS
    if(argc < 2)
    {
        std::cout << "Usage ./fract -d (run with default settings) -p <fractal pattern name> -xs <x scale> --xo <x offset> --f <out filename> -w <img width> -h <img height>" << std::endl;
        std::cout << "\nAvailable fractal patterns:\nmandelbrot\nnewton" << std::endl;
        
        return 1;
    }
    char * option;

    option = getCmdOption(argv, argv + argc, "-p");
    if (option)
    {
        std::string pat(option);
        std::vector<std::string>::iterator it = std::find(VALID_PATTERNS.begin(), VALID_PATTERNS.end(), pat);
        if(it != VALID_PATTERNS.end())
        {
            frac_pattern = pat;
        }
        else
        {
            std::cout << "Invalid pattern" << std::endl;
            std::cout << "fractal pattern names:\nmandelbrot\nnewton" << std::endl;
            return 1;
        }
    }

    option = getCmdOption(argv, argv + argc, "-f");
    if (option)
    {
        filename = std::string(option);
    }

    option = getCmdOption(argv, argv + argc, "-xs");
    if (option)
    {
        xscale = atof(option);
    }

    option = getCmdOption(argv, argv + argc, "-xo");
    if (option)
    {
        xoffset = atof(option);
    }

    option = getCmdOption(argv, argv + argc, "-ys");
    if (option)
    {
        yscale = atof(option);
    }

    option = getCmdOption(argv, argv + argc, "-yo");
    if (option)
    {
        yoffset = atof(option);
    }

    option = getCmdOption(argv, argv + argc, "-w");
    if (option)
    {
        
        img_width = (size_t) std::stoi(option);
    }

    option = getCmdOption(argv, argv + argc, "-h");
    if (option)
    {
        img_height = (size_t) std::stoi(option);
    }
    if(img_width %10 != 0 || img_height %10 != 0 )
    {
        std::cout << "image height/width must be divisible by 10!" << std::endl;
        return 1;
    }



    

    //OPENCL 


    std::vector<unsigned char> img_data(img_height * img_width * 4); //RGB IMAGE

    cl_image_format img_format;
    img_format.image_channel_order = CL_RGBA;
    img_format.image_channel_data_type = CL_UNSIGNED_INT8;


    cl_image_desc img_desc;
    img_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    img_desc.image_width = img_width;
    img_desc.image_height = img_height;
    //img_desc.image_depth = 0;
    img_desc.image_array_size = 0;
    img_desc.image_row_pitch = 0;
    img_desc.image_slice_pitch = 0;
    img_desc.num_mip_levels = 0;
    img_desc.num_samples = 0;
    img_desc.buffer = NULL;


    // Load the kernel source code into the array source_str
    //std::ifstream file("fract.cl");
    std::ifstream file(frac_pattern + ".cl");

    std::string source_string;
    std::string line;
    while(std::getline(file, line))
    {
        source_string += line + "\n";
    }

    // Get platform and device information

    std::string device_name(500, 'x');
    size_t device_name_length = 0;

    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;   

    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_DEFAULT, 1, 
            &device_id, &ret_num_devices);


    clGetDeviceInfo( device_id, CL_DEVICE_NAME, device_name.size(), &device_name.front(), &device_name_length);
    device_name.resize(device_name_length);

    std::cout << "Platform ID: " << platform_id << std::endl;

    std::cout << "Device ID: " << device_id <<  std::endl;
    std::cout << "Device name: " << device_name <<  std::endl;
    

    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &ret);
 

    auto x = source_string.size();
    auto* ptr = &source_string[0];
    cl_program program = clCreateProgramWithSource(context, 1, (const char **) &ptr, (const size_t *)&x, &ret);


    ret = clBuildProgram(program, 1, &device_id, NULL ,NULL, NULL);
    if(ret != CL_SUCCESS)
    {
        std::cout << "Could not build the kernel program" << std::endl;
    } 

    cl_kernel kernel = clCreateKernel(program, frac_pattern.c_str(), &ret); 


    cl_mem cl_img = clCreateImage(context, CL_MEM_WRITE_ONLY, &img_format, &img_desc, NULL, &ret);
    if(ret != CL_SUCCESS)
    {
        std::cout << "Error creating image" << std::endl;
        return 1;
    }
    ret = clSetKernelArg(kernel, 0, sizeof(cl_img), &cl_img);
    ret = clSetKernelArg(kernel, 1, sizeof(float), &xscale);
    ret = clSetKernelArg(kernel, 2, sizeof(float), &yscale);
    ret = clSetKernelArg(kernel, 3, sizeof(float), &xoffset);
    ret = clSetKernelArg(kernel, 4, sizeof(float), &yoffset);


    const size_t global_item_size[2] = {img_width , img_height}; 
    const size_t local_item_size[2] = {TS, TS}; 
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);

    const size_t origin[3] = {0,0,0};
    const size_t region[3] = {img_width , img_height, 1};
    ret = clEnqueueReadImage(command_queue, cl_img, CL_TRUE, origin, region, 0, 0, img_data.data(), 0, NULL, NULL);

    ret = clFlush(command_queue);
    ret = clFinish(command_queue);

    std::cout << "Rendered image" << std::endl;

    //write image out
    std::cout << "Creating png and writing to disk..." << std::endl;
    unsigned error = lodepng::encode(filename, img_data, img_width, img_height);

 

    //Clean up

    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 0;
}