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

SignalManager::SignalManager()
{
    m_fftFileName = std::pair<QString,QString>("C:/Avinger_System/signal/imag.char", "C:/Avinger_System/signal/real.char");

    m_fftRealData = std::make_unique<float []>(size_t(m_dataLen));
    m_fftImagData = std::make_unique<float []>(size_t(m_dataLen));

    m_imagFile.setFileName(m_fftFileName.first);
    m_realFile.setFileName(m_fftFileName.second);

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

bool SignalManager::isSignalQueueEmpty() const
{
    return m_fftSignalQueue.empty();
}

bool SignalManager::isSignalQueueLengthLTE(size_t length) const
{
    return m_fftSignalQueue.size() <= length;
}

const FftSignalType& SignalManager::frontOfSignalContainer() const
{
    return m_fftSignalQueue.front();
}

void SignalManager::popSignalContainer()
{
    m_fftSignalQueue.pop();
}

void SignalManager::pushSignalContainer(const FftSignalType &signal)
{
    m_fftSignalQueue.push(signal);
}

bool SignalManager::loadFftSignalBuffers()
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

    if(m_imagFile.atEnd()){
        m_imagFile.seek(0);
        m_realFile.seek(0);
        m_signalTag = 0;
//        LOG1(m_signalTag)
    }

    FftSignalType thisFft{m_signalTag,{m_fftImagData.get(), m_fftRealData.get()}};
    pushSignalContainer(thisFft);
    emit signalLoaded();
    ++m_signalTag;

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
