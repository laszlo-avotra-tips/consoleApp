#include "selectDialog.h"
#include "ui_selectDialog.h"
#include <logger.h>
#include <caseInformationModel.h>

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

    connect(ui->pushButtonAddNew, &QPushButton::clicked, this, &SelectDialog::reject);
    connect(ui->pushButtonScrollDown, &QPushButton::clicked, this, &SelectDialog::scrollDown);

    connect(m_selectableWidgets[0], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem0);
    connect(m_selectableWidgets[1], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem1);
    connect(m_selectableWidgets[2], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem2);
}

SelectDialog::~SelectDialog()
{
    delete ui;
}

void SelectDialog::populate(const PhysicianNameContainer &sl, const QString &selected)
{
    m_items = sl;
    m_selectedItem = selected;
    if(sl.size() > 3){
        ui->pushButtonScrollDown->show();
    } else {
        ui->pushButtonScrollDown->hide();
    }

    if(!m_selectedItem.isEmpty()){
        m_itemsInView.clear();
        auto si = m_items.indexOf(m_selectedItem);
        LOG2(si, m_selectedItem)
        if(si >= 0){
            if(si < 3){
                m_itemsInView.push_back(m_items[0]);
                m_itemsInView.push_back(m_items[1]);
                m_itemsInView.push_back(m_items[2]);
                auto* wid = m_selectableWidgets[si];
                auto style = wid->styleSheet();
                wid->setStyleSheet(style + QString("color:#F5C400;"));
            } else {
                m_itemsInView.push_back(m_items[si - 2]);
                m_itemsInView.push_back(m_items[si - 1]);
                m_itemsInView.push_back(m_items[si]);
                auto* wid = m_selectableWidgets[2];
                auto style = wid->styleSheet();
                wid->setStyleSheet(style + QString("color:#F5C400;"));
            }
            int index{0};
            for(auto* lineEdit : m_selectableWidgets){
                if(m_items.size()>index){
                    lineEdit->setText(m_itemsInView[index]);
                }
                ++index;
            }
        }
    } else {
        int index{0};
        for(auto* lineEdit : m_selectableWidgets){
            if(m_items.size()>index){
                lineEdit->setText(m_items[index]);
            }
            ++index;
        }
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

void SelectDialog::scrollDown()
{
    if(m_itemsInView.size() == 3){
        auto lastInView = m_itemsInView[2];
        auto indexOfLastInView = m_items.indexOf(lastInView);
        auto maxIndexInView = m_items.size() - 1;
        if(indexOfLastInView == maxIndexInView){
            m_itemsInView[0] = m_items[maxIndexInView - 1];
            m_itemsInView[1] = m_items[maxIndexInView];
            m_itemsInView[2] = m_items[0];
        }else if (indexOfLastInView > 0){
            m_itemsInView[0] = m_items[indexOfLastInView - 1];
            m_itemsInView[1] = m_items[indexOfLastInView];
            m_itemsInView[2] = m_items[indexOfLastInView + 1];
        } else if(indexOfLastInView == 0){
            m_itemsInView[0] = m_items[maxIndexInView];
            m_itemsInView[1] = m_items[0];
            m_itemsInView[2] = m_items[1];
        }

        int index{0};
        for(auto* lineEdit : m_selectableWidgets){
            if(m_items.size()>index){
                auto style = lineEdit->styleSheet();
                lineEdit->setText(m_itemsInView[index]);
                lineEdit->setStyleSheet(style + QString("color:white"));
            }
            ++index;
        }
        auto highlighted =   m_itemsInView.indexOf(m_selectedItem);
        LOG2(m_selectedItem, highlighted)
        if(highlighted >= 0 && highlighted < 3){
            auto* wid = m_selectableWidgets[highlighted];
            auto style = wid->styleSheet();
            wid->setStyleSheet(style + QString("color:#F5C400;"));
        }
    }
}
//border-top: 2px solid rgb( 169, 169, 169);
void SelectDialog::selectItem(int index)
{
    m_selectedItem = m_selectableWidgets[index]->text();

    for(int i = 0; i < 3; ++i ){
        auto* wid = m_selectableWidgets[i];
        auto style = wid->styleSheet();
        if(i == index){
            wid->setStyleSheet(style + QString("color:#F5C400;"));
        } else {
           wid->setStyleSheet(style + QString("color:white;"));
        }
    }
    QTimer::singleShot(500,this,&SelectDialog::accept);
}

QString SelectDialog::selectedItem() const
{
    return m_selectedItem;
}

