#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include "octFile.h"

#include <vector>
#include <map>
#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>

class liveScene;
class OpaqueScreen;
class QPushButton;
class QGraphicsView;
class ScanConversion;


QT_BEGIN_NAMESPACE
namespace Ui { class MainScreen; }
QT_END_NAMESPACE

class MainScreen : public QWidget
{
    Q_OBJECT

public:
    MainScreen(QWidget *parent = nullptr);
    ~MainScreen();

    bool isVisible() const;
    void setScene(liveScene* scene);

    void setDeviceLabel();
    void showSpeed(bool isShown);

signals:
    void captureImage();
    void measureImage(bool isMeasureMode);
    void sledRunningStateChanged(bool isInRunningState);

private slots:
    void on_pushButtonFlip_clicked();

    void on_pushButtonMenu_clicked();

    int getSceneWidth();

    QSize getSceneSize();

    void on_pushButtonEndCase_clicked();

    void on_pushButtonDownArrow_clicked();
    void on_pushButtonCondensUp_clicked();

    void on_pushButtonSettings_clicked();
    void openCaseInformationDialog();
    void openCaseInformationDialogFromReviewAndSettings();
    void openDeviceSelectDialog();
    void openDeviceSelectDialogFromReviewAndSettings();
    void openCaseReview();

    void openDisplayOptionsDialog();

    void updateTime();
    void udpateToSpeed1();
    void udpateToSpeed2();
    void udpateToSpeed3();
    void resetYellowBorder();

    void on_pushButtonCapture_released();

    void on_pushButtonMeasure_clicked(bool checked);

    void updateSledRunningState();
    void handleSledRunningStateChanged(bool isInRunningState);

    void on_pushButtonRecord_clicked();
    void onCaptureImage();
    void setMeasurementMode(bool enabled);

public slots:
    void updateSector(OCTFile::OctData_t* frameData);

private:
    void showEvent(QShowEvent* se) override;
    void hideEvent(QHideEvent* he) override;
    void flipColumns();
    void toggleNavigationButtons(const std::vector<QWidget*>& buttons);
    void setCurrentTime();
    void setSpeed(int speed);
    void highlightSpeedButton(QPushButton* wid);
    int getSledRuntime();
    void setSceneCursor( QCursor cursor );

private:
    Ui::MainScreen *ui;
    QGraphicsView* m_graphicsView{nullptr};
    liveScene* m_scene{nullptr};

    const std::pair<int,int> m_widthHeight{3,2};
    const int m_sceneWidth{2160};
    const QSize m_sceneSize{2160,2160};

    std::vector<QWidget*> m_navigationButtons;

    QTime m_currentTime;
    QElapsedTimer m_runTime;
    QTimer m_updatetimeTimer;
    OpaqueScreen* m_opacScreen{nullptr};
    bool m_sledIsInRunningState{false};
    int m_sledRuntime{0}; //the time the Sled is on in milliseconds

    ScanConversion *m_scanWorker{nullptr};

};
#endif // MAINSCREEN_H
