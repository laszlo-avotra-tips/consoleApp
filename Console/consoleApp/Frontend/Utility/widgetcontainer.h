#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include <map>
#include <QString>
#include <QSize>

#include "dialogfactory.h"

class QStackedWidget;
class FormNavigator;
class WidgetContainer
{
public:
    static WidgetContainer* instance();
    bool registerWidget(const QString& name, QWidget* wid);
    void setStackedWidget(QStackedWidget* sw);

    QWidget* gotoPage(const QString& name);
    QWidget* getPage(const QString& name);
    QDialog* getDialog(const QString& name, QWidget* parent, int y);
    std::pair<QDialog *, int> openDialog(QWidget* parent, const QString& name, int y = 0);

    void close();
    void setNavigator(FormNavigator* n);

    bool isFullScreen() const;
    void setIsFullScreen(bool isFullScreen);


    int middleFrameWidth() const;
    const QSize& middleFrameSize() const;
    void setMiddleFrameSize(int middleFrameWidth);

    int ratio() const;
    void setRatio(int ratio);

    bool getIsNewCase() const;
    void setIsNewCase(bool isNewCase);

private:
    WidgetContainer();

private:
    static WidgetContainer* m_instance;

    DialogFactory m_dialogFactory;
    QStackedWidget* m_stackedWidget{nullptr};
    FormNavigator* m_navigator{nullptr};

    std::map<QString, QWidget*> m_container;
    bool m_isFullScreen{true};
    QSize  m_middleFrameSize{2110,2110};
    int m_ratio{1};
    bool m_isNewCase{true};
};

#endif // WIDGETCONTAINER_H
