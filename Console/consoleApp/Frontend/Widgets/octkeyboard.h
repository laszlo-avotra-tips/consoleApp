#ifndef OCTKEYBOARD_H
#define OCTKEYBOARD_H

#include <QDialog>
#include <QString>
#include <vector>

using std::vector;
class QPushButton;

using ButtonContainer = std::vector<QPushButton*>;

namespace Ui {
class OctKeyboard;
}

class OctKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit OctKeyboard(const vector<QString>& param, QWidget *parent = nullptr);
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

private:
    void initButtonContainers();
    void initNumbers();
    void initLetters();

private:
    Ui::OctKeyboard *ui;

    ButtonContainer m_letterButtons;
    ButtonContainer m_numberButtons;
};

#endif // KEYBOARD_H
