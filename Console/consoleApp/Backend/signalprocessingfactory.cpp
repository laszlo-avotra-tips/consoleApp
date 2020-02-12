#include "signalprocessingfactory.h"
#include <util.h>

#include <QDebug>
#include <QString>

#include <logger.h>

#include <postfft.h>

SignalProcessingFactory* SignalProcessingFactory::m_instance{nullptr};

SignalProcessingFactory *SignalProcessingFactory::instance()
{
    if(!m_instance){
        m_instance = new SignalProcessingFactory();
    }
    return m_instance;
}

IKernelFunction *SignalProcessingFactory::getPostFft()
{
//    LOG2(m_openClSuccess, bool(m_postFftKernelFunction))
    if(m_openClSuccess){
        if(!m_postFftKernelFunction){
            m_postFftKernelFunction = std::make_shared<PostFft>(m_context);
        }
        return m_postFftKernelFunction.get();
    }
    return nullptr;
}

SignalProcessingFactory::SignalProcessingFactory()
{
    int errorLine(__LINE__);
    initOpenClFileMap();

    m_platformId = getPlatformId();
    m_openClSuccess = m_platformId;

    if(m_openClSuccess){
        m_openClSuccess = getGpuDeviceInfo(m_platformId, true);
    }else{
        errorLine = __LINE__;
    }
    if(m_openClSuccess){
        cl_int err{-1};
        m_context = clCreateContext( nullptr, 1, &m_computeDeviceId, nullptr, nullptr, &err );
        m_openClSuccess = err == CL_SUCCESS;
    }else{
        errorLine = __LINE__;
    }
    if(!m_openClSuccess){
        displayFailureMessage( QString( "Could not allocate OpenCL compute context, reason %1" ).arg( errorLine ), true );
    }

    if(m_openClSuccess){
        cl_int err{-1};
        m_commandQueue = clCreateCommandQueueWithProperties( m_context, m_computeDeviceId, nullptr, &err );
        m_openClSuccess = err == CL_SUCCESS;
        if(err != CL_SUCCESS){
            displayFailureMessage( QString( "Could not create OpenCL command queue, reason %1" ).arg( err ), true );
        }
    }
}

void SignalProcessingFactory::initOpenClFileMap()
{
    m_openClFileMap = {
        {"postfft_kernel",  ":/kernel/postfft"},
        {"bandc_kernel",  ":/kernel/bandc"},
        {"warp_kernel",  ":/kernel/warp"}
    };
}

cl_platform_id SignalProcessingFactory::getPlatformId() const
{
    cl_platform_id platformId;
    cl_uint        numPlatforms = 0;

    cl_int err = clGetPlatformIDs( 0, nullptr, &numPlatforms );
    qDebug() << "numPlatforms =" << numPlatforms;

    if( numPlatforms == 0 )
    {
        // fatal error
        displayFailureMessage( QString( "Could not find openCL platform, reason: %1" ).arg( err ), true );
        return nullptr;
    }

    // Found openCL-capable platforms
    cl_platform_id* platformIds = new cl_platform_id [numPlatforms];
    err = clGetPlatformIDs( numPlatforms, platformIds, nullptr );

    uint deviceIndex = 99;

    const int DefaultStringSize = 128;
    char vendor[ DefaultStringSize ];
    char name[ DefaultStringSize ];
    char version[ DefaultStringSize ];

    for ( cl_uint i = 0; i < numPlatforms; i++ )
    {
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VENDOR,  DefaultStringSize, vendor,  nullptr );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_NAME,    DefaultStringSize, name,    nullptr );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VERSION, DefaultStringSize, version, nullptr );

        if ( err != CL_SUCCESS )
        {
            displayFailureMessage( QString( "Could not enumerate OpenCL platform IDs, reason: %1" ).arg( err ), true );
            return nullptr;
        }
        qDebug() << "Platform (" << i << ") Vendor:" << vendor << " Name:" << name << " Version:" << version;

        if ( QString( vendor ) == "Intel(R) Corporation" &&
             QString( name )   == "Intel(R) OpenCL" )
        {
            deviceIndex = i;
        }
    }

    if ( deviceIndex > 10 )
    {
        displayFailureMessage( QString( "Could not find gpu" ), true );
        return nullptr;
    }

    platformId = platformIds[ deviceIndex ];

    // release the memory.  Error paths do not free the memory since they will shut down the program
    delete [] platformIds;

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( QString( "Could not enumerate OpenCL platform IDs, reason: %1" ).arg( err ), true );
        return nullptr;
    }

    return platformId;
}

bool SignalProcessingFactory::getGpuDeviceInfo(cl_platform_id id, bool isLogging)
{
    // Verify the GPU is present
    cl_int err = clGetDeviceIDs( id, CL_DEVICE_TYPE_GPU, 1, &m_computeDeviceId, nullptr );

    // If not, fall back to the CPU. Display a warning if this occurs on the release hardware
    if( err == CL_DEVICE_NOT_FOUND )
    {
        // fall back to CPU when debugging if GPU not present
        err = clGetDeviceIDs( id, CL_DEVICE_TYPE_CPU, 1, &m_computeDeviceId, nullptr );
    }

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( QString( "Could not get OpenCL device IDs, reason: %1" ).arg( err ), true );
        return false;
    }

    size_t returned_size( 0 );
    cl_uint maxComputeUnits;
    err = clGetDeviceInfo( m_computeDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }
    if(isLogging) LOG2(maxComputeUnits,returned_size)

    cl_uint maxWorkItemDimentions;
    err = clGetDeviceInfo( m_computeDeviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxWorkItemDimentions), &maxWorkItemDimentions, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }
    if(isLogging) LOG2(maxWorkItemDimentions,returned_size)

    size_t maxWorkItemSizes[3];
    err = clGetDeviceInfo( m_computeDeviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }
    if(isLogging) LOG3(maxWorkItemSizes[0],maxWorkItemSizes[1],maxWorkItemSizes[2])

    err = clGetDeviceInfo( m_computeDeviceId,
                           CL_DEVICE_MAX_WORK_GROUP_SIZE,
                           sizeof( m_cl_max_workgroup_size ),
                           &m_cl_max_workgroup_size,
                           &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }


    size_t maxWorkGroupSize;
    err = clGetDeviceInfo( m_computeDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        displayFailureMessage( QString( "Could not enumerate OpenCL device IDs, reason: %1" ).arg( err ), true );
        return false;
    }
    if(isLogging) LOG2(maxWorkGroupSize,returned_size)

    cl_char vendor_name[ 1024 ] = { 0 };
    cl_char device_name[ 1024 ] = { 0 };
    err  = clGetDeviceInfo( m_computeDeviceId, CL_DEVICE_VENDOR, sizeof( vendor_name ), vendor_name, &returned_size);
    err |= clGetDeviceInfo( m_computeDeviceId, CL_DEVICE_NAME, sizeof( device_name ), device_name, &returned_size);

    if(!isClReturnValueSuccess(err,__LINE__)){
        displayFailureMessage( QString( "Could not get OpenCL device info, reason: %1").arg( err ), true );
        return false;
    }
    LOG( INFO, "OpenCL device: " + QString( reinterpret_cast<char*>(vendor_name)) + " " + QString( reinterpret_cast<char*>(device_name)) )
    if(isLogging) qDebug() << "DSP: Found OpenCL Device " <<  QString( reinterpret_cast<char*>(vendor_name) ) + " " + QString( reinterpret_cast<char*>(device_name) );

    return true;
}

bool SignalProcessingFactory::isClReturnValueSuccess(cl_int ret, int line) const
{
    QString errorMesage;
    QTextStream qts(&errorMesage);

    bool success{true};
    if(ret != CL_SUCCESS){
        success = false;
        qts << " Open CL failes at line " << line;
        LOG1(errorMesage)
    }
    return success;
}

cl_command_queue SignalProcessingFactory::getCommandQueue() const
{
    return m_commandQueue;
}

cl_device_id SignalProcessingFactory::getComputeDeviceId() const
{
    return m_computeDeviceId;
}

cl_context SignalProcessingFactory::getContext() const
{
    return m_context;
}
