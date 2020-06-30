#include "dialogFactory.h"

#include "caseInformationDialog.h"
#include "deviceSelectDialog.h"
#include "emptyDialog.h"
#include "reviewAndSettingsDialog.h"

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
    if(name == "emptyDialog"){
        dialog = new EmptyDialog(parent);
    }
    if(name == "reviewAndSettingsDialog"){
        dialog = new ReviewAndSettingsDialog(parent);
    }

    return dialog;
}
