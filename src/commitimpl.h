/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef COMMITIMPL_H
#define COMMITIMPL_H

#include "common.h"
#include "ui_commit.h"

class Git;

class CommitImpl : public QWidget, public Ui_CommitBase
{
    Q_OBJECT

    Git* git;
    QString origMsg;
    int ofsX, ofsY;

    static QString lastMsgBeforeError;

private:
    virtual void closeEvent( QCloseEvent* );

    void checkUncheck( bool checkAll );
    bool getFiles( SList selFiles );
    void warnNoFiles();
    bool checkFiles( SList selFiles );
    bool checkMsg( QString& msg );
    bool checkPatchName( QString& patchName );
    bool checkConfirm( SCRef msg, SCRef patchName, SCList selFiles, bool amend );
    void computePosition( int& col_pos, int& line_pos );

private slots:
    void contextMenuPopup( const QPoint& );
    void checkAll();
    void unCheckAll();

public:
    explicit CommitImpl( Git* g, bool amend );

signals:
    void changesCommitted( bool );

public slots:
    void pushButtonCommit_clicked();
    void pushButtonAmend_clicked();
    void pushButtonCancel_clicked();
    void pushButtonUpdateCache_clicked();
    void pushButtonSettings_clicked();
    void textEditMsg_cursorPositionChanged();
};

#endif
