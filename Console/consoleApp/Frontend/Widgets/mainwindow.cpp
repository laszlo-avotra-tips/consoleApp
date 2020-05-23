#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Utility/widgetcontainer.h"
#include "Utility/pagefactory.h"

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

    ui->pushButtonFlip->hide();

    ui->frameButtons->hide();
    const int h{getSceneWidth()};
    double dw = h * 1.5;
    const int w{ int(dw) };

    const QSize sizeMiddle{h,h};
    const QSize sizeSide{(w-h)/2,h};

    qDebug() << "w = " << w << ", h = " << h << ", sizeSide = " << sizeSide;

    ui->frameM->setMaximumSize(sizeMiddle);
    ui->frameM->setMinimumSize(sizeMiddle);

    ui->frameL->setMaximumSize(sizeSide);
    ui->frameL->setMinimumSize(sizeSide);
    ui->frameR->setMaximumSize(sizeSide);
    ui->frameR->setMinimumSize(sizeSide);

//    auto* buttonLayout = ui->frameButtons->layout();
//    const int bc = buttonLayout->count();
//    if(bc){
//        buttonLayout->setSpacing(h/bc);
//    }
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
    if(!ui->frameButtons->isVisible()){
        ui->frameButtons->show();
        ui->pushButtonFlip->show();
//        ui->pushButtonMenu->setText("Hide Menu");
    } else {
        ui->frameButtons->hide();
        ui->pushButtonFlip->hide();
//        ui->pushButtonMenu->setText("Show Menu");
    }
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

void MainWindow::on_pushButtonPage1_clicked()
{

}

void MainWindow::on_pushButtonPage2_clicked()
{
}

void MainWindow::on_pushButtonExitL_clicked()
{
    WidgetContainer::instance()->gotoPage("startPage");
}

int MainWindow::getSceneWidth()
{
    int retVal = m_sceneWidth;
    QString fn("/Avinger_System/screen.dat");
    QFile sf(fn);
    if(sf.open(QIODevice::ReadOnly)){
        QTextStream ts(&sf);
        int width;
        int isFullScreen;
        ts >> width >> isFullScreen;
        qDebug() << fn << " open ok. width = " << width << ", isFullScreen " << isFullScreen;
        WidgetContainer::instance()->setIsFullScreen(isFullScreen);
        WidgetContainer::instance()->setCenterFrameSize(width);
        retVal = width;
        sf.close();
    }

    return retVal;
}
