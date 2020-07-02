#ifndef OCTKEYBOARD_H
#define OCTKEYBOARD_H

#include <QDialog>
#include <QString>
#include <vector>
#include "Frontend/Utility/widgetcontainer.h"

class QPushButton;

/*!
 * \brief ButtonContainer
 */
using ButtonContainer = std::vector<QPushButton*>;

namespace Ui {
class ConsoleKeyboard;
}

/*!
 * \class ConsoleKeyboard consoleKeyboard.h
 * \brief The ConsoleKeyboard class
 *
 * The ConsoleKeyboard class implements a context sensitive in process touch screen keyboard
 * for data input. It's primary use is to edit or create parameter values. The calling
 * routine creates on instance of the ConsoleKeyboard for a named parameter and handles
 * the result of the editing after the keyboard was closed.
 * The ConsoleKeyboard can be instantiated with a parameter name and a parameter value.
 * The parameter value is displayed in a QLineEdit widget with a cursor. Editing of the parameter
 * value is done by moving the cursor and pressing the numbers or letters or delet or ...
 * By pressing the highlighter button ("UPDATE", "ADD", ...) the keyboard is closed and the
 * text of the QLineEdit becomes available by calling value() on the instance.
 */
class ConsoleKeyboard : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief ConsoleKeyboard
     * \param param - ParameterType provides the context for the keyboard
     * \param parent - the parent controls the instances lifetime
     */
    explicit ConsoleKeyboard(const ParameterType& param, QWidget *parent = nullptr);

    ~ConsoleKeyboard();

    /*!
     * \brief value - get the result of the keyboard edit
     * \return QString - the result of the keyboard edit
     */
    QString value();

signals:
    /*!
     * \brief letterClicked
     * \param letter - the letter clikked
     */
    void letterClicked(const QString &letter);

    /*!
     * \brief numberClicked
     * \param number - the number cliked
     */
    void numberClicked(const QString &number);

private slots:
    void handleDelete();
    void handleSpace();
    void handleNumbers(const QString& number);
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
    void highlightEnter();

private:
    Ui::ConsoleKeyboard *ui;

    ButtonContainer m_letterButtons;
    ButtonContainer m_numberButtons;

    bool m_isLowCap{true};
    bool m_isShift{false};
};

#endif // KEYBOARD_H
