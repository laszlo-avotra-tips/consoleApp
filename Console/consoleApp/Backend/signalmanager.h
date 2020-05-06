#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QList>
#include <QPointF>
#include <QMutex>

#include <map>
#include <memory>
#include <queue>

using FftSignalType = std::pair<int, std::pair<float*,float*>>; //<tag, <imag, real>>
using FftSignalQueueType = std::queue<FftSignalType>;

class SignalManager : public QObject
{
    Q_OBJECT

public:
    static SignalManager* instance();

    float *getFftRealDataPointer() const;

    float *getFftImagDataPointer() const;

    const QList<QPointF>& getAdvancedViewFftPlotList() const;

    size_t getFftMemSize() const;
    bool isFftSource()const;

    bool loadFftSignalBuffers();

    bool open();
    void close();

    bool isSignalQueueEmpty() const;
    bool isSignalQueueLengthLTE(size_t length) const;
    bool isSignalQueueLengthGTE(size_t length) const;

    const FftSignalType &frontOfSignalContainer() const;
    void  popSignalContainer();
    void  pushSignalContainer(const FftSignalType& signal);

    bool isPlotListEmpty() const;
    bool setAdvancedViewFftPlotList(const uint8_t* data);

    QMutex* getMutex();

signals:
    void signalSaved();
    void signalLoaded();

private:
    SignalManager();
    void updateAdvancedViewFftPlotList(const FftSignalType& fft);

private:
    static SignalManager* m_instance;
    std::unique_ptr<float[]> m_fftRealData{nullptr};
    std::unique_ptr<float[]> m_fftImagData{nullptr};
    const qint64 m_dataLen{1184 * 1024};
    std::pair<QString,QString> m_fftFileName;
    QFile m_imagFile;
    QFile m_realFile;
    QString m_temp;
    FftSignalQueueType m_fftSignalQueue;
    int m_signalTag{0};
    QList<QPointF> m_advancedViewFftPlotList;
    QMutex m_mutex;

};

#endif // SIGNALMANAGER_H
