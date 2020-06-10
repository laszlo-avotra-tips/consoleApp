#include "dialogFactory.h"

#include "caseInformationDialog.h"
#include "deviceSelectDialog.h"

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
        dialog = new DeviceSelectDialog(parent);
    }
    return dialog;
}
