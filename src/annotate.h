/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef ANNOTATE_H
#define ANNOTATE_H

#include <QObject>
#include <QTime>

#include "common.h"
#include "exceptionmanager.h"

class Git;
class FileHistory;
class MyProcess;

struct RangeInfo
{
    int start, end;  // Ranges count file lines from 1 like patches diffs
    bool modified;

    RangeInfo();
    RangeInfo( int s, int e, bool m );
    void clear();
};
typedef QHash< QString, RangeInfo > Ranges;

//-----------------------------------------------------------------------------

class Annotate : public QObject
{
    Q_OBJECT

    EM_DECLARE( exAnnCanceled );

    Git* git;
    QObject* gui;
    const FileHistory* fh;
    AnnotateHistory ah;
    bool cancelingAnnotate;
    bool annotateRunning;
    bool annotateActivity;
    bool isError;
    int annNumLen;
    int annId;
    int annFilesNum;
    ShaVect histRevOrder;  // TODO use reference
    bool valid;
    bool canceled;
    QTime processingTime;
    Ranges ranges;

private:
    void annotateFileHistory();
    void doAnnotate( const ShaString& sha );
    FileAnnotation* getFileAnnotation( SCRef sha );
    void setInitialAnnotation( SCRef fileSha, FileAnnotation* fa );
    const QString setupAuthor( SCRef origAuthor, int annId );
    bool setAnnotation( SCRef diff, SCRef aut, SCList pAnn, SList nAnn, int ofs = 0 );
    bool getNextLine( SCRef d, int& idx, QString& line );
    static void unify( SList dst, SCList src );
    const QString getPatch( SCRef sha, int parentNum = 0 );
    bool getNextSection( SCRef d, int& idx, QString& sec, SCRef target );
    void updateRange( RangeInfo* r, SCRef diff, bool reverse );
    void updateCrossRanges( SCRef cnk, bool rev, int oStart, int oLineCnt, RangeInfo* r );
    bool isDescendant( SCRef sha, SCRef target );

private slots:
    void on_deleteWhenDone();
    void slotComputeDiffs();

public:
    Annotate( Git* parent, QObject* guiObj );

    void deleteWhenDone();
    const FileAnnotation* lookupAnnotation( SCRef sha );
    bool start( const FileHistory* fh );
    bool isCanceled();
    const QString getAncestor( SCRef sha, int* shaIdx );
    bool getRange( SCRef sha, RangeInfo* r );
    bool seekPosition( int* rangeStart, int* rangeEnd, SCRef fromSha, SCRef toSha );
    const QString computeRanges( SCRef sha, int paraFrom, int paraTo, SCRef target = "" );

signals:
    void annotateReady( Annotate*, bool, const QString& );
};

#endif
