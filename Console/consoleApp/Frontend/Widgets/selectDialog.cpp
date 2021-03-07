#include "selectDialog.h"
#include "ui_selectDialog.h"
#include <logger.h>
#include <caseInformationModel.h>
#include <iterator>
#include "Utility/widgetcontainer.h"

SelectDialog::SelectDialog(const QString& name, CaseInformationDialog *parent) :
    QDialog(parent),
    m_name(name),
    ui(new Ui::SelectDialog)
{
    m_parent = parent;
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    m_selectableWidgets =
    {
        ui->lineEditItem1,
        ui->lineEditItem2,
        ui->lineEditItem3,
    };

    connect(ui->pushButtonAddNew, &QPushButton::clicked, this, &SelectDialog::addNew);
    connect(ui->pushButtonScrollDown, &QPushButton::clicked, this, &SelectDialog::scrollDown);

    connect(m_selectableWidgets[0], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem0);
    connect(m_selectableWidgets[1], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem1);
    connect(m_selectableWidgets[2], &ConsoleLineEdit::mousePressed, this, &SelectDialog::selectItem2);
}

SelectDialog::~SelectDialog()
{
    delete ui;
}

void SelectDialog::initializeSelect(const PhysicianNameContainer &sl, QString selected)
{
    m_items = sl;
    m_selectedItem = selected;
    if(sl.size() > 3){
        ui->pushButtonScrollDown->show();
    } else {
        ui->pushButtonScrollDown->hide();
    }
    LOG1(selected);
    populateItemsInview(m_selectedItem);
    highlight(m_selectedItem);
}

void SelectDialog::populateItemsInview(const QString &selected)
{
    auto selectedIt = m_items.find(selected);

    auto nameIt = selectedIt;
    if(selectedIt == m_items.end()){
        nameIt = m_items.begin();
    }

    m_itemsInView.clear();
    for(int i = 0; i < 3; ++i){
        if(nameIt != m_items.end()){
            const auto& name = *nameIt;
            LOG1(name);
            m_itemsInView.push_back(name);
        }
        incrementCircular(m_items,nameIt);
    }
}



void SelectDialog::highlight(const QString &selected)
{
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
    const QString nextName = m_itemsInView[1];
    populateItemsInview(nextName);
    highlight(m_selectedItem);
}

void SelectDialog::addNew()
{
    /*
     * handle "ADD NEW" physician name
     */
    QString paramName;
    QString paramValue("");
    int keyboardY;

    if(m_name == "physician"){
        paramName = m_parent->getPhysicianName();
        keyboardY = 200;
    } else {
        keyboardY = 0;
        paramName = m_parent->getLocation();
    }
    const ParameterType param{paramName, paramValue, "ADD NEW"};

    /*
     * create the modal keyboard instance for physician name
     */
    auto newName = WidgetContainer::instance()->openKeyboard(this, param, keyboardY);

    /*
     * code execution continues here once the keyboard is closed
     * add newName
     * update selected physician name with newName
     */
    auto model = *CaseInformationModel::instance();
    if(m_name == QString("physician")){
        model.addPhysicianName(newName);
        model.setSelectedPhysicianName(newName);
        model.persistModel();
        initializeSelect(model.physicianNames(),newName);
    } else {
        model.addLocation(newName);
        model.setSelectedLocation(newName);
        model.persistModel();
        initializeSelect(model.locations(),newName);
    }
}

void SelectDialog::closeDialog(bool isChecked)
{
    if(!isChecked){
        LOG1(isChecked);
        emit rejected();
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

int SelectDialog::indexOf(const PhysicianNameContainer &cont, const QString &val) const
{
    const auto it = std::find(cont.begin(), cont.end(), val);

    auto base = cont.begin();

    int index{0};
    while( base != it){
        ++base;
        ++index;
    }

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

