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

#include <graffiti/sections.h>
#include <graffiti/sections_p.h>

#include <QDebug>

namespace Graffiti
{

    SectionsPrivate::SectionsPrivate(QObject * parent)
        : QObject(parent)
    {}

    SectionsPrivate::~SectionsPrivate()
    {}




    Sections::Sections(QObject * parent)
        : QObject(parent), d(new SectionsPrivate(this))
    {
        reset(0);
    }

    Sections::Sections(double length, QObject * parent)
        : QObject(parent), d(new SectionsPrivate(this))
    {
        reset(length);
    }

    Sections::Sections(double from, double to, QObject * parent)
        : QObject(parent), d(new SectionsPrivate(this))
    {
        reset(from, to);
    }

    Sections::~Sections()
    {}

    int Sections::addBoundary(double offset)
    {
        std::pair< std::set< double >::iterator, bool > inserted = d->boundaries.insert(offset);
        if (inserted.second) {
            int idx = 0;
            while (inserted.first != d->boundaries.begin()) {
                --inserted.first;
                ++idx;
            }
            emit boundaryAdded(idx);
            emit boundariesChanged();
            return idx;
        }

        return -1;
    }

    QList< double > Sections::boundaries() const
    {
        QList< double > boundaries;
        foreach (double boundary, d->boundaries) {
            boundaries.push_back(boundary);
        }
        return boundaries;
    }

    double Sections::boundaryAt(int i)
    {
        std::set< double >::const_iterator iter(d->boundaries.begin());
        while (i > 0) { --i; ++iter; }
        return *iter;
    }

    int Sections::count() const
    {
        if (d->boundaries.size() > 1) {
            return d->boundaries.size() - 1;
        } else {
            return 0;
        }
    }

    int Sections::find(double offset) const
    {
        std::set< double >::const_iterator first(d->boundaries.begin());
        std::set< double >::const_iterator next(d->boundaries.upper_bound(offset));
        double last = lastBoundary();
        if (next == first) { // offset before first section
            return -1;
        } else if (offset > last) { // offset after last section
            return -1;
        } else if (count() == 0) { // empty sections
            return -1;
        } else { // otherwise
            if (last == offset) { --next; }
            --next;
            int i = 0;
            while (first != next) { ++i, ++first; }
            return i;
        }
    }

    double Sections::firstBoundary() const
    {
        if (d->boundaries.empty()) {
            return 0.0;
        } else {
            return *d->boundaries.begin();
        }
    }

    double Sections::lastBoundary() const
    {
        if (d->boundaries.empty()) {
            return 0.0;
        } else {
            return *--d->boundaries.end();
        }
    }

    int Sections::moveBoundary(int i, double offset)
    {
        if (i >= 0 && i < count()) {
            std::set< double >::iterator found(d->boundaries.begin());
            while (i > 0) { --i, ++found; }
            d->boundaries.erase(found);
            std::pair< std::set< double >::iterator, bool > moved = d->boundaries.insert(offset);
            int j = 0;
            for (found = d->boundaries.begin(); found != moved.first; ++found) { ++j; }
            emit boundariesChanged();
            return j;
        } else {
            return -1;
        }
    }

    int Sections::next(double offset) const
    {
        std::set< double >::const_iterator first(d->boundaries.begin());
        std::set< double >::const_iterator end(d->boundaries.end());
        std::set< double >::const_iterator found(d->boundaries.upper_bound(offset));
        if (found == end) { // offset after last section
            return -1;
        } else if (found == first) { // offset before first section
            return 0;
        } else if (count() == 0) { // empty sections
            return -1;
        } else { // otherwise
            int i = 0;
            while (first != found) { ++i, ++first; }
            return i;
        }
    }

    int Sections::previous(double offset) const
    {
        std::set< double >::const_iterator first(d->boundaries.begin());
        std::set< double >::const_iterator end(d->boundaries.end());
        std::set< double >::const_iterator found(d->boundaries.lower_bound(offset));
        if (found == end) { // offset after last section
            return d->boundaries.size() - 1;
        } else if (found == first) { // offset before first section
            return -1;
        } else if (count() == 0) { // empty sections
            return -1;
        } else { // otherwise
            --found;
            int i = 0;
            while (first != found) { ++i, ++first; }
            return i;
        }
    }

    void Sections::removeBoundary(int i)
    {
        if (i >= 0 && d->boundaries.size() > 0) {
            int idx = i;
            std::set< double >::iterator found(d->boundaries.begin());
            while (idx > 0) { ++found, --idx; }
            if (found != d->boundaries.end()) {
                d->boundaries.erase(found);
                emit boundaryRemoved(i);
                emit boundariesChanged();
            }
        }
    }

    void Sections::removeSection(int i)
    {
        resizeSection(i, 0);
    }

    void Sections::reset(double length)
    {
        reset(0.0, length);
    }

    void Sections::reset(double from, double to)
    {
        d->boundaries.clear();
        d->boundaries.insert(from);
        if (from != to) {
            d->boundaries.insert(to);
        }
    }

    void Sections::resizeSection(int i, double size)
    {
        if (i >= 0 && i < count()) {
            std::set< double >::iterator found(d->boundaries.begin());
            while (i > 0) { --i, ++found; }
            double offset = *found;
            double old_size = *++found - offset;
            double delta = size - old_size;
            std::set< double > updated;
            while (found != d->boundaries.end()) {
                updated.insert(*found + delta);
                d->boundaries.erase(found++);
            }
            d->boundaries.insert(updated.begin(), updated.end());
        }
    }

    Section Sections::sectionAt(int i) const
    {
        static const Section null = {0, 0};
        if (i >= 0 && i < count()) {
            std::set< double >::const_iterator found(d->boundaries.begin());
            while (i > 0) { --i, ++found; }
            double offset = *found;
            double width = *++found - offset;
            Section section = {offset, width};
            return section;
        } else {
            return null;
        }
    }

    void Sections::setBoundaries(const QList< double > & boundaries)
    {
        if (boundaries.isEmpty()) {
            reset();
        } else {
            std::set< double > ordered(boundaries.begin(), boundaries.end());
            std::set< double >::const_iterator first(ordered.begin());
            std::set< double >::const_iterator last(--ordered.end());
            std::set< double >::const_iterator next(first);
            reset(*first, *last);
            while (++next != last) {
                addBoundary(*next);
            }
        }
    }

    double Sections::size() const
    {
        if (count() > 0) {
            std::set< double >::iterator first(d->boundaries.begin());
            std::set< double >::iterator last(--d->boundaries.end());
            return *last - *first;
        } else {
            return 0;
        }
    }

    void Sections::unsplit(int from, int to)
    {
        if (from >= 0 && from < to && to < count()) {
            int idx = 0;
            std::set< double >::iterator found(d->boundaries.begin());
            while (from > 0) { --from, --to, ++found, ++idx; }
            ++found;
            while (to > 0) {
                d->boundaries.erase(found++);
                emit boundaryRemoved(idx + 1);
                --to;
                ++idx;
            }
            emit boundariesChanged();
        }
    }

} // namespace Graffiti
