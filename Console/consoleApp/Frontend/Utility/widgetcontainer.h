#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include <vector>
#include <map>
#include <QString>
#include <QSize>

#include "dialogFactory.h"

class QStackedWidget;
class ScreenNavigator;
class QDialog;
class QLineEdit;

using ParameterType = std::vector<QString>;

class WidgetContainer
{
public:
    static WidgetContainer* instance();
    bool registerWidget(const QString& name, QWidget* wid);
    bool unRegisterWidget(const QString& name);

    void setStackedWidget(QStackedWidget* sw);
    bool gotoScreen(const QString& name);
    QWidget* getScreen(const QString& name);
    QDialog* getDialog(const QString& name, QWidget* parent, const std::vector<QString>* param = 0);
    std::pair<QDialog *, int> openDialog(QWidget* parent, const QString& name, const std::vector<QString>* param = 0);
    QString openKeyboard(QWidget* parent, const ParameterType& param, int yOffset = 0);
    void close();
    void setNavigator(ScreenNavigator* n);

    bool isFullScreen() const;
    void setIsFullScreen(bool isFullScreen);


    int middleFrameWidth() const;
    const QSize& middleFrameSize() const;
    void setMiddleFrameSize(int middleFrameWidth);

    int ratio() const;
    void setRatio(int ratio);

    bool getIsNewCase() const;
    void setIsNewCase(bool isNewCase);

    void minimize();

private:
    static WidgetContainer* m_instance;
    WidgetContainer() = default;

    DialogFactory m_dialogFactory;
    QStackedWidget* m_stackedWidget{nullptr};
    ScreenNavigator* m_navigator{nullptr};

    std::map<QString, QWidget*> m_widgetContainer;
    std::map<QString, QDialog*> m_dialogContainer;

    bool m_isFullScreen{true};
    QWidget* m_currentWidget{nullptr};
    QSize  m_middleFrameSize{1800,1800};
    int m_ratio{1};
    bool m_isNewCase{true};
};

#endif // WIDGETCONTAINER_H
