#define CL_TARGET_OPENCL_VERSION 220

#include <CL/cl.h>
#include "lodepng.h"

#include <vector> 
#include <fstream>
#include <string>
#include <iostream>

 
int main(void) {
    
    const int img_height = 1080;
    const int img_width = 1920;
    const int TS = 30;

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
    std::ifstream file("fract.cl");

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
    std::cout << ret << std::endl;


    ret = clBuildProgram(program, 1, &device_id, NULL ,NULL, NULL);
    if(ret != CL_SUCCESS)
    {
        std::cout << "Could not build the kernel program" << std::endl;
    }
    std::cout << ret << std::endl;
 

    cl_kernel kernel = clCreateKernel(program, "mandelbrot", &ret);



    cl_mem cl_img = clCreateImage(context, CL_MEM_WRITE_ONLY, &img_format, &img_desc, NULL, &ret);
    if(ret != CL_SUCCESS)
    {
        std::cout << "Error createing image" << std::endl;
        return 1;
    }
    ret = clSetKernelArg(kernel, 0, sizeof(cl_img), &cl_img);

    const size_t global_item_size[2] = {img_width , img_height}; 
    const size_t local_item_size[2] = {TS, TS}; 
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);

    const size_t origin[3] = {0,0,0};
    const size_t region[3] = {img_width , img_height, 1};
    ret = clEnqueueReadImage(command_queue, cl_img, CL_TRUE, origin, region, 0, 0, img_data.data(), 0, NULL, NULL);

    ret = clFlush(command_queue);
    ret = clFinish(command_queue);

    std::cout << "done" << std::endl;

    //write image out
    unsigned error = lodepng::encode("test.png", img_data, img_width, img_height);

 

    //Clean up

    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 0;
}