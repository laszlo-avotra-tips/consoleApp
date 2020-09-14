#ifndef DISPLAYOPTIONSDIALOG_H
#define DISPLAYOPTIONSDIALOG_H

#include <QDialog>
#include <map>
#include <vector>

#include "DisplayOptionsModel.h"

class liveScene;
class QGraphicsView;

namespace Ui {
class DisplayOptionsDialog;
}

class DisplayOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DisplayOptionsDialog(QWidget *parent = nullptr);
    ~DisplayOptionsDialog();
    void setScene(liveScene* scene);
    void setModel(DisplayOptionsModel* model);

signals:
    void setColorModeSepia();
    void setColorModeGray();
    void reticleBrightnessChanged();

private slots:
    void on_pushButtonDone_clicked();

    void on_pushButtonBack_clicked();

    void on_pushButtonDepthMimus_clicked();

    void on_pushButtonDepthPlus_clicked();

    void setImagingDepth(int depth);

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSliderRingBrightness_valueChanged(int reticleBrightness);

    void handleUp();
    void handleDown();
    void updateUpDownButtonColor();
    void updateDistalToProximalSetting(bool isUp);
    void handleGray();
    void handleSepia();
    void updateGraySepiaButtonColor();
    void updateGraySepiaSetting();
    void handleImageBrightness(int value);
    void handleImageContrast(int value);


private:
    void initBrightnessAndContrast();
    void initUpDown();
    void initSepiaGray();
    void initImagingDepth();

    Ui::DisplayOptionsDialog *ui;
    QGraphicsView* m_graphicsView{nullptr};
    liveScene* m_scene{nullptr};
    const std::vector<int> m_imagingDepth{0,300,375,450,525,600};
    const std::map<int,int> m_m_imagingDepthIndexLut
    {
        { 300,0},
        { 375,1},
        { 450,2},
        { 525,3},
        { 600,4}
    };
    int m_depthIndex{1};

    DisplayOptionsModel* m_model{nullptr};
    DisplayOptionsModel  m_model0;

};

#endif // DISPLAYOPTIONSDIALOG_H
