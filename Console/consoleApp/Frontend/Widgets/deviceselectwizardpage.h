/*
 * deviceselectwizardPage.h
 *
 * The deviceselectwizardpage is a customized qt wizard page allowing
 * the user to choose which device is to be used in the case. It presents
 * several presets with text/icon and a custom option.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */

#ifndef DEVICESELECTWIZARDPAGE_H
#define DEVICESELECTWIZARDPAGE_H

#include <QWizardPage>
#include <QListWidgetItem>

namespace Ui 
{
    class deviceSelectWizardPage;
}

class deviceSelectWizardPage : public QWizardPage 
{
    Q_OBJECT

public:
    deviceSelectWizardPage(QWidget *parent = 0);
    ~deviceSelectWizardPage();
    void init( void );
    bool isComplete() const;

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::deviceSelectWizardPage *ui;
    void populateList(void);
};

#endif // DEVICESELECTWIZARDPAGE_H
