#ifndef CONCATENATEVIDEO_H
#define CONCATENATEVIDEO_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <memory>

class ConcatenateVideo : public QObject
{
    Q_OBJECT

public:
    static ConcatenateVideo* instance();

    QString outputPath() const;
    void setOutputPath(const QString &outputPath);

    QString inputPlaylistFile() const;
    void setInputPlaylistFile(const QString &inputPlaylistFile);

    QString outputLoopFile() const;
    void setOutputLoopFile(const QString &outputLoopFile);

    void execute();

private slots:
    void programFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    explicit ConcatenateVideo(QObject *parent = nullptr);

signals:

private:
    static ConcatenateVideo* m_instance;
    const QString m_concatenateProgramName{"concat_streams"};

    QString m_outputPath;           //All segment files should be located under this path
    QString m_inputPlaylistFile;    //concat_streams will parse this playlist file under the output path to collect all segment information
    QString m_outputLoopFile;       //Output loop file will be generated under the output path

    std::unique_ptr<QProcess> m_concatenateProcess{nullptr};
};

#endif // CONCATENATEVIDEO_H
