/*
    Author: Marco Costalba (C) 2005-2007

    Copyright: See COPYING file that comes with this distribution

*/
#ifndef RANGESELECTIMPL_H
#define RANGESELECTIMPL_H

#include "ui_rangeselect.h"

class Git;

class RangeSelectImpl : public QDialog, public Ui_RangeSelectBase
{
    Q_OBJECT

    Git* git;
    QString* range;

    void orderRefs( const QStringList& src, QStringList& dst );

public:
    RangeSelectImpl( QWidget* par, QString* range, bool rc, Git* g );

public slots:
    void pushButtonOk_clicked();
    void checkBoxDiffCache_toggled( bool b );
    void checkBoxShowAll_toggled( bool b );
    void checkBoxShowDialog_toggled( bool b );
    void checkBoxShowWholeHistory_toggled( bool b );
};

#endif
