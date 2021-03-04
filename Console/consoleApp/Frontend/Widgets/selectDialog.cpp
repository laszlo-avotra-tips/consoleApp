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
    LOG1(selected);
    auto selectedIt = m_items.find(selected);
    PhysicianNameContainer::iterator nameIt = selectedIt;
    if(selectedIt != m_items.end()){
        decrementCircular(sl, nameIt);
    } else {
        nameIt = m_items.begin();
    }
    m_itemsInView.clear();
    for(int i = 0; i < 3; ++i){
       if(nameIt != m_items.end()){
           const auto& name = *nameIt++;
           LOG1(name);
           m_itemsInView.push_back(name);
       } else {
           nameIt = m_items.begin();
           const auto& name = *nameIt++;
           LOG1(name);
           m_itemsInView.push_back(name);
       }
    }
    auto itemIt = m_itemsInView.begin();
    for(auto* lineEdit : m_selectableWidgets){
        if(itemIt != m_itemsInView.end()){
            const auto& name = *itemIt++;
            LOG1(name);
            lineEdit->setText(name);
            if(name == selected){
                lineEdit->mark();
            }else {
                lineEdit->unmark();
            }
        }
    }

//    if(!m_selectedItem.isEmpty()){
//        m_itemsInView.clear();
//        int si = indexOf(m_items,m_selectedItem);
//        LOG2(si, m_selectedItem)
//        if(si >= 0){
//            if(si < 3){
//                m_itemsInView.insert(m_items[0]);
//                m_itemsInView.insert(m_items[1]);
//                m_itemsInView.insert(m_items[2]);
//                auto* wid = m_selectableWidgets[si];
//                auto style = wid->styleSheet();
//                wid->setStyleSheet(style + QString("color:#F5C400;"));
//            } else {
//                m_itemsInView.insert(m_items[si - 2]);
//                m_itemsInView.insert(m_items[si - 1]);
//                m_itemsInView.insert(m_items[si]);
//                auto* wid = m_selectableWidgets[2];
//                auto style = wid->styleSheet();
//                wid->setStyleSheet(style + QString("color:#F5C400;"));
//            }
//            int index{0};
//            for(auto* lineEdit : m_selectableWidgets){
//                if(m_items.size()>index){
//                    lineEdit->setText(m_itemsInView[index]);
//                }
//                ++index;
//            }
//        }
//    } else {
//        int index{0};
//        for(auto* lineEdit : m_selectableWidgets){
//            if(m_items.size()>index){
//                lineEdit->setText(m_items[index]);
//            }
//            ++index;
//        }
//    }
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
//    if(m_itemsInView.size() == 3){
//        auto lastInView = m_itemsInView[2];
//        int indexOfLastInView = indexOf(m_items,lastInView);

//        int maxIndexInView = m_items.size() - 1;
//        if(indexOfLastInView == maxIndexInView){
//            m_itemsInView[0] = m_items[maxIndexInView - 1];
//            m_itemsInView[1] = m_items[maxIndexInView];
//            m_itemsInView[2] = m_items[0];
//        }else if (indexOfLastInView > 0){
//            m_itemsInView[0] = m_items[indexOfLastInView - 1];
//            m_itemsInView[1] = m_items[indexOfLastInView];
//            m_itemsInView[2] = m_items[indexOfLastInView + 1];
//        } else if(indexOfLastInView == 0){
//            m_itemsInView[0] = m_items[maxIndexInView];
//            m_itemsInView[1] = m_items[0];
//            m_itemsInView[2] = m_items[1];
//        }

//        size_t index{0};
//        for(auto* lineEdit : m_selectableWidgets){
//            if(m_items.size()>index){
//                auto style = lineEdit->styleSheet();
//                lineEdit->setText(m_itemsInView[index]);
//                lineEdit->setStyleSheet(style + QString("color:white"));
//            }
//            ++index;
//        }
//        int highlighted = indexOf(m_itemsInView,m_selectedItem);
//        LOG2(m_selectedItem, highlighted)
//        if(highlighted >= 0 && highlighted < 3){
//            auto* wid = m_selectableWidgets[highlighted];
//            auto style = wid->styleSheet();
//            wid->setStyleSheet(style + QString("color:#F5C400;"));
//        }
//    }
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

int SelectDialog::indexOf(const PhysicianNameContainer &cont, const QString &val) const
{
    const auto it = std::find(cont.begin(), cont.end(), val);

    auto base = cont.begin();

    int index{0};
    while( base != it){
        ++base;
        ++index;
    }

    //int index = it - cont.begin();

    return index;
}

void SelectDialog::incrementCircular(const PhysicianNameContainer &cont, PhysicianNameContainer::iterator &it)
{
    if(it != cont.end()){
        auto tempIt = it;
        if(++tempIt == cont.end()){
            it = cont.begin();
        } else {
            ++it;
        }
    }
}

void SelectDialog::decrementCircular(const PhysicianNameContainer &cont, PhysicianNameContainer::iterator &it)
{
    if(it != cont.end()){
        if(it == cont.begin()){
            auto tempIt = cont.end();
            it = --tempIt;
        }else{
            --it;
        }
    }
}

QString SelectDialog::selectedItem() const
{
    return m_selectedItem;
}

