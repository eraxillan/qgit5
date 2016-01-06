/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef LANES_H
#define LANES_H

#include <QString>
#include <QVector>

class QStringList;

//
//  At any given time, the Lanes class represents a single revision (row) of the history graph.
//  The Lanes class contains a vector of the sha1 hashes of the next commit to appear in each lane
//  (column).
//  The Lanes class also contains a vector used to decide which glyph to draw on the history graph.
//
//  For each revision (row) (from recent (top) to ancient past (bottom)), the Lanes class is
//  updated, and the
//  current revision (row) of glyphs is saved elsewhere (via getLanes()).
//
//  The ListView class is responsible for rendering the glyphs.
//

class Lanes
{
    int activeLane;
    //
    // Describes which glyphs should be drawn
    //
    QVector< int > typeVec;
    //
    // The sha1 hashes of the next commit to appear in each lane (column)
    //
    QVector< QString > nextShaVec;
    bool boundary;
    int NODE, NODE_L, NODE_R;

private:
    int findNextSha( const QString& next, int pos );
    int findType( int type, int pos );
    int add( int type, const QString& next, int pos );

public:
    Lanes() {}  // NOTE: init() will setup us later, when data become available

    bool isEmpty() { return typeVec.empty(); }
    void init( const QString& expectedSha );
    void clear();
    bool isFork( const QString& sha, bool& isDiscontinuity );
    void setBoundary( bool isBoundary );
    void setFork( const QString& sha );
    void setMerge( const QStringList& parents );
    void setInitial();
    void setApplied();
    void changeActiveLane( const QString& sha );
    void afterMerge();
    void afterFork();
    bool isBranch();
    void afterBranch();
    void afterApplied();
    void nextParent( const QString& sha );
    
    //
    // NOTE: O(1) vector is implicitly shared
    //
    void getLanes( QVector< int >& ln ) { ln = typeVec; }
};

#endif
