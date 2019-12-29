/*
 * caseInfoWizard.h
 *
 * The caseinfowizard is a customized qt wizard allowing
 * data entry of session related data (patient, doctor, etc.)
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#ifndef CASEINFOWIZARD_H
#define CASEINFOWIZARD_H

#include <QWizard>

namespace Ui {
    class caseInfoWizard;
}

class caseInfoWizard : public QWizard {
    Q_OBJECT

public:
    enum SetupType
    {
        InitialCaseSetup,
        UpdateCaseSetup
    };

    caseInfoWizard(QWidget *parent = 0);
    ~caseInfoWizard();
    void init( SetupType type );

protected:
    void changeEvent(QEvent *e);

private:
    Ui::caseInfoWizard *ui;
};

#endif // CASEINFOWIZARD_H
