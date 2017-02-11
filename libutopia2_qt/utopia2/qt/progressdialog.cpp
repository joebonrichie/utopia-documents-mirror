/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#include <utopia2/qt/progressdialog.h>
#include <utopia2/qt/spinner.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

namespace Utopia
{

    class ProgressDialogPrivate
    {
    public:
        QLabel * label;
        Spinner * spinner;
        QTimer timer;
    };




    ProgressDialog::ProgressDialog(const QString & text, QWidget * parent, Qt::WindowFlags f)
        : QDialog(parent, f | Qt::WindowStaysOnTopHint), d(new ProgressDialogPrivate)
    {
        d->label = new QLabel;
        d->label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        d->spinner = new Spinner;
        d->spinner->setFixedSize(32, 32);
        d->spinner->setColor(QColor(80, 80, 80));

        setMinimumDuration(1500);

        d->timer.setSingleShot(true);
        connect(&d->timer, SIGNAL(timeout()), this, SLOT(show()));

        QHBoxLayout * layout = new QHBoxLayout(this);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(16);
        layout->addWidget(d->spinner, 0);
        layout->addWidget(d->label, 1);

        setText(text);
    }

    ProgressDialog::~ProgressDialog()
    {
        delete d;
    }

    int ProgressDialog::minimumDuration() const
    {
        return d->timer.interval();
    }

    qreal ProgressDialog::progress() const
    {
        return d->spinner->progress();
    }

    void ProgressDialog::setMinimumDuration(int minimumDuration)
    {
        d->timer.setInterval(minimumDuration);
    }

    void ProgressDialog::setProgress(qint64 progress, qint64 expected)
    {
        d->spinner->setProgress(progress, expected);
    }

    void ProgressDialog::setProgress(qreal progress)
    {
        d->spinner->setProgress(progress);
    }

    void ProgressDialog::setText(const QString & text)
    {
        d->label->setText(text);
    }

    void ProgressDialog::start()
    {
        d->spinner->start();
        d->timer.start();
    }

    void ProgressDialog::stop()
    {
        d->spinner->stop();
        d->timer.stop();
        hide();
    }

    QString ProgressDialog::text() const
    {
        return d->label->text();
    }

}
