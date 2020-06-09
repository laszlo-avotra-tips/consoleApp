#ifndef OCTKEYBOARD_H
#define OCTKEYBOARD_H

#include <QDialog>
#include <QString>
#include <vector>
#include "Frontend/Utility/widgetcontainer.h"

class QPushButton;

using ButtonContainer = std::vector<QPushButton*>;

namespace Ui {
class OctKeyboard;
}

class OctKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit OctKeyboard(const ParameterType& param, QWidget *parent = nullptr);
    ~OctKeyboard();
    QString value();

signals:
    void letterClicked(const QString &text);
    void numberClicked(const QString &text);

private slots:
    void handleDelete();
    void handleSpace();
    void handleNumbers(const QString& text);
    void handleLetters(const QString& text);
    void handleCapsLock(bool checked);

    void on_pushButton_shiftLeft_clicked();

    void on_pushButton_shiftRight_clicked();

private:
    void initButtonContainers();
    void initNumbers();
    void initLetters();
    void toLowCap();
    void toHighCap();
    void toggleCap();
    void pushButtonEnabled(QPushButton* button);
    void pushButtonDisabled(QPushButton* button);

private:
    Ui::OctKeyboard *ui;

    ButtonContainer m_letterButtons;
    ButtonContainer m_numberButtons;

    bool m_isLowCap{true};
    bool m_isShift{false};
};

#endif // KEYBOARD_H
