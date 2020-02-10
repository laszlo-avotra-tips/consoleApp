#include "signalmanager.h"
#include <QString>
//#include <QTextStream>
#include <QDataStream>
#include "logger.h"
#include <QTime>
#include "theglobals.h"

SignalManager* SignalManager::m_instance = nullptr;

namespace {
QString SignalDir("C:/Avinger_System/signal/");
bool isText(false);
bool isFloat(false);
bool isChar(true);
}

SignalManager *SignalManager::instance()
{
    if(!m_instance){
        m_instance = new SignalManager();
    }
    return m_instance;
}

SignalManager::SignalManager():
    m_realData(nullptr),m_imagData(nullptr),m_lastFramePrescaling(nullptr),m_dataLen(592*2048)
//  m_fftFileName(std::pair<QString,QString>(SignalDir + "imag.float", SignalDir + "real.float"))
//  m_fftFileName(std::pair<QString,QString>(SignalDir + "imag.char", SignalDir + "real.char"))
//  m_fftFileName(std::pair<QString,QString>(SignalDir + "imag.dat", SignalDir + "real.dat"))
{
    if(isText){
        m_fftFileName = std::pair<QString,QString>(SignalDir + "imag.dat", SignalDir + "real.dat");
    }

    if(isFloat){
         m_fftFileName = std::pair<QString,QString>(SignalDir + "imag.float", SignalDir + "real.float");
    }

    if(isChar){
        m_fftFileName = std::pair<QString,QString>(SignalDir + "imag.char", SignalDir + "real.char");
    }

    m_realData = new float[m_dataLen];
    m_imagData = new float[m_dataLen];
    m_lastFramePrescaling = new float[m_dataLen];

    m_imagFile.setFileName(m_fftFileName.first);
    m_realFile.setFileName(m_fftFileName.second);

}

std::map<int,bool> SignalManager::getIsFftDataInitializedFromGpu() const
{
    return m_isFftDataInitializedFromGpu;
}

void SignalManager::setIsFftDataInitializedFromGpu(bool isFftDataInitializedFromGpu, int index)
{
    m_isFftDataInitializedFromGpu[index] = isFftDataInitializedFromGpu;
}

bool SignalManager::open()
{
    bool success{false};
    if(isFftSource()){
        success = m_imagFile.open(QIODevice::ReadOnly);
        if(success){
            m_realFile.open(QIODevice::ReadOnly);
        }
    }
    return success;
}

void SignalManager::close()
{
    m_imagFile.close();
    m_realFile.close();
}

float *SignalManager::getLastFramePrescaling() const
{
    return m_lastFramePrescaling;
}

void SignalManager::saveSignal(int count)
{
    const auto& signalTable = getIsFftDataInitializedFromGpu();
    auto it = signalTable.find(count);
    if(it != signalTable.end()){
        if(it->second){
            auto index = it->first;
            LOG3(index, m_fftFileName.first, m_fftFileName.second);
            if(!isFftSource()){
                bool iFileSuccess(false);
                bool rFileSuccess(false);
                if(count == 0){
                    iFileSuccess = m_imagFile.open(QIODevice::WriteOnly);
                } else{
                    iFileSuccess = (m_imagFile.open(QIODevice::WriteOnly | QIODevice::Append));
                }
                if(iFileSuccess){
                    QTime durationTimer;
                    durationTimer.start();
                    auto countBytesWritten = m_imagFile.write(reinterpret_cast<char*>(m_imagData), m_dataLen * int(sizeof(float)));
                    auto imagSignalSaveDuration = durationTimer.elapsed();
                    LOG2(countBytesWritten,imagSignalSaveDuration);
                    m_imagFile.close();
                }
                if(count == 0){
                    rFileSuccess = m_realFile.open(QIODevice::WriteOnly);
                } else{
                    rFileSuccess = (m_realFile.open(QIODevice::WriteOnly | QIODevice::Append));
                }
                if(rFileSuccess){
                    QTime durationTimer;
                    durationTimer.start();
                    auto countBytesWritten = m_realFile.write(reinterpret_cast<char*>(m_realData), m_dataLen * int(sizeof(float)));
                    auto realSignalSaveDuration = durationTimer.elapsed();
                    LOG2(countBytesWritten, realSignalSaveDuration);
                    m_realFile.close();
                }
                if(iFileSuccess && rFileSuccess){
                    emit signalSaved();
                }
            }
            LOG3(index, m_fftFileName.first, m_fftFileName.second);
        }
    }
}

bool SignalManager::loadSignal(int index)
{
    bool success(true);

    const size_t len(1212416);

    QTime durationTimer;
    const bool isLogging{false};

    if(isLogging){
        LOG2(m_fftFileName.first, m_fftFileName.second)
                durationTimer.start();
    }

    auto imagDataSize = m_imagFile.read(reinterpret_cast<char*>(m_imagData), len * int(sizeof(float)));

    if(isLogging){
        auto imagFileReadDuration = durationTimer.elapsed();
        LOG2(imagDataSize,imagFileReadDuration)
        durationTimer.start();
    }

    auto realDataSize = m_realFile.read(reinterpret_cast<char*>(m_realData), len * int(sizeof(float)));

    if(isLogging){
        auto realFileReadDuration = durationTimer.elapsed();
        LOG2(realDataSize,realFileReadDuration)
    }

    TheGlobals::instance()->pushFrameDataQueue(index);
    emit signalLoaded();

    return success;
}



size_t SignalManager::getFftMemSize() const
{
    return m_dataLen * sizeof (float);
}

bool SignalManager::isFftSource() const
{
    return true;
}

float *SignalManager::getImagDataPointer() const
{
    return m_imagData;
}

float *SignalManager::getRealDataPointer() const
{
    return m_realData;
}
