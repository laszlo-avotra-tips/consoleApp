#include "dialogFactory.h"

#include "caseInformationDialog.h"
#include "deviceSelectDialog.h"
#include "emptyDialog.h"
#include "reviewAndSettingsDialog.h"
#include "displayOptionsDialog.h"

DialogFactory::DialogFactory()
{

}

QDialog *DialogFactory::createDialog(const QString &name, QWidget *parent, const std::vector<QString> *param)
{
    QDialog* dialog{nullptr};

    if(name == "caseInformationDialog"){
        dialog = new CaseInformationDialog(parent, param);
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
    if(name == "displayOptionsDialog"){
        dialog = new DisplayOptionsDialog(parent);
    }

    return dialog;
}
