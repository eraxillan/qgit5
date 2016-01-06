/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef DOMAIN_H
#define DOMAIN_H

#include <QEvent>
#include <QObject>

#include "common.h"
#include "exceptionmanager.h"

#define UPDATE_DOMAIN( x ) QApplication::postEvent( x, new UpdateDomainEvent( false ) )
#define UPDATE() QApplication::postEvent( this, new UpdateDomainEvent( false ) )
#define UPDATE_DM_MASTER( x, f ) QApplication::postEvent( x, new UpdateDomainEvent( true, f ) )

class Domain;
class FileHistory;
class Git;
class MainImpl;

class UpdateDomainEvent : public QEvent
{
    bool f;

public:
    explicit UpdateDomainEvent( bool fromMaster, bool force = false );
    bool isForced() const;
};

//-----------------------------------------------------------------------------

class StateInfo
{
    friend class Domain;

    class S
    {
    public:
        QString sha;
        QString fn;
        QString dtSha;
        bool sel;
        bool isM;
        bool allM;

    public:
        S();

        bool operator==( const S& newState ) const;
        bool operator!=( const S& newState ) const;

        void clear();
    };

    S curS;   // Current state, what returns from StateInfo::sha()
    S prevS;  // Previous good state, used to rollBack in case state update fails
    S nextS;  // Next queued state, waiting for current update to finish
    bool isLocked;

    bool requestPending() const;
    void setLock( bool b );
    void commit();
    void rollBack();
    bool flushQueue();

public:
    enum Field
    {
        SHA = 1,
        FILE_NAME = 2,
        DIFF_TO_SHA = 4,
        ALL_MERGE_FILES = 8,
        ANY = 15
    };

public:
    StateInfo();

    StateInfo& operator=( const StateInfo& newState );
    bool operator==( const StateInfo& newState ) const;
    bool operator!=( const StateInfo& newState ) const;

    void clear();

    const QString sha( bool n = true ) const;
    const QString fileName( bool n = true ) const;
    const QString diffToSha( bool n = true ) const;
    bool selectItem( bool n = true ) const;
    bool isMerge( bool n = true ) const;
    bool allMergeFiles( bool n = true ) const;
    void setSha( const QString& s );
    void setFileName( const QString& s );
    void setDiffToSha( const QString& s );
    void setSelectItem( bool b );
    void setIsMerge( bool b );
    void setAllMergeFiles( bool b );
    bool isChanged( uint what = ANY ) const;
};

//-----------------------------------------------------------------------------

class Domain : public QObject
{
    Q_OBJECT

    EM_DECLARE( exDeleteRequest );
    EM_DECLARE( exCancelRequest );

    FileHistory* fileHistory;
    bool readyToDrag;
    bool dragging;
    bool dropping;
    bool linked;
    int popupType;
    QString popupData;
    QString statusBarRequest;

    void populateState();
    void update( bool fromMaster, bool force );
    bool flushQueue();
    void sendPopupEvent();

protected:
    Git* git;
    QWidget* container;
    bool busy;

protected:
    virtual void clear( bool complete = true );
    virtual bool event( QEvent* e );
    virtual bool doUpdate( bool force ) = 0;

protected:
    void linkDomain( Domain* d );
    void unlinkDomain( Domain* d );
    void setTabCaption( const QString& caption );

protected slots:
    virtual void on_contextMenu( const QString&, int );
    void on_updateRequested( StateInfo newSt );
    void on_deleteWhenDone();

public:
    StateInfo st;

public:
    Domain();
    Domain( MainImpl* m, Git* git, bool isMain );
    virtual ~Domain();

    //
    // Will delete when no more run() are pending
    //
    void deleteWhenDone();

    void showStatusBarMessage( const QString& msg, int timeout = 0 );
    void setThrowOnDelete( bool b );
    bool isThrowOnDeleteRaised( int excpId, SCRef curContext );
    MainImpl* m() const;
    FileHistory* model() const;
    bool isReadyToDrag() const;
    bool setReadyToDrag( bool b );
    bool isDragging() const;
    bool setDragging( bool b );
    bool isDropping() const;
    void setDropping( bool b );
    bool isLinked() const;
    QWidget* tabPage() const;

public:
    virtual bool isMatch( SCRef );

signals:
    void updateRequested( StateInfo newSt );
    void cancelDomainProcesses();

public slots:
    void on_closeAllTabs();
};

#endif
