/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef MYPROCESS_H
#define MYPROCESS_H

#include "git.h"

#include <QProcess>

class Git;

//
// Custom process used to run shell commands in parallel
//
class MyProcess : public QProcess
{
    Q_OBJECT

    QObject* guiObject;
    Git* git;
    QString runCmd;
    QByteArray* runOutput;
    QString workDir;
    QObject* receiver;
    QStringList arguments;
    bool errorReportingEnabled;
    bool canceling;
    bool busy;
    bool async;
    bool isWinShell;
    bool isErrorExit;

    void setupSignals();
    bool launchMe( SCRef runCmd, SCRef buf );
    void sendErrorMsg( bool notStarted = false, SCRef errDesc = "" );
    static void restoreSpaces( QString& newCmd, const QChar& sepChar );

private slots:
    void on_readyReadStandardOutput();
    void on_readyReadStandardError();
    void on_finished( int, QProcess::ExitStatus );

public:
    MyProcess( QObject* go, Git* g, const QString& wd, bool reportErrors );

    bool runSync( SCRef runCmd, QByteArray* runOutput, QObject* rcv, SCRef buf );
    bool runAsync( SCRef rc, QObject* rcv, SCRef buf );
    static const QStringList splitArgList( SCRef cmd );

signals:
    void procDataReady( const QByteArray& );
    void eof();

public slots:
    void on_cancel();
};

#endif
