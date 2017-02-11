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

#ifndef PAGER_H
#define PAGER_H

#include <papyro/config.h>

#include <QFrame>
#include <QSize>
#include <QMap>

class QPixmap;
class QRect;

template< class T > class QList;

namespace Papyro
{

    class PagerPrivate;

    class LIBPAPYRO_API Pager : public QFrame
    {
        Q_OBJECT

    public:
        typedef QMap<int,int> QIntMap;

        Pager(QWidget * parent = 0);
        Pager(Qt::Orientation orientation, int pageCount, QWidget * parent = 0);
        Pager(Qt::Orientation orientation, const QList< QPixmap > & images, QWidget * parent = 0);
        ~Pager();

        // Add / remove pages
        int append(const QPixmap & image = QPixmap());
        const QPixmap & at(int index) const;
        void clear();
        int insert(int index, const QPixmap & image = QPixmap());
        QString labelAt(int index) const;
        bool remove(int index);
        bool rename(int index, const QString & label);
        bool replace(int index, const QPixmap & image = QPixmap());

        // Current state
        int count() const;
        int current() const;

        // Layout
        const QSize & boundingBox() const;
        QSize clampedBoundingBox() const;
        bool drawLabels() const;
        void getContentsMargins(int * left, int * top, int * right, int * bottom) const;
        Qt::Orientation orientation() const;
        void setBoundingBox(const QSize & box);
        void setContentsMargins(int left, int top, int right, int bottom);
        void setDrawLabels(bool draw);
        void setSpread(double spread);
        double spread() const;

    Q_SIGNALS:
        void focusChanged(int index);
        void pageClicked(int index);

    public Q_SLOTS:
        void first();
        void focus(int index);
        void last();
        void next(int delta = 1);
        void previous(int delta = 1);

        void setOrientation(Qt::Orientation orientation);
        void setSpotlights(const QIntMap & spotlights = QIntMap());
        void setAnnotations(const QIntMap & annotations);
        void hideSpotlights(bool hide = true);
        void clearAnnotation(int annotation);

    protected Q_SLOTS:
        void animate();

    protected:
        void mouseReleaseEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);
        void wheelEvent(QWheelEvent * event);

        void drawSearchHits(QPainter *painter, int numHits, QRect rect);
        void drawAnnotationTag(QPainter *painter, QRect rect);


    private:
        PagerPrivate * d;

        void initialise();
        int clampIndex(int index, bool append = false) const;
        bool validIndex(int index, bool append = false) const;

        friend class PagerPrivate;

    }; // class Pager

} // namespace Papyro

#endif
