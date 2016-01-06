/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef SMARTBROWSE_H
#define SMARTBROWSE_H

#include "revsview.h"

#include <QLabel>
#include <QTime>

class SmartLabel : public QLabel
{
    Q_OBJECT

protected:
    virtual void contextMenuEvent( QContextMenuEvent* e );

private slots:
    void switchLinks();

public:
    SmartLabel( const QString& text, QWidget* par );

    void paintEvent( QPaintEvent* event );
};

class SmartBrowse : public QObject
{
    Q_OBJECT

    RevsView* rv;
    SmartLabel* logTopLbl;
    SmartLabel* logBottomLbl;
    SmartLabel* diffTopLbl;
    SmartLabel* diffBottomLbl;
    QTime scrollTimer, switchTimer, timeoutTimer;
    int wheelCnt;
    bool lablesEnabled;

    QTextEdit* curTextEdit( bool* isDiff = NULL );
    void setVisible( bool b );
    void updatePosition();
    int visibilityFlags( bool* isDiff = NULL );
    bool wheelRolled( int delta, int flags );

protected:
    bool eventFilter( QObject* obj, QEvent* event );

public:
    SmartBrowse( RevsView* par );

public slots:
    void updateVisibility();
    void linkActivated( const QString& );
    void flagChanged( uint );
};

#endif
