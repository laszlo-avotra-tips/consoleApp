#include "signalmanager.h"
#include <QString>
#include <QDataStream>
#include "logger.h"
#include <QTime>
#include "theglobals.h"

SignalManager* SignalManager::m_instance = nullptr;


SignalManager *SignalManager::instance()
{
    if(!m_instance){
        m_instance = new SignalManager();
    }
    return m_instance;
}

SignalManager::SignalManager():
    m_fftRealData(nullptr),m_fftImagData(nullptr),m_dataLen(592*2048)
{
    m_fftFileName = std::pair<QString,QString>("C:/Avinger_System/signal/imag.char", "C:/Avinger_System/signal/real.char");

    m_fftRealData = std::make_unique<float []>(size_t(m_dataLen));
    m_fftImagData = std::make_unique<float []>(size_t(m_dataLen));

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

void SignalManager::saveSignal(int count)
{
    const auto& signalTable = getIsFftDataInitializedFromGpu();
    auto it = signalTable.find(count);
    if(it != signalTable.end()){
        if(it->second){
            auto index = it->first;
            LOG3(index, m_fftFileName.first, m_fftFileName.second)
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
                    auto countBytesWritten = m_imagFile.write(reinterpret_cast<char*>(m_fftImagData.get()), m_dataLen * int(sizeof(float)));
                    auto imagSignalSaveDuration = durationTimer.elapsed();
                    LOG2(countBytesWritten,imagSignalSaveDuration)
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
                    auto countBytesWritten = m_realFile.write(reinterpret_cast<char*>(m_fftRealData.get()), m_dataLen * int(sizeof(float)));
                    auto realSignalSaveDuration = durationTimer.elapsed();
                    LOG2(countBytesWritten, realSignalSaveDuration)
                    m_realFile.close();
                }
                if(iFileSuccess && rFileSuccess){
                    emit signalSaved();
                }
            }
            LOG3(index, m_fftFileName.first, m_fftFileName.second)
        }
    }
}

bool SignalManager::loadFftSignalBuffers(int index)
{
    bool success(true);

    const size_t len(1212416);

    QTime durationTimer;
    const bool isLogging{false};

    if(isLogging){
        LOG2(m_fftFileName.first, m_fftFileName.second)
                durationTimer.start();
    }

    auto imagDataSize = m_imagFile.read(reinterpret_cast<char*>(m_fftImagData.get()), len * int(sizeof(float)));

    if(isLogging){
        auto imagFileReadDuration = durationTimer.elapsed();
        LOG2(imagDataSize,imagFileReadDuration)
        durationTimer.start();
    }

    auto realDataSize = m_realFile.read(reinterpret_cast<char*>(m_fftRealData.get()), len * int(sizeof(float)));

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
    return size_t(m_dataLen) * sizeof (float);
}

bool SignalManager::isFftSource() const
{
    return true;
}

float *SignalManager::getFftImagDataPointer() const
{
    return m_fftImagData.get();
}

float *SignalManager::getFftRealDataPointer() const
{
    return m_fftRealData.get();
}
