#include "dialogfactory.h"

#include "dialogcaseinformation.h"
#include "dialogdeviceselect.h"
#include "mykeyboard.h"

DialogFactory::DialogFactory()
{

}

QDialog *DialogFactory::createDialog(const QString &name, QWidget *parent, int y)
{
    QDialog* dialog{nullptr};

    if(name == "caseInformationDialog"){
        dialog = new DialogCaseInformation(parent);
    }
    if(name == "deviceSelectDialog"){
        dialog = new DialogDeviceSelect(parent);
    }
    if(name == "keyboard"){
        dialog = new MyKeyboard(parent);
        dialog->move(parent->x()-100, parent->y() + y);
    }
    return dialog;
}
