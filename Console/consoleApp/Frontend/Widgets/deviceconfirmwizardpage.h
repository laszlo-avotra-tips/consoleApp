/*
 * deviceConfirmWizardPage.h
 *
 * The deviceconfirmwizardpage is a customized qt wizard page allowing
 * the user to confirm their selection of imaging device.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger
 */
#ifndef DEVICECONFIRMWIZARDPAGE_H
#define DEVICECONFIRMWIZARDPAGE_H

#include <QWizardPage>

namespace Ui
{
    class deviceConfirmWizardPage;
}

class deviceConfirmWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    deviceConfirmWizardPage(QWidget *parent = nullptr);
    ~deviceConfirmWizardPage();
    bool validatePage();
    void initializePage();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::deviceConfirmWizardPage *ui;
};

#endif // DEVICECONFIRMWIZARDPAGE_H
