/*
    Description: stdout viewer

    Author: Marco Costalba (C) 2006-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef CONSOLEIMPL_H
#define CONSOLEIMPL_H

#include <QCloseEvent>
#include <QPointer>

#include "ui_console.h"

class MyProcess;
class Git;

// we need a statusbar
class ConsoleImpl : public QMainWindow, Ui_Console
{
    Q_OBJECT

    Git* git;
    QString actionName;
    QPointer< MyProcess > proc;
    QString inpBuf;

protected:
    virtual void closeEvent( QCloseEvent* ce );

protected slots:
    void pushButtonTerminate_clicked();
    void pushButtonOk_clicked();

public:
    ConsoleImpl( const QString& nm, Git* g );
    bool start( const QString& cmd, const QString& args );

signals:
    void customAction_exited( const QString& name );

public slots:
    void typeWriterFontChanged();
    void procReadyRead( const QByteArray& data );
    void procFinished();
};

#endif
