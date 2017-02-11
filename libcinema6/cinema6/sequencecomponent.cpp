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

#include <cinema6/alignmentview.h>
#include <cinema6/sequence.h>
#include <cinema6/sequencecomponent.h>
#include <cinema6/singleton.h>
#include <utopia2/node.h>

#include <QColor>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QPointer>

namespace CINEMA6
{

    class AminoAlphabetPixmapFactory
    {
    public:
        // Constructor
        AminoAlphabetPixmapFactory()
            : _tileSize(-1)
            {
                this->_colourmap['Z'] = QColor(221, 221, 221);
                this->_colourmap['Y'] = QColor(255, 195, 255);
                this->_colourmap['X'] = QColor(221, 221, 221);
                this->_colourmap['W'] = QColor(255, 195, 255);
                this->_colourmap['V'] = QColor(252, 252, 252);
                this->_colourmap['U'] = QColor(221, 221, 221);
                this->_colourmap['T'] = QColor(195, 255, 195);
                this->_colourmap['S'] = QColor(195, 255, 195);
                this->_colourmap['R'] = QColor(195, 255, 255);
                this->_colourmap['Q'] = QColor(195, 255, 195);
                this->_colourmap['P'] = QColor(255, 212, 195);
                this->_colourmap['O'] = QColor(221, 221, 221);
                this->_colourmap['N'] = QColor(195, 255, 195);
                this->_colourmap['M'] = QColor(252, 252, 252);
                this->_colourmap['L'] = QColor(252, 252, 252);
                this->_colourmap['K'] = QColor(195, 255, 255);
                this->_colourmap['J'] = QColor(221, 221, 221);
                this->_colourmap['I'] = QColor(252, 252, 252);
                this->_colourmap['H'] = QColor(195, 255, 255);
                this->_colourmap['G'] = QColor(255, 212, 195);
                this->_colourmap['F'] = QColor(255, 195, 255);
                this->_colourmap['E'] = QColor(255, 195, 195);
                this->_colourmap['D'] = QColor(255, 195, 195);
                this->_colourmap['C'] = QColor(255, 255, 195);
                this->_colourmap['B'] = QColor(221, 221, 221);
                this->_colourmap['A'] = QColor(252, 252, 252);
                this->_colourmap['-'] = QColor(227, 227, 227);
                this->_colourmap['*'] = QColor(221, 221, 221);
            }

        // Get pixmap
        QPixmap & pixmap(const QChar & code, int tileSize, const QColor & col)
            {
                static QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ-";

                // Recreate if necessary
                if (this->_tileSize != tileSize)
                {
                    // Set size
                    this->_tileSize = tileSize;

                    // Clear old
                    this->_pixmaps.clear();

                    // Create new
                    for (int index = 0; index < alphabet.size(); ++index)
                    {
                        QPixmap pixmap(this->_tileSize, this->_tileSize);
                        QColor col = QColor(0, 0, 0, 0);
                        pixmap.fill(col);
                        QPainter painter(&pixmap);
                        painter.setRenderHint(QPainter::TextAntialiasing);
                        QFont font = painter.font();
                        font.setPixelSize((int) (this->_tileSize * 0.6));
                        painter.setFont(font);
                        QColor penCol = alphabet.at(index) == '-' ? QColor(100, 100, 100) : QColor(50, 50, 50);
                        if (tileSize <= 11)
                        {
                            penCol.setAlphaF((tileSize - 8) / 3.0);
                        }
                        painter.setPen(penCol);
                        painter.drawText(0, 0, this->_tileSize, this->_tileSize + 1, Qt::AlignVCenter | Qt::AlignHCenter, alphabet.at(index));
                        painter.end();
                        this->_pixmaps[alphabet.at(index)] = pixmap;
                    }
                }

                return this->_pixmaps[code];
            }

        // Get colour
        QColor & color(const QChar & code)
            {
                return this->_colourmap[code.toLatin1()];
            }

    private:
        // Cache
        QMap< QChar, QPixmap > _pixmaps;
        int _tileSize;
        QMap< char, QColor > _colourmap;
    };

    class SequenceComponentPrivate
    {
    public:
        SequenceComponentPrivate(Sequence * sequence)
            : sequence(sequence)
            {}

        QPointer< Sequence > sequence;

        QPixmap background;
        Singleton< AminoAlphabetPixmapFactory > pixmapFactory;
    };


    /**
     *  \brief Default component construction.
     */
    SequenceComponent::SequenceComponent(Sequence * sequence)
        : DataComponent(sequence->title()), d(new SequenceComponentPrivate(sequence))
    {
        connect(sequence, SIGNAL(changed()), this, SLOT(dataChanged()));
        dataChanged();
    }

    /**
     *  \brief Destructor.
     */
    SequenceComponent::~SequenceComponent()
    {}

    void SequenceComponent::dataChanged()
    {
        QString sequenceStr(sequence()->toString());
        d->background = QPixmap(sequenceStr.size(), 1);
        QPainter painter(&d->background);
        for (int i = 0; i < sequenceStr.size(); ++i)
        {
            painter.setPen(d->pixmapFactory().color(sequenceStr[i]));
            painter.drawPoint(i, 0);
        }
        update();
    }

    void SequenceComponent::render(QPaintDevice * target, const QPoint & targetOffset, const QRect & sourceRect)
    {
        if (sequence())
        {
            QString sequenceStr(sequence()->toString());
            QRect rect(sourceRect.left(), 0, sourceRect.width(), this->height());
            QPainter painter(target);
            painter.translate(targetOffset);
            painter.translate(-rect.topLeft());
            painter.setClipRect(rect);

            // Find cell sizes
            double unitSize = alignmentView()->unitSizeF();
            int firstUnit = alignmentIndexAt(rect.topLeft());
            int lastUnit = alignmentIndexAt(rect.topRight());
            if (lastUnit >= sequenceStr.size()) { lastUnit = sequenceStr.size() - 1; }
            int unitLeft = rectAt(firstUnit).left();

            painter.save();
            if (unitSize < 1)
            {
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
            }
            painter.scale(unitSize, (double) height());
            painter.drawPixmap(0, 0, d->background);
            painter.restore();

            if (unitSize > 8)
            {
                // Set font to be a little smaller than usual
                painter.setPen(Qt::NoPen);

                QFont font = painter.font();
                font.setPixelSize((int) (unitSize * 0.6));
                painter.setFont(font);
                painter.setPen(Qt::NoPen);

                for (int res = firstUnit; res <= lastUnit && res < sequenceStr.size(); ++res)
                {
                    QColor col = Qt::red;
                    painter.drawPixmap(unitLeft, 0, d->pixmapFactory().pixmap(sequenceStr[res], (int) unitSize, col));

                    // Skip to next position
                    unitLeft += unitSize;
                }
            }
        }
    }

    Sequence * SequenceComponent::sequence() const
    {
        return d->sequence;
    }

    QString SequenceComponent::title() const
    {
        return sequence() ? sequence()->title() : QString();
    }

}
