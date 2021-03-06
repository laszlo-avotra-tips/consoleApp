#include "selectDialog.h"
#include "ui_selectDialog.h"
#include <logger.h>
#include <caseInformationModel.h>
#include <iterator>
#include "Utility/widgetcontainer.h"
#include "Utility/preferencesModel.h"

SelectDialog::SelectDialog(const QString& name, CaseInformationDialog *parent) :
    QDialog(parent),
    m_name(name),
    ui(new Ui::SelectDialog)
{
    m_parent = parent;
    m_pModel = PreferencesModel::instance();
    m_pModel->loadPreferences();
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
    if(!selected.isEmpty()){
        m_selectedItem = selected;
    } else if(!sl.empty()){
        m_selectedItem = *(sl.begin());
    }
    if(sl.size() > m_selectableCount){
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

    m_itemsInView.clear();
    if(m_items.size() > m_selectableCount){
        auto nameIt = selectedIt;
        for(size_t i = 0; i < m_selectableCount; ++i){
            if(nameIt != m_items.end()){
                const auto& name = *nameIt;
                incrementCircular(m_items,nameIt);
                LOG1(name);
                m_itemsInView.push_back(name);
            }
        }
    } else {
        for(const auto& item : m_items){
           const auto& name = item;
           LOG1(name);
           m_itemsInView.push_back(name);
        }
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
    LOGUA;
    selectItem(0);
}

void SelectDialog::selectItem1()
{
    LOGUA;
    selectItem(1);
}

void SelectDialog::selectItem2()
{
    LOGUA;
    selectItem(2);
}

void SelectDialog::scrollDown()
{
    LOGUA;
    const QString nextName = m_itemsInView[1];
    populateItemsInview(nextName);
    highlight(m_selectedItem);
}

void SelectDialog::addNew()
{
    LOGUA;
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
        m_pModel->addPhysician(newName);
        model.setSelectedPhysicianName(newName);
        m_pModel->persistPreferences();
        initializeSelect(m_pModel->physicians(),newName);
    } else {
        m_pModel->addLocation(newName);
        model.setSelectedLocation(newName);
        m_pModel->persistPreferences();
        initializeSelect(m_pModel->locations(),newName);
    }
    accept();
}

void SelectDialog::closeDialog(bool isChecked)
{
    if(!isChecked){
        LOG1(isChecked);
        emit rejected();
    }
}

//border-top: 2px solid rgb( 169, 169, 169);
void SelectDialog::selectItem(size_t index)
{
    m_selectedItem = m_selectableWidgets[index]->text();

    for(size_t i = 0; i < m_selectableCount; ++i ){
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

