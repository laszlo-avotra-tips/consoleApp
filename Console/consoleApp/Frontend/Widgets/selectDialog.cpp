#include "selectDialog.h"
#include "ui_selectDialog.h"
#include "physicianNameModel.h"


SelectDialog::SelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    m_selectableWidgets =
    {
        ui->lineEditItem1,
        ui->lineEditItem2,
        ui->lineEditItem3,
    };

    connect(ui->pushButtonOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButtonAddNew, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_selectableWidgets[0], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem0);
    connect(m_selectableWidgets[1], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem1);
    connect(m_selectableWidgets[2], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem2);
}

SelectDialog::~SelectDialog()
{
    delete ui;
}

void SelectDialog::update(const QStringList &sl)
{
    if(sl.size() > 3){
        ui->pushButtonScrollDown->show();
    } else {
        ui->pushButtonScrollDown->hide();
    }

    int index{0};
    for(auto* lineEdit : m_selectableWidgets){
        if(sl.size()>index){
            lineEdit->setText(sl[index]);
        }
        ++index;
    }
}

void SelectDialog::selectItem0()
{
    selectItem(0);
}

void SelectDialog::selectItem1()
{
    selectItem(1);
}

void SelectDialog::selectItem2()
{
    selectItem(2);
}

void SelectDialog::selectItem(int index)
{
    auto name = m_selectableWidgets[index]->text();
    PhysicianNameModel::instance()->setSelectedPysicianName(name);

    for(int i = 0; i < 3; ++i ){
        if(i == index){
            m_selectableWidgets[i]->setStyleSheet("color:orange");
        } else {
            m_selectableWidgets[i]->setStyleSheet("color:white");
        }
    }
//    accept();
    QTimer::singleShot(500,this,&SelectDialog::accept);
}

