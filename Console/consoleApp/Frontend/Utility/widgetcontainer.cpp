#include "widgetcontainer.h"
#include "formnavigator.h"
#include "dialogfactory.h"

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
    m_container[name] = wid;
    int index = m_stackedWidget->addWidget(wid);
//    wid->setStyleSheet(m_stackedWidget->styleSheet());

    return index >= 0;
}

void WidgetContainer::setStackedWidget(QStackedWidget *sw)
{
    while(sw->count()){
        sw->removeWidget(sw->widget(0));
    }

    m_stackedWidget = sw;
}

QWidget* WidgetContainer::gotoPage(const QString &name)
{
    QWidget* widget{getPage(name)};
//    auto it = m_container.find(name);
//    if(it != m_container.end()){
//        widget = it->second;
//        m_stackedWidget->setCurrentWidget(widget);
//    }
    if(widget){
        m_stackedWidget->setCurrentWidget(widget);
    }

    return widget;
}

QWidget *WidgetContainer::getPage(const QString &name)
{
    QWidget* widget{nullptr};
    auto it = m_container.find(name);
    if(it != m_container.end()){
        widget = it->second;
    }
    return widget;
}

void WidgetContainer::close()
{
    m_navigator->close();
}

void WidgetContainer::setNavigator(FormNavigator *n)
{
    m_navigator = n;
}

WidgetContainer::WidgetContainer()
{

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

std::pair<QDialog*, int> WidgetContainer::openDialog(QWidget *parent, const QString &name, int y)
{
    int result{-1};
    QDialog* dialog = getDialog(name,parent,y);
    if(dialog){
        qDebug() << "X = " << dialog->x() << "Y = " << dialog->y();
        qDebug() << "parent.X = " << parent->x() << "parent.Y = " << parent->y();

        dialog->show();
        result = dialog->exec();
    }
    return std::pair<QDialog*,int>{dialog, result};
}

QDialog *WidgetContainer::getDialog(const QString &name, QWidget* parent, int y)
{
    return m_dialogFactory.createDialog(name,parent,y);
}
