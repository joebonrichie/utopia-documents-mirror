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

#include <cinema6/sequence.h>
#include <cinema6/singleton.h>

#include <utopia2/node.h>

#include <QColor>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QVector>

namespace CINEMA6
{

    class SequencePrivate
    {
    public:
        SequencePrivate(Sequence * sequence, Utopia::Node * model)
            : sequence(sequence), model(model)
            {}

        // Model
        Sequence * sequence;
        Utopia::Node * model;

        // Cache
        QString sequenceStr;
        QVector< Utopia::Node * > residueNodes;
        QMap< int, int > gapMap;

        void recache()
            {
                Utopia::Node * c_Gap = Utopia::UtopiaDomain.term("Gap");
                Utopia::Node * p_size = Utopia::UtopiaDomain.term("size");

                int sequenceIndex = 0;
                int alignmentIndex = 0;
                gapMap.clear();
                sequenceStr = "";
                residueNodes.clear();
                gapMap[0] = 0;

                Utopia::Node::relation::iterator iter = model->relations(Utopia::UtopiaSystem.hasPart).begin();
                Utopia::Node::relation::iterator end = model->relations(Utopia::UtopiaSystem.hasPart).end();
                for (; iter != end; ++iter)
                {
                    // cache gaps
                    Utopia::Node::relation::const_iterator gap_iter = (*iter)->relations(~Utopia::UtopiaSystem.annotates).begin();
                    Utopia::Node::relation::const_iterator gap_end = (*iter)->relations(~Utopia::UtopiaSystem.annotates).end();
                    for (; gap_iter != gap_end; ++gap_iter)
                    {
                        if ((*gap_iter)->type() == c_Gap && (*gap_iter)->attributes.exists(p_size))
                        {
                            int size = (*gap_iter)->attributes.get(p_size).toInt();
                            alignmentIndex += size;
                            gapMap[sequenceIndex] = alignmentIndex;
                            sequenceStr += QString(size, '-');
                        }
                    }

                    sequenceStr += (*iter)->type()->attributes.get(Utopia::UtopiaDomain.term("code")).toString();
                    residueNodes.append(*iter);

                    ++sequenceIndex;
                    ++alignmentIndex;
                }

                emit sequence->changed();
            }
    };



    Sequence::Sequence(Utopia::Node * sequence)
        : AbstractSequence(), d(new SequencePrivate(this, sequence))
    {
        d->recache();
    }

    Sequence::~Sequence()
    {}

    int Sequence::gap(int sequenceIndex) const
    {
        if (sequenceIndex >= 0 && sequenceIndex < d->residueNodes.size())
        {
            static Utopia::Node * c_Gap = Utopia::UtopiaDomain.term("Gap");
            static Utopia::Node * p_size = Utopia::UtopiaDomain.term("size");

            Utopia::Node * node = d->residueNodes.at(sequenceIndex);
            int gapSize = 0;

            Utopia::Node::relation::const_iterator gap_iter = node->relations(~Utopia::UtopiaSystem.annotates).begin();
            Utopia::Node::relation::const_iterator gap_end = node->relations(~Utopia::UtopiaSystem.annotates).end();
            for (; gap_iter != gap_end; ++gap_iter)
            {
                if ((*gap_iter)->type() == c_Gap && (*gap_iter)->attributes.exists(p_size))
                {
                    gapSize += (*gap_iter)->attributes.get(p_size).toInt();
                }
            }
            return gapSize;
        }
        else
        {
            return 0;
        }
    }

    int Sequence::mapFromSequence(int sequenceIndex) const
    {
        QMap< int, int >::iterator map_iter = d->gapMap.lowerBound(sequenceIndex);
        if (map_iter == d->gapMap.end())
        {
            // FIXME check that this is the only error case worth caring about
            return -1;
        }
        else
        {
            int nextSequenceIndex = map_iter.key();
            int alignmentIndex = map_iter.value();
            return alignmentIndex + (sequenceIndex - nextSequenceIndex);
        }
    }

    int Sequence::mapToSequence(int index) const
    {
        QMap< int, int >::iterator map_begin = d->gapMap.begin();
        QMap< int, int >::iterator map_iter = d->gapMap.begin();
        QMap< int, int >::iterator map_end = d->gapMap.end();
        for (; map_iter != map_end; ++map_iter)
        {
            if (map_iter.value() >= index)
            {
                break;
            }
        }
        if (map_iter == map_end)
        {
            if (map_iter == map_begin)
            {
                return -1;
            }
            --map_iter;
            int sequenceIndex = map_iter.key();
            int alignmentIndex = map_iter.value();
            sequenceIndex = sequenceIndex + (index - alignmentIndex);
            return d->residueNodes.size() > sequenceIndex ? sequenceIndex : -1;
        }
        int sequenceIndex = map_iter.key();
        int alignmentIndex = map_iter.value();
        if (index == alignmentIndex)
        {
            return sequenceIndex;
        }
        int gapSize = gap(sequenceIndex);
        if (index >= (alignmentIndex - gapSize))
        {
            return -1;
        }
        else
        {
            return sequenceIndex - (alignmentIndex - index - gapSize);
        }
    }

    void Sequence::setGap(int sequenceIndex, int newGap)
    {
        static Utopia::Node * c_Gap = Utopia::UtopiaDomain.term("Gap");
        static Utopia::Node * p_size = Utopia::UtopiaDomain.term("size");

        int oldGap = gap(sequenceIndex);
        if (sequenceIndex == -1)
        {
            QMap< int, int >::iterator map_iter = d->gapMap.begin();
            QMap< int, int >::iterator map_end = d->gapMap.end();
            for (; map_iter != map_end; ++map_iter)
            {
                if (map_iter.value() >= sequenceIndex)
                {
                    sequenceIndex = map_iter.key();
                    break;
                }
            }
        }
        Utopia::Node * node = d->residueNodes.at(sequenceIndex);
        Utopia::Node::relation::iterator gap_iter = node->relations(~Utopia::UtopiaSystem.annotates).begin();
        Utopia::Node::relation::iterator gap_end = node->relations(~Utopia::UtopiaSystem.annotates).end();
        int delta = newGap - oldGap;
        if (delta != 0)
        {
            if (gap_iter == gap_end && delta > 0)
            {
                Utopia::Node * gap_annotation = d->model->create(c_Gap);
                gap_annotation->relations(Utopia::UtopiaSystem.annotates).append(node);
                gap_annotation->attributes.set(p_size, delta);
            }


            for (; gap_iter != gap_end; ++gap_iter)
            {
                if ((*gap_iter)->type() == c_Gap && (*gap_iter)->attributes.exists(p_size))
                {
                    int size = (*gap_iter)->attributes.get(p_size).toInt();
                    if (size + delta < 0)
                    {
                        size = 0;
                        delta += size;
                        (*gap_iter)->attributes.set(p_size, size);
                    }
                    else
                    {
                        size += delta;
                        (*gap_iter)->attributes.set(p_size, size);
                        break;
                    }
                }
            }

            d->recache();
        }
    }

    QString Sequence::title() const
    {
        static Utopia::Node* p_title = Utopia::UtopiaDomain.term("title");
        return d->model ? d->model->attributes.get(p_title).toString() : QString();
    }

    QString Sequence::toString() const
    {
        return d->sequenceStr;
    }

}
