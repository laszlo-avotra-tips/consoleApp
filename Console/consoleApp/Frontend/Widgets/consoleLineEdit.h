#ifndef CONSOLELINEEDIT_H
#define CONSOLELINEEDIT_H

#include <QObject>
#include <QLineEdit>

/*!
    \class ConsoleLineEdit consoleLineEdit.h
    \brief The sole purpose of the ConsoleLineEdit class is to capture the mouse press event while the
    mouse is on the widget. It generates a mousePressed signal if the left button was pressed
*/
class ConsoleLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    /*!
     * \brief ConsoleLineEdit
     * \param parent
     */
    ConsoleLineEdit(QWidget* parent = nullptr);

    /*!
     * \brief mousePressEvent
     * \param e - pointer to the mouse event
     */
    void mousePressEvent(QMouseEvent * e) override;

signals:
    /*!
     * \brief mousePressed
     */
    void mousePressed();

};

#endif // CONSOLELINEEDIT_H
