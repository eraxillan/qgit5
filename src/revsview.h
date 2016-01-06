/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef REVSVIEW_H
#define REVSVIEW_H

#include "common.h"
#include "domain.h"

#include "ui_revsview.h"  // Needed by moc_* file to understand tab() function

#include <QPointer>

class MainImpl;
class Git;
class FileHistory;
class PatchView;

class RevsView : public Domain
{
    Q_OBJECT

    friend class MainImpl;

    Ui_TabRev* revTab;
    QPointer< PatchView > linkedPatchView;

    void updateLineEditSHA( bool clear = false );

protected:
    virtual bool doUpdate( bool force );

private slots:
    void on_newRevsAdded( const FileHistory*, const QVector< ShaString >& );
    void on_loadCompleted( const FileHistory*, const QString& stats );
    void on_lanesContextMenuRequested( const QStringList&, const QStringList& );
    void on_updateRevDesc();

public:
    RevsView( MainImpl* parent, Git* git, bool isMain = false );
    ~RevsView();

    void clear( bool complete );
    void viewPatch( bool newTab );
    void setEnabled( bool b );
    void setTabLogDiffVisible( bool );
    Ui_TabRev* tab() { return revTab; }

public slots:
    void toggleDiffView();
};

#endif
