#ifndef FORMSECONDMONITOR_H
#define FORMSECONDMONITOR_H

#include <QWidget>

class QGraphicsView;
class liveScene;

namespace Ui {
class FormSecondMonitor;
}

class LiveSceneView : public QWidget
{
    Q_OBJECT

public:
    explicit LiveSceneView(QWidget *parent = nullptr);
    ~LiveSceneView();
    void setScene(liveScene* scene);
    void setRuntimeLabel(const QString &msg);
    void setCurrentTime(const QString &msg);
    void setDevice(const QString &msg);
    void setLabelLiveColor(const QString &msg);
    void setBorderForRecording(const QString& styleSheet);
    void setEnableRecording(bool isEnabled);
    void setCheckRecording(bool isChecked);
    void pushButtonRecord_clicked(bool isChecked);
    void setSpeedVisible(bool isVisible);
    void setSpeed(const QString &speed);

private:
    Ui::FormSecondMonitor *ui;
};

#endif // FORMSECONDMONITOR_H
