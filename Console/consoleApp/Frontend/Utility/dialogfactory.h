#ifndef DIALOGFACTORY_H
#define DIALOGFACTORY_H

class QWidget;
class QDialog;
class QString;


/*!
    \class DialogFactory dialogFactory.h
    \brief The DialogFactory class instanciates dialogs by name
*/
class DialogFactory
{
public:
    /*!
     * \brief constructor
     */
    DialogFactory();


    /*!
     * \brief createDialog
     * \param name  - the name of the dialog
     * \param parent - the parent controls the instances lifetime
     * \return the created dialog
     */
    QDialog* createDialog(const QString& name, QWidget* parent);
};

#endif // DIALOGFACTORY_H
