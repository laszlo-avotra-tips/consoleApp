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
}

SignalManager *SignalManager::instance()
{
    if(!m_instance){
        m_instance = new SignalManager();
    }
    return m_instance;
}

SignalManager::SignalManager():
    m_realData(nullptr),m_imagData(nullptr),m_lastFramePrescaling(nullptr),m_dataLen(592*2048),
    m_fftFileName(std::pair<QString,QString>(SignalDir + "imag.float", SignalDir + "real.float"))
//  m_fftFileName(std::pair<QString,QString>(SignalDir + "imag.char", SignalDir + "real.char"))
//  m_fftFileName(std::pair<QString,QString>(SignalDir + "imag.dat", SignalDir + "real.dat"))
{
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

float *SignalManager::getLastFramePrescaling() const
{
    return m_lastFramePrescaling;
}

void SignalManager::saveSignal()
{
    const auto& signalTable = getIsFftDataInitializedFromGpu();
    for ( auto it = signalTable.begin(); it != signalTable.end(); ++it){
        if(it->second){
            auto index = it->first;
            LOG3(index, m_fftFileName.first, m_fftFileName.second);
            const size_t len(m_dataLen);
            if(!isFftSource()){
                if(m_imagFile.open(QIODevice::WriteOnly)){
                    QTextStream ifs(&m_imagFile);
                    ifs << m_dataLen << " "<< index <<endl;
                    for(size_t i = 0; i < len; ++i){
                        ifs << *m_imagData++ << endl;
                    }
                }
                if(m_realFile.open(QIODevice::WriteOnly)){
                    QTextStream ifs(&m_realFile);
                    ifs << m_dataLen << " "<< index <<endl;
                    for(size_t i = 0; i < len; ++i){
                        ifs << *m_realData++ << endl;
                    }
                }
            }
            LOG3(index, m_fftFileName.first, m_fftFileName.second);
            m_imagFile.close();
            m_realFile.close();
            emit signalSaved();
        }
    }
}

bool SignalManager::loadSignal()
{
    bool success(true);

    LOG2(m_fftFileName.first, m_fftFileName.second);
    size_t leni(1212416);
    size_t lenr(1212416);
    int indexi(0);
    int indexr(0);
    if(isFftSource()){
        if(m_imagFile.open(QIODevice::ReadOnly)){
            QDataStream qds(&m_imagFile);
            for(qint64 i = 0; i < leni; ++i){
                qds >> m_imagData[i];
            }
//            char* pImag = reinterpret_cast<char*>(m_imagData);
//            qint64 readCount(leni * sizeof(float));
//            m_imagFile.read(pImag, readCount);
        }
        if(m_realFile.open(QIODevice::ReadOnly)){

            QDataStream qds(&m_realFile);
            for(qint64 i = 0; i < leni; ++i){
                qds >> m_realData[i];
            }

//            char* pReal = reinterpret_cast<char*>(m_realData);
//            qint64 readCount(lenr * sizeof(float));
//            m_realFile.read(pReal, readCount);
        }
    }
    m_imagFile.close();
    m_realFile.close();
    if(indexi == indexr){
        LOG3(indexi,m_fftFileName.first, m_fftFileName.second);
        if(isFftSource()){
            TheGlobals::instance()->pushFrameDataQueue(indexi);
        } else {
            emit signalLoaded();
        }
    }

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

bool SignalManager::isPreScalingSource() const
{
    return false;
}

float *SignalManager::getImagDataPointer() const
{
    return m_imagData;
}

float *SignalManager::getRealDataPointer() const
{
    return m_realData;
}
