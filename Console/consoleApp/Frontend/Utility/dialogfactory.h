#ifndef DIALOGFACTORY_H
#define DIALOGFACTORY_H

#include <vector>

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
    QDialog* createDialog(const QString& name, QWidget* parent, const std::vector<QString> *param = nullptr);
};

#endif // DIALOGFACTORY_H
