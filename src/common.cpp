/*
        Description: interface to git programs

        Author: Marco Costalba (C) 2005-2007

        Copyright: See COPYING file that comes with this distribution

*/

#include "common.h"

#include <QDataStream>
#include <QTextCodec>

const QString
Rev::mid( int start, int len ) const
{
    //
    // FIXME: no sanity check is done on arguments
    //
    const char* data = ba.constData();
#if QT_VERSION < 0x050000
    return QString::fromAscii( data + start, len );
#else
    QTextCodec* codec = QTextCodec::codecForLocale();
    if( !codec )
    {
        dbp( "ASSERT: local codec <%1> not available", 0 );
        return QString();
    }
    return codec->toUnicode( data + start, len ).toUtf8();
#endif
}

const QString
Rev::midSha( int start, int len ) const
{
    //
    // FIXME: no sanity check is done on arguments
    //
    const char* data = ba.constData();
    //
    // NOTE: faster then fromAscii, and don't deprecated in Qt5
    //
    return QString::fromLatin1( data + start, len );
}

Rev::Rev( const QByteArray& b, uint s, int idx, int* next, bool withDiff )
    : orderIdx( idx ), ba( b ), start( s )
{
    indexed = isDiffCache = isApplied = isUnApplied = false;
    descRefsMaster = ancRefsMaster = descBrnMaster = -1;
    *next = indexData( true, withDiff );
}

bool
Rev::isBoundary() const
{
    return ( ba.at( shaStart - 1 ) == '-' );
}

uint
Rev::parentsCount() const
{
    return parentsCnt;
}

const ShaString
Rev::parent( int idx ) const
{
    return ShaString( ba.constData() + shaStart + 41 + 41 * idx );
}

const QStringList
Rev::parents() const
{
    QStringList p;
    int idx = shaStart + 41;

    for( int i = 0; i < parentsCnt; i++ )
    {
        p.append( midSha( idx, 40 ) );
        idx += 41;
    }
    return p;
}

const ShaString
Rev::sha() const
{
    return ShaString( ba.constData() + shaStart );
}

const QString
Rev::committer() const
{
    setup();
    return mid( comStart, autStart - comStart - 1 );
}

const QString
Rev::author() const
{
    setup();
    return mid( autStart, autDateStart - autStart - 1 );
}

const QString
Rev::authorDate() const
{
    setup();
    return mid( autDateStart, 10 );
}

const QString
Rev::shortLog() const
{
    setup();
    return mid( sLogStart, sLogLen );
}

const QString
Rev::longLog() const
{
    setup();
    return mid( lLogStart, lLogLen );
}

const QString
Rev::diff() const
{
    setup();
    return mid( diffStart, diffLen );
}

void
Rev::setup() const
{
    if( !indexed )
        indexData( false, false );
}

int
Rev::indexData( bool quick, bool withDiff ) const
{
    //
    //    This is what 'git log' produces:
    //
    //    - a possible one line with "Final output:\n" in case of --early-output option
    //    - one line with "log size" + len of this record
    //    - one line with boundary info + sha + an arbitrary amount of parent's sha
    //    - one line with committer name + e-mail
    //    - one line with author name + e-mail
    //    - one line with author date as unix timestamp
    //    - zero or more non blank lines with other info, as the encoding FIXME
    //    - one blank line
    //    - zero or one line with log title
    //    - zero or more lines with log message
    //    - zero or more lines with diff content (only for file history)
    //    - a terminating '\0'
    //
    static int error = -1;
    static int shaLength = 40;                  // From git ref. spec.
    static int shaEndlLength = shaLength + 1;   // An sha key + \n
    static int shaXEndlLength = shaLength + 2;  // An sha key + X marker + \n
    static char finalOutputMarker = 'F';        // Marks the beginning of "Final output" string
    static char logSizeMarker = 'l';            // Marks the beginning of "log size" string
    static int logSizeStrLength = 9;            // "log size"
    static int asciiPosOfZeroChar = 48;         // Char "0" has value 48 in ascii table

    const int last = ba.size() - 1;
    int logSize = 0, idx = start;
    int logEnd, revEnd;

    //
    // Direct access is faster then QByteArray.at()
    //
    const char* data = ba.constData();
    char* fixup = const_cast< char* >( data );  // To build '\0' terminating strings

    if( start + shaXEndlLength > last )  // At least sha header must be present
        return -1;

    if( data[ start ] == finalOutputMarker )  // "Final output", let caller handle this
        return ( ba.indexOf( '\n', start ) != -1 ? -2 : -1 );

    //
    // Parse   'log size xxx\n'   if present -- from git ref. spec.
    //
    if( data[ idx ] == logSizeMarker )
    {
        idx += logSizeStrLength;  // Move idx to beginning of log size value

        //
        // Parse log size value
        //
        int digit;
        while( ( digit = data[ idx++ ] ) != '\n' )
            logSize = logSize * 10 + digit - asciiPosOfZeroChar;
    }
    //
    // idx points to the boundary information, which has the same length as an sha header
    //
    if( ++idx + shaXEndlLength > last )
        return error;

    shaStart = idx;

    //
    // Ok, now shaStart is valid but msgSize could be still 0 if not available
    //
    logEnd = shaStart - 1 + logSize;
    if( logEnd > last )
        return error;

    idx += shaLength;  // Now points to 'X' place holder

    fixup[ idx ] = '\0';  // We want sha to be a '\0' terminated ascii string

    parentsCnt = 0;

    if( data[ idx + 2 ] == '\n' )  // Initial revision
        ++idx;
    else
        do
        {
            parentsCnt++;
            idx += shaEndlLength;

            if( idx + 1 >= last )
                break;

            fixup[ idx ] = '\0';  // We want parents '\0' terminated

        } while( data[ idx + 1 ] != '\n' );

    ++idx;  // Now points to the trailing '\n' of sha line

    //
    // Check for !msgSize
    //
    if( withDiff || !logSize )
    {
        revEnd = ( logEnd > idx ) ? logEnd - 1 : idx;
        revEnd = ba.indexOf( '\0', revEnd + 1 );
        if( revEnd == -1 )
            return -1;
    }
    else
        revEnd = logEnd;

    if( revEnd > last )  // After this point we know to have the whole record
        return error;

    //
    // Ok, now revEnd is valid but logEnd could be not if !logSize
    // in case of diff we are sure content will be consumed so
    // we go all the way
    //
    if( quick && !withDiff )
        return ++revEnd;

    //
    // Commiter
    //
    comStart = ++idx;
    idx = ba.indexOf( '\n', idx );  // committer line end
    if( idx == -1 )
    {
        dbs( "ASSERT in indexData: unexpected end of data" );
        return -1;
    }

    //
    // Author
    //
    autStart = ++idx;
    idx = ba.indexOf( '\n', idx );  // author line end
    if( idx == -1 )
    {
        dbs( "ASSERT in indexData: unexpected end of data" );
        return -1;
    }

    //
    // Author date in Unix format (seconds since epoch)
    //
    autDateStart = ++idx;
    idx = ba.indexOf( '\n', idx );  // author date end without '\n'
    if( idx == -1 )
    {
        dbs( "ASSERT in indexData: unexpected end of data" );
        return -1;
    }
    //
    // If no error, point to trailing \n
    //
    ++idx;

    diffStart = diffLen = 0;
    if( withDiff )
    {
        diffStart = logSize ? logEnd : ba.indexOf( "\ndiff ", idx );

        if( diffStart != -1 && diffStart < revEnd )
            diffLen = revEnd - ++diffStart;
        else
            diffStart = 0;
    }
    if( !logSize )
        logEnd = diffStart ? diffStart : revEnd;

    //
    // Ok, now logEnd is valid and we can handle the log
    //
    sLogStart = idx;

    if( logEnd < sLogStart )
    {
        //
        // No shortlog no longLog
        //
        sLogStart = sLogLen = 0;
        lLogStart = lLogLen = 0;
    }
    else
    {
        lLogStart = ba.indexOf( '\n', sLogStart );
        if( lLogStart != -1 && lLogStart < logEnd - 1 )
        {
            sLogLen = lLogStart - sLogStart;  // Skip sLog trailing '\n'
            lLogLen = logEnd - lLogStart;     // Include heading '\n' in long log
        }
        else
        {
            //
            // no longLog
            //
            sLogLen = logEnd - sLogStart;
            if( data[ sLogStart + sLogLen - 1 ] == '\n' )
                sLogLen--;  // Skip trailing '\n' if any

            lLogStart = lLogLen = 0;
        }
    }
    indexed = true;
    return ++revEnd;
}

RevFile::RevFile() : onlyModified( true ) {}

int
RevFile::dirAt( uint idx ) const
{
    return ( ( int* )pathsIdx.constData() )[ idx ];
}

int
RevFile::nameAt( uint idx ) const
{
    return ( ( int* )pathsIdx.constData() )[ count() + idx ];
}

int
RevFile::count() const
{
    return pathsIdx.size() / ( ( int )sizeof( int ) * 2 );
}

bool
RevFile::statusCmp( int idx, RevFile::StatusFlag sf ) const
{
    return ( ( onlyModified ? MODIFIED : status.at( idx ) ) & sf );
}

const QString
RevFile::extendedStatus( int idx ) const
{
    //
    //   rf.extStatus has size equal to position of latest copied/renamed file,
    //   that could be lower then count(), so we have to explicitly check for
    //   an out of bound condition
    //
    return ( !extStatus.isEmpty() && idx < extStatus.count() ? extStatus.at( idx ) : "" );
}

//
// RevFile streaming out
//
const RevFile&
RevFile::operator>>( QDataStream& stream ) const
{
    stream << pathsIdx;

    //
    // Skip common case of only modified files
    //
    bool isEmpty = onlyModified;
    stream << ( quint32 )isEmpty;
    if( !isEmpty )
        stream << status;

    //
    // Skip common case of just one parent
    //
    isEmpty = ( mergeParent.isEmpty() || mergeParent.last() == 1 );
    stream << ( quint32 )isEmpty;
    if( !isEmpty )
        stream << mergeParent;

    //
    // Skip common case of no rename/copies
    //
    isEmpty = extStatus.isEmpty();
    stream << ( quint32 )isEmpty;
    if( !isEmpty )
        stream << extStatus;

    return *this;
}

//
// RevFile streaming in
//
RevFile&
RevFile::operator<<( QDataStream& stream )
{
    stream >> pathsIdx;

    bool isEmpty;
    quint32 tmp;

    stream >> tmp;
    onlyModified = ( bool )tmp;
    if( !onlyModified )
        stream >> status;

    stream >> tmp;
    isEmpty = ( bool )tmp;
    if( !isEmpty )
        stream >> mergeParent;

    stream >> tmp;
    isEmpty = ( bool )tmp;
    if( !isEmpty )
        stream >> extStatus;

    return *this;
}

//-----------------------------------------------------------------------------

FileAnnotation::FileAnnotation( int id ) : isValid( false ), annId( id ) {}

FileAnnotation::FileAnnotation() : isValid( false ) {}

BaseEvent::BaseEvent( SCRef d, int id ) : QEvent( ( QEvent::Type )id ), payLoad( d ) {}

const QString
BaseEvent::myData() const
{
    return payLoad;
}

DeferredPopupEvent::DeferredPopupEvent( SCRef msg, int type ) : BaseEvent( msg, type ) {}

MainExecErrorEvent::MainExecErrorEvent( SCRef c, SCRef e )
    : BaseEvent( "", QGit::ERROR_EV ), cmd( c ), err( e )
{
}

const QString
MainExecErrorEvent::command() const
{
    return cmd;
}

const QString
MainExecErrorEvent::report() const
{
    return err;
}
