#ifndef ENGINEERINGCONTROLLER_H
#define ENGINEERINGCONTROLLER_H

#include <QWidget>
#include <QTimer>

class EngineeringDialog;
class EngineeringModel;

class EngineeringController : public QObject
{
    Q_OBJECT

public:
    EngineeringController(QWidget *parent = nullptr);
    void setViewPosition(int x = 400, int y = 0);
    void showOrHideView(bool isShown = true);

private slots:
    void saveFrameBuffers();
    void loadFrameBuffers();
    void playbackStartStopCommand(bool isStart);
    void setPlaybackSpeed(int speed);
    void handleSaveDataToFile();
    void saveDataToFile();

private:
    void startPlayback();
    void stopPlayback();

    EngineeringDialog* m_view;
    EngineeringModel* m_model;
    bool m_isGeometrySet;
};

#endif // ENGINEERINGCONTROLLER_H
