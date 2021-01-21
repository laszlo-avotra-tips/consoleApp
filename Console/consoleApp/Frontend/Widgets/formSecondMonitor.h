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

private:
    Ui::FormSecondMonitor *ui;
};

#endif // FORMSECONDMONITOR_H
