/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#include <QApplication>
#include <QStatusBar>
#include <QTimer>

#include "FileHistory.h"
#include "domain.h"
#include "exceptionmanager.h"
#include "git.h"
#include "mainimpl.h"

using namespace QGit;

UpdateDomainEvent::UpdateDomainEvent( bool fromMaster, bool force )
    : QEvent( fromMaster ? ( QEvent::Type )QGit::UPD_DM_MST_EV : ( QEvent::Type )QGit::UPD_DM_EV ),
      f( force )
{
}

bool
UpdateDomainEvent::isForced() const
{
    return f;
}

//-----------------------------------------------------------------------------

StateInfo::S::S() { clear(); }
void
StateInfo::S::clear()
{
    sha = fn = dtSha = "";
    isM = allM = false;
    sel = true;
}

bool
StateInfo::S::operator==( const StateInfo::S& st ) const
{
    if( &st == this )
        return true;

    return ( sha == st.sha && fn == st.fn && dtSha == st.dtSha && sel == st.sel && isM == st.isM &&
             allM == st.allM );
}

bool
StateInfo::S::operator!=( const StateInfo::S& st ) const
{
    return !( StateInfo::S::operator==( st ) );
}

//-----------------------------------------------------------------------------

void
StateInfo::clear()
{
    nextS.clear();
    curS.clear();
    prevS.clear();

    isLocked = false;
}

const QString
StateInfo::sha( bool n ) const
{
    return ( n ? curS.sha : prevS.sha );
}

const QString
StateInfo::fileName( bool n ) const
{
    return ( n ? curS.fn : prevS.fn );
}

const QString
StateInfo::diffToSha( bool n ) const
{
    return ( n ? curS.dtSha : prevS.dtSha );
}

bool
StateInfo::selectItem( bool n ) const
{
    return ( n ? curS.sel : prevS.sel );
}

bool
StateInfo::isMerge( bool n ) const
{
    return ( n ? curS.isM : prevS.isM );
}

bool
StateInfo::allMergeFiles( bool n ) const
{
    return ( n ? curS.allM : prevS.allM );
}

void
StateInfo::setSha( const QString& s )
{
    if( isLocked )
        nextS.sha = s;
    else
        curS.sha = s;
}

void
StateInfo::setFileName( const QString& s )
{
    if( isLocked )
        nextS.fn = s;
    else
        curS.fn = s;
}

void
StateInfo::setDiffToSha( const QString& s )
{
    if( isLocked )
        nextS.dtSha = s;
    else
        curS.dtSha = s;
}

void
StateInfo::setSelectItem( bool b )
{
    if( isLocked )
        nextS.sel = b;
    else
        curS.sel = b;
}

void
StateInfo::setIsMerge( bool b )
{
    if( isLocked )
        nextS.isM = b;
    else
        curS.isM = b;
}

void
StateInfo::setAllMergeFiles( bool b )
{
    if( isLocked )
        nextS.allM = b;
    else
        curS.allM = b;
}

bool
StateInfo::requestPending() const
{
    return ( !nextS.sha.isEmpty() && ( nextS != curS ) );
}

void
StateInfo::setLock( bool b )
{
    isLocked = b;
    if( b )
        nextS = curS;
}

void
StateInfo::commit()
{
    prevS = curS;
}

void
StateInfo::rollBack()
{
    if( nextS == curS )
    {
        // invalidate to avoid infinite loop
        nextS.clear();
    }

    curS = prevS;
}

bool
StateInfo::flushQueue()
{
    if( requestPending() )
    {
        curS = nextS;
        return true;
    }
    return false;
}

StateInfo::StateInfo() { clear(); }
StateInfo&
StateInfo::operator=( const StateInfo& newState )
{
    if( &newState != this )
    {
        if( isLocked )
        {
            nextS = newState.curS;
        }
        else
        {
            //
            // prevS is mot modified to allow a rollback
            //
            curS = newState.curS;
        }
    }
    return *this;
}

bool
StateInfo::operator==( const StateInfo& newState ) const
{
    if( &newState == this )
        return true;

    //
    // Compare is made on curS only
    //
    return ( curS == newState.curS );
}

bool
StateInfo::operator!=( const StateInfo& newState ) const
{
    return !( StateInfo::operator==( newState ) );
}

bool
StateInfo::isChanged( uint what ) const
{
    bool ret = false;
    if( what & SHA )
        ret = ( sha( true ) != sha( false ) );

    if( !ret && ( what & FILE_NAME ) )
        ret = ( fileName( true ) != fileName( false ) );

    if( !ret && ( what & DIFF_TO_SHA ) )
        ret = ( diffToSha( true ) != diffToSha( false ) );

    if( !ret && ( what & ALL_MERGE_FILES ) )
        ret = ( allMergeFiles( true ) != allMergeFiles( false ) );

    return ret;
}

// ************************* Domain ****************************

Domain::Domain() {}

Domain::Domain( MainImpl* m, Git* g, bool isMain ) : QObject( m ), git( g )
{
    EM_INIT( exDeleteRequest, "Deleting domain" );
    EM_INIT( exCancelRequest, "Canceling update" );

    fileHistory = new FileHistory( this, git );
    if( isMain )
        git->setDefaultModel( fileHistory );

    st.clear();
    busy = readyToDrag = dragging = dropping = linked = false;
    popupType = 0;
    //
    // Will be reparented to m()->tabWdg
    //
    container = new QWidget( NULL );
}

Domain::~Domain()
{
    if( !parent() )
        return;

    //
    // Remove before to delete, avoids a Qt warning in QInputContext()
    //
    m()->tabWdg->removeTab( m()->tabWdg->indexOf( container ) );
    container->deleteLater();
}

void
Domain::clear( bool complete )
{
    if( complete )
        st.clear();

    fileHistory->clear();
}

void
Domain::on_closeAllTabs()
{
    //
    // NOTE: must be sync, deleteLater() does not work
    //
    delete this;
}

void
Domain::deleteWhenDone()
{
    if( !EM_IS_PENDING( exDeleteRequest ) )
        EM_RAISE( exDeleteRequest );

    emit cancelDomainProcesses();

    on_deleteWhenDone();
}

void
Domain::on_deleteWhenDone()
{
    if( !EM_IS_PENDING( exDeleteRequest ) )
        deleteLater();
    else
        QTimer::singleShot( 20, this, SLOT( on_deleteWhenDone() ) );
}

void
Domain::setThrowOnDelete( bool b )
{
    if( b )
        EM_REGISTER( exDeleteRequest );
    else
        EM_REMOVE( exDeleteRequest );
}

bool
Domain::isThrowOnDeleteRaised( int excpId, SCRef curContext )
{
    return EM_MATCH( excpId, exDeleteRequest, curContext );
}

MainImpl*
Domain::m() const
{
    return static_cast< MainImpl* >( parent() );
}

FileHistory*
Domain::model() const
{
    return fileHistory;
}

bool
Domain::isReadyToDrag() const
{
    return readyToDrag;
}

void
Domain::showStatusBarMessage( const QString& msg, int timeout )
{
    m()->statusBar()->showMessage( msg, timeout );
}

void
Domain::setTabCaption( const QString& caption )
{
    int idx = m()->tabWdg->indexOf( container );
    m()->tabWdg->setTabText( idx, caption );
}

bool
Domain::setReadyToDrag( bool b )
{
    readyToDrag = ( b && !busy && !dragging && !dropping );
    return readyToDrag;
}

bool
Domain::isDragging() const
{
    return dragging;
}

bool
Domain::setDragging( bool b )
{
    bool dragFinished = ( !b && dragging );

    dragging = ( b && readyToDrag && !dropping );

    if( dragging )
        readyToDrag = false;

    if( dragFinished )
        flushQueue();

    return dragging;
}

bool
Domain::isDropping() const
{
    return dropping;
}

void
Domain::setDropping( bool b )
{
    dropping = b;
}

bool
Domain::isLinked() const
{
    return linked;
}

QWidget*
Domain::tabPage() const
{
    return container;
}

bool Domain::isMatch( SCRef ) { return false; }
void
Domain::unlinkDomain( Domain* d )
{
    d->linked = false;

    //
    // NOTE: a signal is emitted for every connection you make,
    // so if you duplicate a connection, two signals will be emitted.
    //
    while( d->disconnect( SIGNAL( updateRequested( StateInfo ) ), this ) )
        ;
}

void
Domain::linkDomain( Domain* d )
{
    //
    // Be sure only one connection is active
    //
    unlinkDomain( d );

    connect( d, SIGNAL( updateRequested( StateInfo ) ), this,
             SLOT( on_updateRequested( StateInfo ) ) );
    d->linked = true;
}

void
Domain::on_updateRequested( StateInfo newSt )
{
    st = newSt;
    UPDATE();
}

bool
Domain::flushQueue()
{
    //
    // During dragging any state update is queued, so try to flush pending now
    //
    if( !busy && st.flushQueue() )
    {
        UPDATE();
        return true;
    }
    return false;
}

bool
Domain::event( QEvent* e )
{
    bool fromMaster = false;

    switch( e->type() )
    {
    case UPD_DM_MST_EV:
        fromMaster = true;
    // fall through
    case UPD_DM_EV:
        update( fromMaster, ( ( UpdateDomainEvent* )e )->isForced() );
        break;
    case MSG_EV:
        if( !busy && !st.requestPending() )
        {
            QApplication::postEvent( m(), new MessageEvent( ( ( MessageEvent* )e )->myData() ) );
        }
        else
        {
            // Waiting for the end of updating
            statusBarRequest = ( ( MessageEvent* )e )->myData();
        }
        break;
    default:
        break;
    }

    return QObject::event( e );
}

void
Domain::populateState()
{
    const Rev* r = git->revLookup( st.sha() );
    if( r )
        st.setIsMerge( r->parentsCount() > 1 );
}

void
Domain::update( bool fromMaster, bool force )
{
    if( busy && st.requestPending() )
    {
        //
        // Quick exit current (obsoleted) update but only if state
        // is going to change. Without this check calling update()
        // many times with the same data nullify the update
        //
        EM_RAISE( exCancelRequest );
        emit cancelDomainProcesses();
    }

    if( busy || dragging )
        return;

    if( linked && !fromMaster )
    {
        // In this case let the update to fall down from master domain
        StateInfo tmp( st );
        st.rollBack();  // We don't want to filter out next update sent from master
        emit updateRequested( tmp );
        return;
    }

    try
    {
        //
        // Quiet, no messages when thrown
        //
        EM_REGISTER_Q( exCancelRequest );
        setThrowOnDelete( true );

        git->setThrowOnStop( true );
        git->setCurContext( this );

        busy = true;
        setReadyToDrag( false );  // Do not start any drag while updating
        populateState();          // Complete any missing state information
        st.setLock( true );       // Any state change will be queued now

        if( doUpdate( force ) )
            st.commit();
        else
            st.rollBack();

        st.setLock( false );
        busy = false;
        if( git->curContext() != this )
        {
            qDebug( "ASSERT in Domain::update, context is %p instead of %p",
                    ( void* )git->curContext(), ( void* )this );
        }

        git->setCurContext( NULL );
        git->setThrowOnStop( false );
        setThrowOnDelete( false );
        EM_REMOVE( exCancelRequest );
    }
    catch( int i )
    {
        //
        // If we have a cancel request because of a new update is in queue we
        // cannot roolback current state to avoid new update is filtered out
        // in case rolled back sha and new sha are the same.
        // This could happen with arrow navigation:
        //
        // sha -> go UP (new sha) -> go DOWN (cancel) -> rollback to sha
        //
        // And pending request 'sha' is filtered out leaving us in an
        // inconsistent state.
        //
        st.commit();
        st.setLock( false );
        busy = false;

        git->setCurContext( NULL );
        git->setThrowOnStop( false );
        setThrowOnDelete( false );
        EM_REMOVE( exCancelRequest );

        if( QApplication::overrideCursor() )
            QApplication::restoreOverrideCursor();

        QString context( "updating " );
        if( git->isThrowOnStopRaised( i, context + metaObject()->className() ) )
        {
            EM_THROW_PENDING;
            return;
        }

        if( isThrowOnDeleteRaised( i, context + metaObject()->className() ) )
        {
            EM_THROW_PENDING;
            return;
        }

        if( i == exCancelRequest )
        {
            EM_THROW_PENDING;
            // Do not return
        }
        else
        {
            const QString info( "Exception \'" + EM_DESC( i ) +
                                "\' "
                                "not handled in init...re-throw" );
            dbs( info );
            throw;
        }
    }

    bool nextRequestPending = flushQueue();
    if( !nextRequestPending && !statusBarRequest.isEmpty() )
    {
        //
        // Update status bar when we are sure no more work is pending
        //
        QApplication::postEvent( m(), new MessageEvent( statusBarRequest ) );
        statusBarRequest = "";
    }
    if( !nextRequestPending && popupType )
        sendPopupEvent();
}

void
Domain::sendPopupEvent()
{
    //
    // Call an async context popup, must be executed after returning to event loop
    //
    DeferredPopupEvent* e = new DeferredPopupEvent( popupData, popupType );
    QApplication::postEvent( m(), e );
    popupType = 0;
}

void
Domain::on_contextMenu( const QString& data, int type )
{
    popupType = type;
    popupData = data;

    if( busy )
    {
        //
        // We are in the middle of an update
        //
        return;
    }

    //
    // If list view is already updated pop-up
    // context menu, otherwise it means update()
    // has still not been called, a pop-up request,
    // will be fired up at the end of next update()
    //
    if( ( type == POPUP_LIST_EV ) && ( data != st.sha() ) )
        return;

    sendPopupEvent();
}
