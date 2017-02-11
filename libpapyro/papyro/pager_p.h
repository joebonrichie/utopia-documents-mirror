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

#ifndef PAGER_P_H
#define PAGER_P_H

#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QTime>
#include <QTimer>

class QVBoxLayout;
class QScrollBar;

namespace Papyro
{

    class Pager;
    class PagerPrivate;

    class PagerPrivate : public QObject
    {
        Q_OBJECT

    public:
        typedef QMap<int,int> QIntMap;

        PagerPrivate(QObject * parent, Pager * pager, Qt::Orientation orientation);

        Pager * pager;

        // Page Images
        QList< QPixmap > images;
        QList< QString > labels;

        // Current state
        int currentIndex;

        // GUI state
        double guiIndex;
        QTime whenFocused;
        double oldGuiIndex;
        QMap< int, double > indexPreModifiers;
        QMap< int, double > indexPostModifiers;
        QMap< int, int > searchHits;
        QMap< int, int > hasAnnotation;
        bool spotlightsHidden;

        QMap< int, QTime > transitionTimes;
        QTimer timer;

        // Layout
        QSize box;
        double spread;
        int marginLeft;
        int marginTop;
        int marginRight;
        int marginBottom;
        bool drawLabels;
        Qt::Orientation orientation;
        QVBoxLayout * layout;
        QScrollBar * scrollBar;

        void first(bool user = false);
        void focus(int index, bool user = false);
        void last(bool user = false);
        void next(int delta = 1, bool user = false);
        void previous(int delta = 1, bool user = false);
        QSize clampedBoundingBox(bool ignoreScrollBars = false);
        void updateScrollBar();

    public slots:
        void onScrollBarValueChanged(int value);

    }; // class PagerPrivate

} // namespace Papyro

#endif
