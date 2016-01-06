/*
        Description: interface to git programs

        Author: Marco Costalba (C) 2005-2007

        Copyright: See COPYING file that comes with this distribution

*/

#ifndef FILEHISTORY_H
#define FILEHISTORY_H

#include <QAbstractItemModel>

#include "common.h"

// class Annotate;
// class DataLoader;
class Git;
class Lanes;

class FileHistory : public QAbstractItemModel
{
    Q_OBJECT

    friend class Annotate;
    friend class DataLoader;
    friend class Git;

    Git* git;
    RevMap revs;
    ShaVect revOrder;
    Lanes* lns;
    uint firstFreeLane;
    QList< QByteArray* > rowData;
    QList< QVariant > headerInfo;
    int rowCnt;
    bool annIdValid;
    unsigned long secs;
    int loadTime;
    int earlyOutputCnt;
    int earlyOutputCntBase;
    QStringList fNames;
    QStringList curFNames;
    QStringList renamedRevs;
    QHash< QString, QString > renamedPatches;

    void flushTail();
    const QString timeDiff( unsigned long secs ) const;

private slots:
    void on_newRevsAdded( const FileHistory*, const QVector< ShaString >& );
    void on_loadCompleted( const FileHistory*, const QString& );

public:
    FileHistory( QObject* parent, Git* git );
    ~FileHistory();

    void clear( bool complete = true );
    const QString sha( int row ) const;
    int row( SCRef sha ) const;
    const QStringList fileNames() const;
    void resetFileNames( SCRef fn );
    void setEarlyOutputState( bool b = true );
    void setAnnIdValid( bool b = true );

public:
    virtual QVariant data( const QModelIndex& index, int role ) const;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual QVariant headerData( int s, Qt::Orientation o, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int r, int c, const QModelIndex& par = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex& index ) const;
    virtual int rowCount( const QModelIndex& par = QModelIndex() ) const;
    virtual bool hasChildren( const QModelIndex& par = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex& ) const;

public slots:
    void on_changeFont( const QFont& );
};

#endif  // FILEHISTORY_H
