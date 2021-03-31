/*
 * styledmessagebox.h
 *
 * styledMessageBox is a simple replacement for qt's default
 * QMessageBox in the Avinger style. This just allows us to
 * apply a custom style sheet to notification dialog boxes.
 *
 * Author: Chris White
 * Copyright (c) 2011-2018 Avinger, Inc.
 */
#ifndef STYLEDMESSAGEBOX_H
#define STYLEDMESSAGEBOX_H

#include <QDialog>
#include "backgroundmask.h"

namespace Ui {
    class styledMessageBox;
}

class styledMessageBox : public QDialog
{
    Q_OBJECT

public:
    static styledMessageBox* instance();
    static void warning( QString message );
    static void info( QString message );
    static void critical( QString message );

signals:
    void userAcknowledged();

public slots:
    void onShowMessageBox(const QString& msg, const QString& module, const QString& actionString);
    void onHideMessageBox();

private slots:
    void on_buttonBox_accepted();

private:
    explicit styledMessageBox(QWidget *parent = nullptr, QString msg = QString());
    ~styledMessageBox();
    void setHasCancel( bool has, bool cancelIsDefault = false, bool hasOk = true );
    void setTitle( QString titleMsg );
    void setInstructions( QString instructionsMsg );
    void setText( QString msg );
    QString text();

private:
    static styledMessageBox* m_instance;
    Ui::styledMessageBox *ui;
    backgroundMask *mask;
};

#endif // STYLEDMESSAGEBOX_H
