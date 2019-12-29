/*
 * caseInfoWizardPage.h
 *
 * The caseinfowizardpage is a customized qt wizard page allowing
 * data entry of session related data (patient, doctor, etc.)
 *
 * Handles the validation, saving, and setup of case information within
 * the wizard.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#ifndef CASEINFOWIZARDPAGE_H
#define CASEINFOWIZARDPAGE_H

#include <QWizardPage>
#include <QTimer>
#include <QCompleter>
#include <QStringList>
#include <QSettings>
#include "caseinfowizard.h"

namespace Ui {
    class caseInfoWizardPage;
}

class caseInfoWizardPage : public QWizardPage {
    Q_OBJECT

public:
    caseInfoWizardPage(QWidget *parent = 0);
    ~caseInfoWizardPage();
    bool validatePage();
    void init( caseInfoWizard::SetupType type );

protected:
    void changeEvent(QEvent *e);

private:
    Ui::caseInfoWizardPage *ui;
    caseInfoWizard::SetupType caseSetupType;

    QSettings   *autoCompleteSettings;
    QStringList  docList;
    QCompleter  *docCompleter;
    QStringList  locationList;
    QCompleter  *locationCompleter;

    void loadAutoComplete( void );
    void saveAutoComplete( void );

private slots:
    void on_patientIdField_editingFinished();
    void on_doctorField_editingFinished();
    void on_locationField_editingFinished();
};

#endif // CASEINFOWIZARDPAGE_H
