/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution
*/
#ifndef REVDESC_H
#define REVDESC_H

#include <QTextBrowser>

class Domain;

class RevDesc : public QTextBrowser
{
    Q_OBJECT

    Domain* d;
    QString highlightedLink;

private slots:
    void on_anchorClicked( const QUrl& link );
    void on_highlighted( const QUrl& link );
    void on_linkCopy();

protected:
    virtual void contextMenuEvent( QContextMenuEvent* e );

public:
    RevDesc( QWidget* parent );
    void setup( Domain* dm ) { d = dm; }
};

#endif
