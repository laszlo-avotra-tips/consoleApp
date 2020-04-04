#ifndef IMAGEDESCRIPTOR_H
#define IMAGEDESCRIPTOR_H

#include <CL/opencl.h>

struct ImageDescriptor
{
    ImageDescriptor();

    const cl_image_desc* getFftImageDescriptor() const;
    const cl_image_desc* getBeAndCeImageDescriptor() const;
    const cl_image_desc* getWarpImageDescriptor() const;
    const cl_image_desc* getOutputVideoImageDescriptor() const;
    const cl_image_desc* getOutputImageDescriptor() const;

    const cl_image_format* getImageFormat() const;
    const cl_image_desc* getInputImageDescriptor() const;

private:
    const cl_image_format m_imageFormat{CL_R,CL_UNSIGNED_INT8};

    const cl_mem_object_type m_imageType{CL_MEM_OBJECT_IMAGE2D};

    const size_t m_imageWidth; //input_image_width
    const size_t m_imageHeight; //input_image_height
    const size_t m_warpImageWidth;
    const size_t m_warpImageHeight;
    const size_t m_imageDepth{1};
    const size_t m_imageArraySize{1};
    const size_t m_imageRowPitch{0};
    const size_t m_imageSlicePitch{0};
    const cl_uint m_numMipLevels{0};
    const cl_uint m_numSamples{0};

    cl_mem m_buffer{nullptr};

    const cl_image_desc m_inputImageDescriptor; //inputImageDescriptor
    const cl_image_desc warpImageDescriptor;
};

#endif // IMAGEDESCRIPTOR_H
