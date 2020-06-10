#include "dialogFactory.h"

#include "caseInformationDialog.h"
#include "dialogdeviceselect.h"

DialogFactory::DialogFactory()
{

}

QDialog *DialogFactory::createDialog(const QString &name, QWidget *parent)
{
    QDialog* dialog{nullptr};

    if(name == "caseInformationDialog"){
        dialog = new CaseInformationDialog(parent);
    }
    if(name == "deviceSelectDialog"){
        dialog = new DialogDeviceSelect(parent);
    }
    return dialog;
}
