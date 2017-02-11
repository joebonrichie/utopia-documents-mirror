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

#ifndef GRAFFITI_SECTIONS_H
#define GRAFFITI_SECTIONS_H

#include <QList>
#include <QObject>

namespace Graffiti
{

    typedef struct {
        double offset;
        double size;
    } Section;




    class SectionsPrivate;
    class Sections : public QObject
    {
        Q_OBJECT

    public:
        Sections(QObject * parent = 0);
        Sections(double length, QObject * parent = 0);
        Sections(double from, double to, QObject * parent = 0);
        ~Sections();

        int addBoundary(double offset);
        QList< double > boundaries() const;
        double boundaryAt(int i);
        int count() const;
        int find(double offset) const;
        double firstBoundary() const;
        double lastBoundary() const;
        int moveBoundary(int i, double offset);
        int next(double offset) const;
        int previous(double offset) const;
        void removeBoundary(int i);
        void removeSection(int i);
        void reset(double length = 0);
        void reset(double from, double to);
        void resizeSection(int i, double size);
        Section sectionAt(int i) const;
        void setBoundaries(const QList< double > & boundaries);
        double size() const;
        void unsplit(int from, int to);

    signals:
        void boundaryAdded(int boundary);
        void boundaryRemoved(int boundary);
        void boundariesChanged();

    protected:
        SectionsPrivate * d;

    }; // class Sections

} // namespace Graffiti

#endif // GRAFFITI_SECTIONS_H
