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

    /*!
     * \brief otherThanLetterOrNumberClicked
     * \param character - the character clicked
     */
    void otherThanLetterOrNumberClicked(const QString& character);

private slots:
    /*!
     * \brief handleDelete - what happens when <delete> is pressed
     */
    void handleDelete();

    /*!
     * \brief handleSpace - what happens when the spacebar is pressed
     */
    void handleSpace();

    /*!
     * \brief handleNumbersAndOthers - what happens when a number or other key is pressed
     * \param number - the number or some other key
     */
    void handleNumbersAndOthers(const QString& number);

    /*!
     * \brief handleLetters - what happens when a letter key is pressed
     * \param letter - the letter key
     */
    void handleLetters(const QString& letter);

    /*!
     * \brief handleCapsLock - what happens when a the <caps lock> key is pressed
     * \param checked - the caps lock is toggled on/off; checked/not checked
     */
    void handleCapsLock(bool checked);

    /*!
     * \brief handleLeftShift - what happens when a the left shift key is pressed
     */
    void handleLeftShift();

    /*!
     * \brief handleRightShift - what happens when a the right shift key is pressed
     */
    void handleRightShift();

private:
    /*!
     * \brief initButtonContainers -
     */
    void initButtonContainers();

    /*!
     * \brief connectNumbers - connect the m_numberButtons with signals
     */
    void connectNumbers();

    /*!
     * \brief connectLetters - connect the m_letterButtons with signals
     */
    void connectLetters();

    /*!
     * \brief connectOtherThanLetterOrNumber - connect the m_otherThanLetterOrNumberContainer with signals
     */
    void connectOtherThanLetterOrNumber();

    /*!
     * \brief toLowCap - set all letter to low cap
     */
    void toLowCap();

    /*!
     * \brief toHighCap - set all letter to high cap
     */
    void toHighCap();

    /*!
     * \brief toggleCap - toggle from lowCap to highCap and back
     */
    void toggleCap();

    /*!
     * \brief buttonToUpper - set button to upper case
     * \param button
     */
    void buttonToUpper(QPushButton* button);

    /*!
     * \brief buttonToLower - set button to upper case
     * \param button
     */
    void buttonToLower(QPushButton* button);

    /*!
     * \brief highlightEnter - highlight the enter button
     */
    void highlightEnter();

private:
    Ui::ConsoleKeyboard *ui;

    /*!
     * \brief m_letterButtons - widgets for [A..z]
     */
    ButtonContainer m_letterButtons;

    /*!
     * \brief m_otherThanLetterOrNumberContainer -  widgets for [[,.-=[]/\]
     */
    ButtonContainer m_otherThanLetterOrNumberContainer;

    /*!
     * \brief m_numberButtons -  widgets for [[0..9]
     */
    ButtonContainer m_numberButtons;

    /*!
     * \brief m_isLowCap - keyboard is low cap
     */
    bool m_isLowCap{true};

    /*!
     * \brief m_isShift - the shift key is on
     */
    bool m_isShift{false};
};

#endif // KEYBOARD_H
