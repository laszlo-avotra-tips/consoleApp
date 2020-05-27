#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Utility/widgetcontainer.h"
#include "Utility/pagefactory.h"
#include "Frontend/Screens/frontend.h"
#include "devicewizard.h"
#include "deviceselectwizardpage.h"
#include "deviceSettings.h"


#include <QDebug>
#include <QLayoutItem>
#include <QLayout>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const int h{getSceneWidth()};
    const int ratio = WidgetContainer::instance()->ratio();
    if(!ratio){
        return;
    }
    const int height = m_sceneWidth / ratio;
    const int wL = 550 / ratio;
    const int wM = height;
    const int wR = 3240 / ratio - wM - wL;
    const QSize sL(wL,height);
    const QSize sM(wM,height);
    const QSize sR(wR,height);

    ui->frameM->setMaximumSize(sM);
    ui->frameM->setMinimumSize(sM);

    ui->frameL->setMaximumSize(sL);
    ui->frameL->setMinimumSize(sL);

    ui->frameR->setMaximumSize(sR);
    ui->frameR->setMinimumSize(sR);

    m_navigationButtons.push_back(ui->pushButtonEndCase);
    m_navigationButtons.push_back(ui->pushButtonSettings);
    m_navigationButtons.push_back(ui->pushButtonMeasure);
    m_navigationButtons.push_back(ui->pushButtonRecord);
    m_navigationButtons.push_back(ui->pushButtonCapture);
    m_navigationButtons.push_back(ui->pushButtonFlip);
    m_navigationButtons.push_back(ui->pushButtonDeviceSelect);

    for(auto* button : m_navigationButtons){
        button->hide();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButtonFlip_clicked()
{
    flipColumns();
}


void MainWindow::on_pushButtonMenu_clicked()
{
    toggleNavigationButtons(m_navigationButtons);
}

void MainWindow::flipColumns()
{
    QLayout* tl = layout();
    std::vector<QLayoutItem*> current{tl->itemAt(0),tl->itemAt(1),tl->itemAt(2)};

    tl->removeItem(current[2]);
    tl->removeItem(current[1]);
    tl->removeItem(current[0]);
    tl->addItem(current[2]);
    tl->addItem(current[1]);
    tl->addItem(current[0]);
    tl->update();
}

void MainWindow::toggleNavigationButtons(const std::vector<QWidget *> &buttons)
{
    if(buttons[0]->isVisible()){
        for(auto* button : buttons){
            button->hide();
        }
    }else {
        for(auto* button : buttons){
            button->show();
        }
    }
}


int MainWindow::getSceneWidth()
{
    int retVal = m_sceneWidth;
    QString fn("/Avinger_System/screen.dat");
    QFile sf(fn);
    if(sf.open(QIODevice::ReadOnly)){
        QTextStream ts(&sf);
        int ratio{1};
        int isFullScreen;
        ts >> ratio >> isFullScreen;
        if(ratio){
            int width = m_sceneWidth / ratio;
            qDebug() << fn << " open ok. width = " << width << ", isFullScreen " << isFullScreen;
            WidgetContainer::instance()->setIsFullScreen(isFullScreen);
            WidgetContainer::instance()->setMiddleFrameSize(width);
            WidgetContainer::instance()->setRatio(ratio);
            retVal = width;
        }
        sf.close();
    }

    return retVal;
}

QSize MainWindow::getSceneSize()
{
    return m_sceneSize;
}

void MainWindow::on_pushButtonEndCase_clicked()
{
    WidgetContainer::instance()->gotoPage("startPage");
}

void MainWindow::on_pushButtonDownArrow_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainWindow::on_pushButtonCapture_clicked()
{
}

void MainWindow::on_pushButtonDeviceSelect_clicked()
{
    deviceWizard deviceSelection(this);
    deviceSelection.exec();
}

void MainWindow::setDeviceLabel()
{
    deviceSettings &dev = deviceSettings::Instance();
    const QString name{dev.getCurrentDeviceName()};
    ui->labelDevice->setText(name);
}
