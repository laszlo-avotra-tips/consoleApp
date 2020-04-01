#ifndef SCANCONVERSION_H
#define SCANCONVERSION_H

#include <QDebug>
#include <QThread>
#include "qmath.h"
#include "defaults.h"
#include <CL/opencl.h>
#include <QDir>
#include "octFile.h"
#include <imagedescriptor.h>


class ScanConversion: public QThread
{
    Q_OBJECT

public:
    ScanConversion();

    void mapBufferToSector( unsigned char *pDataIn, unsigned char *pDataOut );
    bool isLutGenerated( void ) { return lutGenerated; }
    void generateLutRadiusTheta( int maxDepth, int numberOfLines, int catheterRadius, int w, int h );
    bool warpData( OCTFile::OctData_t *dataFrame, size_t pBufferLength );
    bool isReady;

public slots:
    void handleDisplayAngle( float angle, int direction );

private:
    float **rLUT;
    float **tLUT;

    int sectorDimension;
    bool lutGenerated;
    float displayAngle_deg;
    int reverseDirection;

    // opencl
    bool buildOpenCLKernel( QString clSourceFile, char *kernelName, cl_program *program, cl_kernel *kernel );
    char *loadCLProgramSourceFromFile( QString );
    bool initOpenCL();
    bool createCLMemObjects( cl_context context );
    bool createCLMemObjectsOld(cl_context);
    cl_mem  warpInputImageMemObj;
    cl_mem  outputImageMemObj;
    cl_mem  outputVideoImageMemObj;
    cl_kernel cl_WarpKernel;
    cl_device_id cl_ComputeDeviceId;
    size_t cl_max_workgroup_size;
    cl_context cl_Context;
    cl_command_queue cl_Commands;
    cl_program       cl_WarpProgram;
    cl_image_format deviceSpecificImageFormat;
    cl_mem_flags deviceSpecificMemFlags;

    cl_mem     lineAvgInputMemObj;
    cl_program cl_LineAvgProgram;
    cl_kernel  cl_LineAvgKernel;
    ImageDescriptor m_imageDescriptor;

};

#endif // SCANCONVERSION_H
