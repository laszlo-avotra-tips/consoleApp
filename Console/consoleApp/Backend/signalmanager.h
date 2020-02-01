#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QString>
#include <QFile>

#include <map>


class SignalManager : public QObject
{
    Q_OBJECT

public:
    static SignalManager* instance();

    float *getRealDataPointer() const;

    float *getImagDataPointer() const;

    size_t getFftMemSize() const;
    bool isFftSource()const;
    bool isPreScalingSource()const;

    float *getLastFramePrescaling() const;
    bool loadSignal();


    std::map<int,bool> getIsFftDataInitializedFromGpu() const;
    void setIsFftDataInitializedFromGpu(bool isFftDataInitializedFromGpu, int index);

public slots:
    void saveSignal();

signals:
    void signalSaved();
    void signalLoaded();

private:
    SignalManager();

private:
    static SignalManager* m_instance;
    float* m_realData;
    float* m_imagData;
    float* m_lastFramePrescaling;
    const size_t m_dataLen;
    bool m_isSource;
    const std::pair<QString,QString> m_fftFileName;
    QFile m_imagFile;
    QFile m_realFile;
    QString m_temp;
    std::map<int,bool> m_isFftDataInitializedFromGpu;

};

#endif // SIGNALMANAGER_H
