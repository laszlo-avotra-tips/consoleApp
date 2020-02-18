#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QString>
#include <QFile>

#include <map>
#include <memory>


class SignalManager : public QObject
{
    Q_OBJECT

public:
    static SignalManager* instance();

    float *getFftRealDataPointer() const;

    float *getFftImagDataPointer() const;

    size_t getFftMemSize() const;
    bool isFftSource()const;

    bool loadFftSignalBuffers(int index);

    std::map<int,bool> getIsFftDataInitializedFromGpu() const;
    void setIsFftDataInitializedFromGpu(bool isFftDataInitializedFromGpu, int index);

    bool open();
    void close();

public slots:
    void saveSignal(int count);

signals:
    void signalSaved();
    void signalLoaded();

private:
    SignalManager();

private:
    static SignalManager* m_instance;
    std::unique_ptr<float[]> m_fftRealData{nullptr};
    std::unique_ptr<float[]> m_fftImagData{nullptr};
    const qint64 m_dataLen;
    bool m_isSource;
    std::pair<QString,QString> m_fftFileName;
    QFile m_imagFile;
    QFile m_realFile;
    QString m_temp;
    std::map<int,bool> m_isFftDataInitializedFromGpu;

};

#endif // SIGNALMANAGER_H
