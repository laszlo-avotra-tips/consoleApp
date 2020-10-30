#include "widgetcontainer.h"
#include "screenNavigator.h"
#include "dialogfactory.h"
#include "consoleKeyboard.h"
#include "logger.h"

#include <QStackedWidget>
#include <QDebug>
#include <QDialog>

WidgetContainer* WidgetContainer::m_instance{nullptr};

WidgetContainer *WidgetContainer::instance()
{
    if(!m_instance){
        m_instance = new WidgetContainer();
    }
    return m_instance;
}

bool WidgetContainer::registerWidget(const QString &name, QWidget *wid)
{
    m_widgetContainer[name] = wid;
    int index = m_stackedWidget->addWidget(wid);

    return index >= 0;
}

bool WidgetContainer::unRegisterWidget(const QString &name)
{
    bool success{false};
    auto nameWidgetPair = m_widgetContainer.find(name);
    QWidget* wid{nullptr};

    if(nameWidgetPair != m_widgetContainer.end()){
        wid = nameWidgetPair->second;
        if(wid){
            m_stackedWidget->removeWidget(wid);
            delete wid;
            m_widgetContainer.erase(nameWidgetPair);
            success = true;
        }
    }
    return success;
}

void WidgetContainer::setStackedWidget(QStackedWidget *sw)
{
    while(sw->count()){
        sw->removeWidget(sw->widget(0));
    }

    m_stackedWidget = sw;
}

bool WidgetContainer::gotoScreen(const QString &name)
{
    bool success{false};
    auto it = m_widgetContainer.find(name);
    if(it != m_widgetContainer.end()){
        if(m_currentWidget){
            m_currentWidget->hide();
        }
        m_currentWidget = it->second;
        m_stackedWidget->setCurrentWidget(m_currentWidget);
        m_currentWidget->show();
    }

    return success;
}

QWidget *WidgetContainer::getScreen(const QString &name)
{
    QWidget* retVal{nullptr};

    auto it = m_widgetContainer.find(name);
    if(it != m_widgetContainer.end()){
        retVal = it->second;
    }
    return retVal;
}

QDialog *WidgetContainer::getDialog(const QString &name, QWidget* parent, const std::vector<QString> *param)
{
    LOG2(name,parent)
    return m_dialogFactory.createDialog(name,parent,param);
}

std::pair<QDialog*, int> WidgetContainer::openDialog(QWidget *parent, const QString &name, const std::vector<QString> *param)
{
    int result{-1};
    QDialog* dialog = getDialog(name,parent,param);

    if(dialog){
//        dialog->hide();
//        dialog->setModal(true);
//        LOG1(dialog->isModal());
        result = dialog->exec();
    }
    return std::pair<QDialog*,int>{dialog, result};
}

QString WidgetContainer::openKeyboard(QWidget *parent, const ParameterType& param, int yOffset)
{
    QString retVal;
    ConsoleKeyboard* okb = new ConsoleKeyboard(param, parent);
    auto pw = parent->width();
    auto dw = okb->width();
    int x = parent->x() + pw/2 - dw/2;

    okb->move(x, parent->y() + yOffset);
    okb->show();

    if(okb->exec() == QDialog::Accepted){
        retVal = okb->value();
    } else {
        retVal = param[1];
    }

    return retVal;
}

void WidgetContainer::close()
{
    m_navigator->close();
}

void WidgetContainer::setNavigator(ScreenNavigator *n)
{
    m_navigator = n;
}

bool WidgetContainer::getIsNewCase() const
{
    return m_isNewCase;
}

void WidgetContainer::setIsNewCase(bool isNewCase)
{
    m_isNewCase = isNewCase;
}

void WidgetContainer::minimize()
{
    if(m_navigator){
        m_navigator->minimize();
    }
}

int WidgetContainer::ratio() const
{
    return m_ratio;
}

void WidgetContainer::setRatio(int ratio)
{
    m_ratio = ratio;
}

int WidgetContainer::middleFrameWidth() const
{
    return m_middleFrameSize.width();
}

const QSize &WidgetContainer::middleFrameSize() const
{
    return m_middleFrameSize;
}

void WidgetContainer::setMiddleFrameSize(int middleFrameSize)
{
    m_middleFrameSize = QSize(middleFrameSize, middleFrameSize);
}

bool WidgetContainer::isFullScreen() const
{
    return m_isFullScreen;
}

void WidgetContainer::setIsFullScreen(bool isFullScreen)
{
    m_isFullScreen = isFullScreen;
}
