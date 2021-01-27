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
    static void setSpeedAndEnableDisableBidirectional(int speed);

signals:
    void captureImage();
    void measureImage(bool isMeasureMode);
    void sledRunningStateChanged(int isInRunningState);
    void updateRecorder(uint8_t *frame, const char *catheterName, const char *cathalogName,
                    const char *activePassive, const char* timestamp, int width, int height);

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

    void openDisplayOptionsDialog();

    void updateTime();
    void udpateToSpeed1();
    void udpateToSpeed2();
    void udpateToSpeed3();
    void handleYellowBorder();

    void on_pushButtonCapture_released();

    void on_pushButtonMeasure_clicked(bool checked);

    void updateSledRunningState();
    void handleSledRunningState(int runningStateVal);

    void onCaptureImage();
    void grabImage();

    void setMeasurementMode(bool enabled);
    void enableRecordButton();

    void on_pushButton_clicked();

    void on_pushButtonRecord_clicked(bool checked);

public slots:
    void updateSector(OCTFile::OctData_t* frameData);

private:
    void showEvent(QShowEvent* se) override;
    void hideEvent(QHideEvent* he) override;
    void flipColumns();
    void toggleNavigationButtons(const std::vector<QWidget*>& buttons);
    void setCurrentTime();
    void highlightSpeedButton(QPushButton* wid);
    int getSledRuntime();
    void setSceneCursor( QCursor cursor );
    void updateDeviceSettings();
    void showYellowBorderForRecordingOn(bool recordingIsOn);
    void initRecording();

private:
    Ui::MainScreen *ui;
    QGraphicsView* m_graphicsView{nullptr};
    liveScene* m_scene{nullptr};
    QImage m_sectorImage;
    QImage m_grayImage;

    const std::pair<int,int> m_widthHeight{3,2};
    const int m_sceneWidth{2160};
    const QSize m_sceneSize{2160,2160};

    std::vector<QWidget*> m_navigationButtons;

    QTime m_currentTime;
    QElapsedTimer m_runTime;
    QTimer m_updatetimeTimer;
    OpaqueScreen* m_opacScreen{nullptr};
    bool m_sledIsInRunningState{false};
    int m_sledRunningStateVal{0};
    int m_sledRuntime{0}; //the time the Sled is on in milliseconds

    ScanConversion *m_scanWorker{nullptr};
    bool m_recordingIsOn{false};
    bool m_recordingIsInitialized{false};
    uint8_t* m_clipBuffer{nullptr};
    int m_numberOfMissedImages[2]{};
    int m_imageFrame[2]{};
    bool startCount{false};
    int m_imageDecimation{0};

};
#endif // MAINSCREEN_H
