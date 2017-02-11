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

#ifndef PAPYRO_PROGRESSLOZENGE_H
#define PAPYRO_PROGRESSLOZENGE_H

#include <QColor>
#include <QString>
#include <QWidget>

class QPaintEvent;

namespace Papyro
{

    class ProgressLozengePrivate;

    class ProgressLozenge : public QWidget
    {
        Q_OBJECT

    public:
        ProgressLozenge(QString label, QColor color = QColor(0, 0, 0), QWidget * parent = 0);
        virtual ~ProgressLozenge();

        QColor color() const;
        QString label() const;
        int period() const;
        void setPeriod(int msecs);
        virtual QSize sizeHint() const;

    public Q_SLOTS:
        void setColor(QColor color);
        void setLabel(QString label);
        void setProgress(qreal progress);
        void start();
        void stop();

    protected:
        void paintEvent(QPaintEvent * event);

    private:
        ProgressLozengePrivate * d;

    };

}

#endif // PAPYRO_PROGRESSLOZENGE_H
