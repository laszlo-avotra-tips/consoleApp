#include "scanconversion.h"
#include "deviceSettings.h"
#include "depthsetting.h"
#include "daq.h"

int gCounter = 1;

#define DEFAULT_LOCAL_UNITS  ( 16 ) // size of number of buffers from DSPGPU
size_t global_unit_dim[] = { FFT_DATA_SIZE, FFT_DATA_SIZE };
size_t local_unit_dim[]  = { DEFAULT_LOCAL_UNITS,  DEFAULT_LOCAL_UNITS  };

ScanConversion::ScanConversion()
{
    isReady = false;
    lutGenerated = false;
    displayAngle_deg = 0.0f;
    reverseDirection = 0;

    if( initOpenCL() )
    {
        isReady = true;
        qDebug() << "initOpenCL() Complete.";
    }
}

/*
 * generateLutRadiusTheta()
 */
/*
void ScanConversion::generateLutRadiusTheta( int maxDepth, int numberOfLines, int catheterRadius, int w, int h )
{
    // Square sector, so sectorDimension_x = sectorDimension_y = sectorDimension.
    sectorDimension = SECTOR_HEIGHT_PX;


    // create the two dimensional arrays

    rLUT = new float* [ sectorDimension ];
    tLUT = new float* [ sectorDimension ];
    for( int i = 0; i < sectorDimension; i++ )
    {
        rLUT[ i ] = new float[ sectorDimension ];
        tLUT[ i ] = new float[ sectorDimension ];
    }

    // offset for the catheter center
    int offset = sectorDimension/2;

    float maxOfRadius = 0;
    float maxOfTheta  = 0;

    qDebug() << "DSPGPU LUT: ";
    qDebug() << "           maxDepth:       " << maxDepth;
    qDebug() << "           numberOfLines:  " << numberOfLines;
    qDebug() << "           catheterRadius: " << catheterRadius;
    qDebug() << "           w:              " << w;
    qDebug() << "           h:              " << h;
    qDebug() << "           offset:         " << offset;


    for( int xi = 0; xi < sectorDimension; xi++ )
    {
        for( int yi = 0; yi < sectorDimension; yi++ )
        {
            int x = xi - offset;
            int y = yi - offset;

            rLUT[ xi ][ yi ] = qSqrt( ( x*x + y*y ) ) - catheterRadius;
            tLUT[ xi ][ yi ] = qAtan2( y, x );

            // do not allow radius beyond the depth limits
            if( rLUT[ xi ][ yi ] > ( maxDepth + catheterRadius ) || rLUT[ xi ][ yi ] < 0 )
            {
                rLUT[ xi ][ yi ] = 0;
            }

            // do not allow negative angles
            if( tLUT[ xi ][ yi ] < 0 )
            {
                tLUT[ xi ][ yi ] = tLUT[ xi ][ yi ] + 2 * 3.14;
            }

            // Find maximum values for Radius and Theta
            if( rLUT[ xi ][ yi ] > maxOfRadius )
            {
                maxOfRadius = rLUT[ xi ][ yi ];
            }
            if( tLUT[ xi ][ yi ] > maxOfTheta )
            {
                maxOfTheta = tLUT[ xi ][ yi ];
            }
        }
    }

    // Normalize rLUT to the proper depth
    for( int xi = 0; xi < sectorDimension; xi++ )
    {
        for( int yi = 0; yi < sectorDimension; yi++ )
        {
            // Normalize rLUT to the proper depth
            rLUT[ xi ][ yi ] = rLUT[ xi ][ yi ] / maxOfRadius * maxDepth;

            // normalize tLUT to number of A lines. angles in terms of A lines.
            tLUT[ xi ][ yi ] = tLUT[ xi ][ yi ] / ( 2 * 3.14 ) * numberOfLines;
        }
    }

    lutGenerated = true;
}
*/
/*
 * mapBufferToSector()
 *
 * Walk down the dimensions of the output image (sector). Find the Radius and Angle from LUT,
 * and map at those dimensions from input image to output image (B-scan to Sector).
 *
 * This algorithm speed does not depend on the input image size (6000 lines vs 1000 lines makes
 * no difference), since it operates based on the output image.
 *
 * TODO:
 *      * Interpolation (only uses nearest neighbor currently)
 *      * Adjust output image size.
 */
/*
void ScanConversion::mapBufferToSector( unsigned char *pDataIn, unsigned char *pDataOut )
{
    for( int yi = 0; yi < sectorDimension; yi++ )
    {
        for( int xi = 0; xi < sectorDimension; xi++ )
        {
            int ri, ti; // don't need to round to nearest if we use integers, this will cast floats to int;

            ri = rLUT[ xi ][ yi ];
            ti = tLUT[ xi ][ yi ];

            if( ri < 0 )
            {
                ri = 0; // don't index into negatives
            }
            if( ti < 0 )
            {
                ti = 0; // don't index into negatives
            }

            // lookup into the sector
            // XXX: This results in stack overflow if gFlattenedImage is not the full size.
            pDataOut[ ( yi * sectorDimension ) + xi ] = pDataIn[ ( ti * FFT_DATA_SIZE ) + ri ];
        }
    }
}
*/
bool ScanConversion::initOpenCL()
{
    qDebug() << "initOpenCL start";

    QString path = "";

    cl_platform_id platformId;
    cl_uint        numPlatforms = 0;

    int err = clGetPlatformIDs( 0, NULL, &numPlatforms );
    qDebug() << "numPlatforms =" << numPlatforms;

    if( numPlatforms == 0 )
    {
        qDebug() << "Could not find openCL platform, reason: " << err;
        return false;
    }

    // Found openCL-capable platforms
    cl_platform_id* platformIds = ( cl_platform_id* )malloc( sizeof( cl_platform_id ) * numPlatforms );
    err = clGetPlatformIDs( numPlatforms, platformIds, NULL );

    int amdIdx = -1;

    const int DefaultStringSize = 128;
    char vendor[ DefaultStringSize ];
    char name[ DefaultStringSize ];
    char version[ DefaultStringSize ];

    for ( cl_uint i = 0; i < numPlatforms; i++ )
    {
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VENDOR,  DefaultStringSize, vendor,  NULL );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_NAME,    DefaultStringSize, name,    NULL );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VERSION, DefaultStringSize, version, NULL );

        if ( err != CL_SUCCESS )
        {
            qDebug() << "Could not enumerate OpenCL platform IDs, reason: " << err;
            return false;
        }
        qDebug() << "Platform (" << i << ") Vendor:" << vendor << " Name:" << name << " Version:" << version;

        if( ( QString( vendor ) == "Intel(R) Corporation" &&    // Intel integrated GPU compatibility
              QString( name )   == "Intel(R) OpenCL" ) ||
            ( QString( vendor ) == "Apple" &&                   // Apple compatibility
              QString( name )   == "Apple" ) )
        {
            amdIdx = i;

            qDebug() << "Compatible platform found. Platform ID: " << amdIdx;
        }
    }

    // Fatal error if no AMD-compatible platforms are found
    if ( amdIdx < 0 )
    {
        qDebug() << "Could not find AMD platform";
        return false;
    }

    platformId = platformIds[ amdIdx ];

    // release the memory.  Error paths do not free the memory since they will shut down the program
    free( platformIds );

    if( err != CL_SUCCESS )
    {
        qDebug() << "Could not enumerate OpenCL platform IDs, reason: " << err;
        return false;
    }

    // Verify the GPU is present
    err = clGetDeviceIDs( platformId, CL_DEVICE_TYPE_GPU, 1, &cl_ComputeDeviceId, NULL );

    // If not, fall back to the CPU. Display a warning if this occurs on the release hardware
    if( err == CL_DEVICE_NOT_FOUND )
    {
        // fall back to CPU when debugging if GPU not present
        qDebug() << "GPU not present.  Falling back to the CPU.";
        err = clGetDeviceIDs( platformId, CL_DEVICE_TYPE_CPU, 1, &cl_ComputeDeviceId, NULL );
    }

    if( err != CL_SUCCESS )
    {
        qDebug() << "Could not get OpenCL device IDs, reason: " << err;
        return false;
    }

    size_t returned_size( 0 );
    err = clGetDeviceInfo( cl_ComputeDeviceId,
                           CL_DEVICE_MAX_WORK_GROUP_SIZE,
                           sizeof( size_t ),
                           &cl_max_workgroup_size,
                           &returned_size );

    if( err != CL_SUCCESS )
    {
        qDebug() << "Could not enumerate OpenCL device IDs, reason: " << err;
        return false;
    }

    cl_char vendor_name[ 1024 ] = { 0 };
    cl_char device_name[ 1024 ] = { 0 };
    err  = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_VENDOR, sizeof( vendor_name ), vendor_name, &returned_size);
    err |= clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_NAME, sizeof( device_name ), device_name, &returned_size);

    if( err != CL_SUCCESS )
    {
        qDebug() << "Could not get OpenCL device info, reason: " << err;
        return false;
    }

    qDebug() << "DSP: Found OpenCL Device " <<  QString( (char *)vendor_name ) + " " + QString( (char *)device_name );

    if( QString( (char *)device_name ) == "Intel(R) HD Graphics 4600" )
    {
        qDebug() << "using image format for Gen 4 Intel";
        deviceSpecificImageFormat.image_channel_order = CL_R;
        deviceSpecificImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        deviceSpecificMemFlags = CL_MEM_COPY_HOST_PTR;
    }
    else if( QString( (char *)device_name ).contains( "Intel(R) Core(TM) i7" ) )
    {
        qDebug() << "using image format for Arbor Computer";
        deviceSpecificImageFormat.image_channel_order = CL_INTENSITY;
        deviceSpecificImageFormat.image_channel_data_type = CL_UNORM_INT8;
        deviceSpecificMemFlags = CL_MEM_READ_WRITE;
    }
    else if( QString( (char *)device_name ).contains( "Intel(R) HD Graphics 505" ) )
    {
        qDebug() << "using image format for Gen 7 Intel";
        deviceSpecificImageFormat.image_channel_order = CL_R;
        deviceSpecificImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        deviceSpecificMemFlags =  CL_MEM_COPY_HOST_PTR;
    }
    else if( QString( (char *)device_name ).contains( "Intel(R) Pentium(R) CPU J4205" ) )
    {
        qDebug() << "using image format for Intel Apollo Lake";
        deviceSpecificImageFormat.image_channel_order = CL_R;
        deviceSpecificImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        deviceSpecificMemFlags =  CL_MEM_COPY_HOST_PTR;
    }
    else
    {
        qDebug() << "unable to match device name. using default image format" << QString( (char *)device_name ) ;
        deviceSpecificImageFormat.image_channel_order = CL_R;
        deviceSpecificImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        deviceSpecificMemFlags = CL_MEM_COPY_HOST_PTR;
    }

    cl_Context = clCreateContext( 0, 1, &cl_ComputeDeviceId, NULL, NULL, &err );
    if( !cl_Context )
    {
        qDebug() << "DSP: OpenCL could not create compute context.";
        qDebug() << "Could not allocate OpenCL compute context, reason " << err;
        return false;
    }

//    cl_Commands = clCreateCommandQueue( cl_Context, cl_ComputeDeviceId, 0, &err );
    cl_Commands = clCreateCommandQueueWithProperties( cl_Context, cl_ComputeDeviceId, 0, &err );

    if( !cl_Commands )
    {
        qDebug() << "DSP: OpenCL could not create command queue. reason: " << err;
        return false;
    }

    char warpkernelname[] = "warp_kernel";

    if( !buildOpenCLKernel( QString( path + "warp.cl" ), warpkernelname, &cl_WarpProgram, &cl_WarpKernel ) )
    {
        return false;
    }

    char lineavgkernelname[] = "line_avg_kernel";
    if( !buildOpenCLKernel( QString( path + "line_avg.cl" ), lineavgkernelname, &cl_LineAvgProgram, &cl_LineAvgKernel ) )
    {
        qDebug() << "Unable to build line_avg.cl";
        return false;
    }
    qDebug() << "*****we got here " << __LINE__;

    createCLMemObjects( cl_Context );
    qDebug() << "*****we got here " << __LINE__;

    global_unit_dim[ 0 ] = FFT_DATA_SIZE;
    global_unit_dim[ 1 ] = MAX_LINES_PER_FRAME;

    qDebug() << "DSPGPU: OpenCL init complete.";

    return true;
}

bool ScanConversion::buildOpenCLKernel( QString clSourceFile, char *kernelName, cl_program *program, cl_kernel *kernel )
{
    qDebug() << "DSPGPU::buildOpenCLKernel:" << clSourceFile;
//    QTime buildTimer;
//    buildTimer.start();

    int err;


    // look for CL file
    QDir dir;
#ifdef Q_OS_MAC
    dir.setPath( "../../../" ); // OS dependent relative path between executable and CL files
#endif
    qDebug() << clSourceFile << "exists: " << dir.exists( clSourceFile ) << "location:" << dir.absoluteFilePath( clSourceFile );
    if( dir.exists( clSourceFile ) )
    {
        clSourceFile = dir.absoluteFilePath( clSourceFile );
    }

    /*
     * Load, compile, link the source
     */
    char *sourceBuf = loadCLProgramSourceFromFile( clSourceFile ); // XXX: We should switch to pre-compiled binary. See #1057
    if( !sourceBuf )
    {
        qDebug() << "Failed to load program source file: " << clSourceFile;
        return false;
    }
    qDebug() << "*****we got here " << __LINE__;

    /*
     * Create the compute program(s) from the source buffer
     */
    *program = clCreateProgramWithSource( cl_Context, 1, (const char **) &sourceBuf, NULL, &err );
    if( !*program || ( err != CL_SUCCESS ) )
    {
        qDebug() << "DSP: OpenCL could not create program from source: " << err;
        return false;
    }
    free( sourceBuf );
    qDebug() << "*****we got here " << __LINE__;

    err = clBuildProgram( *program, 0, NULL, NULL, NULL, NULL );
    if( err != CL_SUCCESS )
    {
        size_t length;
        const int BuildLogLength = 2048;
        char *build_log = (char *)malloc( BuildLogLength );

        qDebug() << "DSP: OpenCL build failed: " << err;
        clGetProgramBuildInfo( *program, cl_ComputeDeviceId, CL_PROGRAM_BUILD_LOG, BuildLogLength, build_log, &length );
        qDebug() << "openCl Build log:" << build_log;

        qDebug() << "Could not build program, reason: " << err;
        free( build_log );
        return false;
    }

    *kernel = clCreateKernel( *program, kernelName, &err );

    if( err != CL_SUCCESS )
    {
        qDebug() << "DSP: OpenCL could not create compute kernel: " << err;
        return false;
    }

//    qDebug() << "Build time:" << buildTimer.elapsed() << "ms";
    return true;
}

/*
 * loadCLProgramSourceFromFile
 */
char *ScanConversion::loadCLProgramSourceFromFile( QString filename )
{
    QFile     sourceFile( filename );
    QFileInfo sourceFileInfo( filename );

    char *sourceBuf;

    if( !sourceFile.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "Failed to load OpenCL source file " << filename;
        return NULL;
    }

    int srcSize = sourceFileInfo.size();

    // memory is freed by the calling routine
    sourceBuf = (char *)malloc( srcSize + 1 );
    sourceFile.read( sourceBuf, srcSize );
    sourceBuf[ srcSize ] = '\0'; // Very important!
    return sourceBuf;
}

bool ScanConversion::createCLMemObjectsOld( cl_context /*context*/ )
{
    int err{-1};
#if LINE_AVERAGING
    lineAvgInputMemObj     = clCreateBuffer( context, deviceSpecificMemFlags, FFT_DATA_SIZE * MAX_LINES_PER_FRAME, NULL, &err );
#endif
//    warpInputImageMemObj   = clCreateImage2D( context, deviceSpecificMemFlags, &deviceSpecificImageFormat, FFT_DATA_SIZE, MAX_LINES_PER_FRAME, 0, NULL, &err );
//    outputImageMemObj      = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &deviceSpecificImageFormat, SECTOR_HEIGHT_PX, SECTOR_HEIGHT_PX, 0, NULL, &err );
//    outputVideoImageMemObj = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &deviceSpecificImageFormat, SECTOR_HEIGHT_PX, SECTOR_HEIGHT_PX, 0, NULL, &err );

    if( err != CL_SUCCESS )
    {
        qDebug() << "Failed to create GPU image outputVideoImageMemObj, reason: " << err;
        return false;
    }

    return true;
}

bool ScanConversion::createCLMemObjects( cl_context context )
{
    int err{-1};

//    const cl_image_format imageFormat{CL_R,CL_UNSIGNED_INT8};

    const cl_mem_object_type imageType{CL_MEM_OBJECT_IMAGE2D};
    const size_t imageDepth{1};
    const size_t imageArraySize{1};
    const size_t imageRowPitch{0};
    const size_t imageSlicePitch{0};
    const cl_uint numMipLevels{0};
    const cl_uint numSamples{0};


#if LINE_AVERAGING
    lineAvgInputMemObj     = clCreateBuffer( context, deviceSpecificMemFlags, FFT_DATA_SIZE * MAX_LINES_PER_FRAME, NULL, &err );
#endif
////    warpInputImageMemObj   = clCreateImage2D( context, deviceSpecificMemFlags, &deviceSpecificImageFormat, FFT_DATA_SIZE, MAX_LINES_PER_FRAME, 0, NULL, &err );
//    {
////        const cl_image_format* inputImageFormat = m_imageDescriptor.getImageFormat();
////        const cl_image_desc* inputImageDescriptor = m_imageDescriptor.getInputImageDescriptor();

//        const size_t imageWidth{FFT_DATA_SIZE}; //input_image_width
//        const size_t imageHeight{MAX_LINES_PER_FRAME}; //input_image_height
//        cl_mem buffer{nullptr};

//        const cl_image_desc m_warpInputImageDescriptor{
//            imageType,
//            imageWidth,
//            imageHeight,
//            imageDepth,
//            imageArraySize,
//            imageRowPitch,
//            imageSlicePitch,
//            numMipLevels,
//            numSamples,
//            {buffer}
//        };

//        warpInputImageMemObj = clCreateImage( context, deviceSpecificMemFlags, &deviceSpecificImageFormat, &m_warpInputImageDescriptor, nullptr, &err );
//    }
//    if( err != CL_SUCCESS )
//    {
//        qDebug() << "Failed to create GPU image warpInputImageMemObj, reason: " << err;
//        return false;
//    }

//    outputImageMemObj      = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &deviceSpecificImageFormat, SECTOR_HEIGHT_PX, SECTOR_HEIGHT_PX, 0, NULL, &err );
    {
//        const cl_image_format* outputImageFormat = m_imageDescriptor.getImageFormat();
//        const cl_image_desc* outputImageDescriptor = m_imageDescriptor.getOutputImageDescriptor();

        const size_t imageWidth{SECTOR_HEIGHT_PX}; //input_image_width
        const size_t imageHeight{SECTOR_HEIGHT_PX}; //input_image_height
        cl_mem buffer{nullptr};

        const cl_image_desc outputImageDescriptor{
            imageType,
            imageWidth,
            imageHeight,
            imageDepth,
            imageArraySize,
            imageRowPitch,
            imageSlicePitch,
            numMipLevels,
            numSamples,
            {buffer}
        };

        outputImageMemObj = clCreateImage( context, CL_MEM_WRITE_ONLY, &deviceSpecificImageFormat, &outputImageDescriptor, nullptr, &err );
    }
    if( err != CL_SUCCESS )
    {
        qDebug() << "Failed to create GPU image outputImageMemObj, reason: " << err;
        return false;
    }

//    outputVideoImageMemObj = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &deviceSpecificImageFormat, SECTOR_HEIGHT_PX, SECTOR_HEIGHT_PX, 0, NULL, &err );
    {
//        const cl_image_format* outputVideoImageFormat = m_imageDescriptor.getImageFormat();
//        const cl_image_desc* outputVideoImageDescriptor = m_imageDescriptor.getWarpImageDescriptor();

        const size_t imageWidth{SECTOR_HEIGHT_PX}; //input_image_width
        const size_t imageHeight{SECTOR_HEIGHT_PX}; //input_image_height
        cl_mem buffer{nullptr};

        const cl_image_desc outputVideoImageDescriptor{
            imageType,
            imageWidth,
            imageHeight,
            imageDepth,
            imageArraySize,
            imageRowPitch,
            imageSlicePitch,
            numMipLevels,
            numSamples,
            {buffer}
        };

        outputVideoImageMemObj = clCreateImage( context, CL_MEM_WRITE_ONLY, &deviceSpecificImageFormat, &outputVideoImageDescriptor, nullptr, &err );

    }
    if( err != CL_SUCCESS )
    {
        qDebug() << "Failed to create GPU image outputVideoImageMemObj, reason: " << err;
        return false;
    }

//    warpInputImageMemObj   = clCreateImage2D( context, deviceSpecificMemFlags, &deviceSpecificImageFormat, FFT_DATA_SIZE, MAX_LINES_PER_FRAME, 0, NULL, &err );
    {
//        const cl_image_format* inputImageFormat = m_imageDescriptor.getImageFormat();
//        const cl_image_desc* inputImageDescriptor = m_imageDescriptor.getInputImageDescriptor();

        const size_t imageWidth{FFT_DATA_SIZE}; //input_image_width
        const size_t imageHeight{MAX_LINES_PER_FRAME}; //input_image_height
        cl_mem buffer{nullptr};

        const cl_image_desc m_warpInputImageDescriptor{
            imageType,
            imageWidth,
            imageHeight,
            imageDepth,
            imageArraySize,
            imageRowPitch,
            imageSlicePitch,
            numMipLevels,
            numSamples,
            {buffer}
        };

        warpInputImageMemObj = clCreateImage( context, deviceSpecificMemFlags, &deviceSpecificImageFormat, &m_warpInputImageDescriptor, nullptr, &err );
    }
    if( err != CL_SUCCESS )
    {
        qDebug() << "Failed to create GPU image warpInputImageMemObj, reason: " << err;
        return false;
    }

    return true;
}

bool ScanConversion::warpData( OCTFile::OctData_t *dataFrame, size_t pBufferLength )
{
    unsigned char *pDataIn = dataFrame->acqData;
    unsigned char *pDataOut = dataFrame->dispData;
    cl_int clStatus{-1};
    cl_int err{-1};

//    const cl_image_format imageFormat{CL_R,CL_UNSIGNED_INT8};

    const cl_mem_object_type imageType{CL_MEM_OBJECT_IMAGE2D};
    const size_t imageDepth{1};
    const size_t imageArraySize{1};
    const size_t imageRowPitch{0};
    const size_t imageSlicePitch{0};
    const cl_uint numMipLevels{0};
    const cl_uint numSamples{0};

//    qDebug() << "=================================";
//    for( int i = 0; i < FFT_DATA_SIZE * pBufferLength; i += (FFT_DATA_SIZE*128) )
//    {
//        qDebug() << (i/FFT_DATA_SIZE) << ":       warp data IN: " << pDataIn[ i + (FFT_DATA_SIZE/6) ] << pDataIn[ i + (FFT_DATA_SIZE/5) ] << pDataIn[ i + (FFT_DATA_SIZE/4) ] << pDataIn[ i + (FFT_DATA_SIZE/3) ] << pDataIn[ i + (FFT_DATA_SIZE/2) ]<< pDataIn[ i + (FFT_DATA_SIZE/1) ] ;
//    }

    /* RFR
     * Remember that buffer input lengths need to be multiples of 16 for some reason.
     * Line Average will fail if not using multiples of 16 for length.
     * subsampledBufferLength not required to be multiple of 16.
     * pBufferLength required to be multiple of 16.
     */

    size_t subsampledBufferLength = pBufferLength;


#if LINE_AVERAGING
    int numLinesToAverage = 3;
    const int MinNumLines = 1200; // always display at least 1200 lines per frame.
    if( (pBufferLength / numLinesToAverage) > MinNumLines )
    {
        subsampledBufferLength = pBufferLength / numLinesToAverage;
        qDebug() << "Enough lines to subsample. subsampledBufferLength:" << subsampledBufferLength << "pBufferLength: " << pBufferLength;
    }
    else
    {
        numLinesToAverage = 1; // not enough lines to apply averaging.
    }

    lineAvgInputMemObj = clCreateImage2D( cl_Context, deviceSpecificMemFlags, &deviceSpecificImageFormat, FFT_DATA_SIZE, pBufferLength, 0, pDataIn, &err );
    if( err != CL_SUCCESS )
    {
        qDebug() << "warpData: Failed to create lineAvgInputMemObj! Err = " << err;
        return false;
    }
#endif
//    for( int i = 0; i < FFT_DATA_SIZE * pBufferLength; i += (FFT_DATA_SIZE*128) )
//    {
//        qDebug() << (i/FFT_DATA_SIZE) << ":       warp data IN: " << pDataIn[ i + (FFT_DATA_SIZE/6) ] << pDataIn[ i + (FFT_DATA_SIZE/5) ] << pDataIn[ i + (FFT_DATA_SIZE/4) ] << pDataIn[ i + (FFT_DATA_SIZE/3) ] << pDataIn[ i + (FFT_DATA_SIZE/2) ]<< pDataIn[ i + (FFT_DATA_SIZE/1) ] ;
//    }

    // read the B-scan input data
//    warpInputImageMemObj = clCreateImage2D( cl_Context, deviceSpecificMemFlags, &deviceSpecificImageFormat, FFT_DATA_SIZE, subsampledBufferLength, 0, pDataIn, &err );
    {
//        const cl_image_format* warpImageFormat = m_imageDescriptor.getImageFormat();
//        const cl_image_desc* warpImageDescriptor = m_imageDescriptor.getWarpImageDescriptor();

        const size_t imageWidth{FFT_DATA_SIZE};
        const size_t imageHeight{subsampledBufferLength};
        cl_mem buffer{nullptr};

        const cl_image_desc warpInputImageDescriptor{
            imageType,
            imageWidth,
            imageHeight,
            imageDepth,
            imageArraySize,
            imageRowPitch,
            imageSlicePitch,
            numMipLevels,
            numSamples,
            {buffer}
        };
        warpInputImageMemObj = clCreateImage( cl_Context, deviceSpecificMemFlags, &deviceSpecificImageFormat, &warpInputImageDescriptor, pDataIn, &err );

    }

    if( err != CL_SUCCESS )
    {
        qDebug() << "Error: Failed to enqueue new data to GPU! Err = " << err;
        return false;
    }

#if LINE_AVERAGING
    clStatus  = clSetKernelArg( cl_LineAvgKernel, 0, sizeof(cl_mem), &lineAvgInputMemObj );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set LineAverage kernel arguments: arg0:" << clStatus;
        return false;
    }
    clStatus |= clSetKernelArg( cl_LineAvgKernel, 1, sizeof(cl_mem), &warpInputImageMemObj );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set LineAverage kernel arguments: arg1:" << clStatus;
        return false;
    }
    clStatus |= clSetKernelArg( cl_LineAvgKernel, 2, sizeof(int),    &numLinesToAverage );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set LineAverage kernel arguments: arg2:" << clStatus;
        return false;
    }
    global_unit_dim[ 0 ] = FFT_DATA_SIZE;
    global_unit_dim[ 1 ] = pBufferLength;

    if( numLinesToAverage > 1 )
    {
        clStatus = clEnqueueNDRangeKernel( cl_Commands, cl_LineAvgKernel, 2, NULL, global_unit_dim, NULL, 0, NULL, NULL );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to execute LineAverage kernel:" << clStatus << " buffer len, subsampled len:" << pBufferLength << subsampledBufferLength;
            return false;
        }
        else
        {
            qDebug() << "                   LINEAVERAGE Execute SUCCESS: buffer len, subsampled len:" << pBufferLength << subsampledBufferLength;
        }
    }
    else
    {
        // don't process, instead read the warp image.
    }
#endif

/*
 * Set up Catheter specific parameters
 */
    deviceSettings &device = deviceSettings::Instance();
    int index = device.getCurrentDevice();
    float catheterRadius_um = device.deviceAt(index)->getCatheterRadius_um();
    float internalImagingMask_px = device.deviceAt(index)->getInternalImagingMask_px();
    float standardDepth_mm = device.deviceAt( index )->getImagingDepthNormal_mm();
    const int standardDepth_S = device.deviceAt( index )->getALineLengthNormal_px();
    int SectorWidth_px = SECTOR_HEIGHT_PX;
    int SectorHeight_px = SECTOR_HEIGHT_PX;
    depthSetting &depth = depthSetting::Instance();
    float fractionOfCanvas = depth.getFractionOfCanvas();
    int imagingDepth_S = depth.getImagingDepth_S();
    int brightness = device.getBrightness(); // lcv TODO init
    int contrast =   device.getContrast();

    float displayAngle = displayAngle_deg;
/*
    if( reverseDirection == 1 )
    {
        displayAngle = displayAngle_deg + device.deviceAt(index)->getReverseAngle().toFloat();
    }
    else
    {
        displayAngle = displayAngle_deg;
    }
*/
//    qDebug() << "Direction:" << reverseDirection << "Angle:" << displayAngle;

    clStatus  = clSetKernelArg( cl_WarpKernel,  0, sizeof(cl_mem), &warpInputImageMemObj );
    clStatus |= clSetKernelArg( cl_WarpKernel,  1, sizeof(cl_mem), &outputImageMemObj );
    clStatus |= clSetKernelArg( cl_WarpKernel,  2, sizeof(cl_mem), &outputVideoImageMemObj );
    clStatus |= clSetKernelArg( cl_WarpKernel,  3, sizeof(float),  &catheterRadius_um );
    clStatus |= clSetKernelArg( cl_WarpKernel,  4, sizeof(float),  &internalImagingMask_px );
    clStatus |= clSetKernelArg( cl_WarpKernel,  5, sizeof(float),  &standardDepth_mm );
    clStatus |= clSetKernelArg( cl_WarpKernel,  6, sizeof(int),    &standardDepth_S );
    clStatus |= clSetKernelArg( cl_WarpKernel,  7, sizeof(float),  &displayAngle );
    clStatus |= clSetKernelArg( cl_WarpKernel,  8, sizeof(int),    &reverseDirection );
    clStatus |= clSetKernelArg( cl_WarpKernel,  9, sizeof(int),    &SectorWidth_px );
    clStatus |= clSetKernelArg( cl_WarpKernel, 10, sizeof(int),    &SectorHeight_px );
    clStatus |= clSetKernelArg( cl_WarpKernel, 11, sizeof(float),  &fractionOfCanvas );
    clStatus |= clSetKernelArg( cl_WarpKernel, 12, sizeof(int),    &imagingDepth_S );
    clStatus |= clSetKernelArg( cl_WarpKernel, 13, sizeof(int),    &brightness );
    clStatus |= clSetKernelArg( cl_WarpKernel, 14, sizeof(int),    &contrast );

    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set warp kernel arguments:" << clStatus;
        return false;
    }

//    qDebug() << "===========2";

    global_unit_dim[ 0 ] = SectorWidth_px;
    global_unit_dim[ 1 ] = SectorHeight_px;

    clStatus = clEnqueueNDRangeKernel( cl_Commands, cl_WarpKernel, 2, NULL, global_unit_dim, local_unit_dim, 0, NULL, NULL );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to execute warp kernel:" << clStatus;
        return false;
    }

//    qDebug() << "===========3";

    // Do all the work that was queued up on the GPU
    clFinish( cl_Commands );

//    qDebug() << "===========4";

    size_t origin[ 3 ] = { 0, 0, 0 };
    size_t region[ 3 ] = { SECTOR_HEIGHT_PX, SECTOR_HEIGHT_PX, 1 };

    /*
     * read out the display frame
     */
    clStatus = clEnqueueReadImage( cl_Commands, outputImageMemObj, CL_TRUE, origin, region, 0, 0, pDataOut, 0, NULL, NULL );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to read back final image data from warp kernel: " << clStatus;
        return false;
    }

//    qDebug() << "=================================";
//    for( int i = 0; i < SECTOR_HEIGHT_PX * SECTOR_HEIGHT_PX; i += (SECTOR_HEIGHT_PX*64) )
//    {
//        qDebug() << (i/SECTOR_HEIGHT_PX) << ":       warp data OUT: " << pDataOut[ i + (SECTOR_HEIGHT_PX/6) ] << pDataOut[ i + (SECTOR_HEIGHT_PX/5) ] << pDataOut[ i + (SECTOR_HEIGHT_PX/4) ] << pDataOut[ i + (SECTOR_HEIGHT_PX/3) ] << pDataOut[ i + (SECTOR_HEIGHT_PX/2) ]<< pDataOut[ i + (SECTOR_HEIGHT_PX/1) ] ;
//    }

    clReleaseMemObject( warpInputImageMemObj );
#if LINE_AVERAGING
    clReleaseMemObject( lineAvgInputMemObj );
#endif
    return true;
}

void ScanConversion::handleDisplayAngle( float angle, int direction )
{
    qDebug() << "Change display angle:" << angle << direction;
    displayAngle_deg = angle;
    reverseDirection = direction;
}
