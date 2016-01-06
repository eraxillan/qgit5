/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef FILECONTENT_H
#define FILECONTENT_H

#include <QPointer>
#include <QTextEdit>

#include "common.h"

class FileHighlighter;
class Domain;
class StateInfo;
class Annotate;
class Git;
class MyProcess;
struct RangeInfo;
class FileHistory;

class QListWidget;
class QListWidgetItem;

class FileContent : public QTextEdit
{
    Q_OBJECT

    friend class FileHighlighter;

    enum BoolOption
    {
        //
        // Used as self-documenting boolean parameters
        //
        optFalse,
        optEmitSignal
    };

    Domain* d;
    Git* git;
    QListWidget* listWidgetAnn;
    StateInfo* st;
    RangeInfo* rangeInfo;
    FileHighlighter* fileHighlighter;
    QPointer< MyProcess > proc;
    QPointer< Annotate > annotateObj;  // Valid from beginning of annotation loading
    const FileAnnotation* curAnn;      // Valid at the end of annotation loading
    QByteArray fileRowData;
    QString histTime;
    bool isFileAvail;
    bool isAnnotationLoading;
    bool isAnnotationAppended;
    bool isRangeFilterActive;
    bool isShowAnnotate;
    bool isHtmlSource;
    bool isImageFile;

    struct ScreenState
    {
        bool isValid, hasSelectedText;
        int topPara, paraFrom, indexFrom, paraTo, indexTo;
    };
    ScreenState ss;

    //
    // NOTE: declared as private, to avoid indirect access to QTextEdit::clear()
    //
    void clear();

    void clearAnnotate( bool emitSignal );
    void clearText( bool emitSignal );
    void findInFile( SCRef str );
    void scrollCursorToTop();
    void scrollLineToTop( int lineNum );
    int positionToLineNum( int pos = -1 );
    int lineAtTop();
    bool lookupAnnotation();
    uint annotateLength( const FileAnnotation* curAnn );
    void saveScreenState();
    void restoreScreenState();
    void showFileImage();
    void adjustAnnListSize( int width );
    void setAnnList();

protected:
    virtual void resizeEvent( QResizeEvent* e );

private slots:
    void on_list_doubleClicked( QListWidgetItem* );
    void on_scrollBar_valueChanged( int );
    void on_listScrollBar_valueChanged( int );

public:
    FileContent( QWidget* parent );
    ~FileContent();

    void setup( Domain* parent, Git* git, QListWidget* lwa );
    void doUpdate( bool force = false );
    void clearAll( bool emitSignal = true );
    void copySelection();
    void goToAnnotation( int id, int direction );
    bool goToRangeStart();
    bool rangeFilter( bool b );
    bool getRange( SCRef sha, RangeInfo* r );
    bool startAnnotate( FileHistory* fh, SCRef histTime );
    void setShowAnnotate( bool b );
    void setHighlightSource( bool b );
    void setSelection( int paraFrom, int indexFrom, int paraTo, int indexTo );
    int itemAnnId( QListWidgetItem* item );
    bool isFileAvailable() const;
    bool isAnnotateAvailable() const;

signals:
    void annotationAvailable( bool );
    void fileAvailable( bool );
    void revIdSelected( int );

public slots:
    void on_annotateReady( Annotate*, bool, const QString& );
    void procReadyRead( const QByteArray& );
    void procFinished( bool emitSignal = true );
    void typeWriterFontChanged();
};

#endif
