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

#include <papyro/pager_p.h>
#include <papyro/pager.h>
#include <math.h>

#include <QApplication>
#include <QPixmap>
#include <QList>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QRectF>
#include <QScrollBar>
#include <QSize>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

#include <QDebug>

#ifndef _PI
#define _PI 3.1415926535897932384626433832795028841968
#endif

namespace Papyro
{

    /// PagerPrivate //////////////////////////////////////////////////////////////////////////////

    PagerPrivate::PagerPrivate(QObject * parent, Pager * pager, Qt::Orientation orientation)
        : QObject(parent), pager(pager), orientation(orientation)
    {}

    void PagerPrivate::first(bool user)
    {
        focus(0, user);
    }

    void PagerPrivate::focus(int index, bool user)
    {
        index = pager->clampIndex(index);
        if (index != currentIndex) {
            currentIndex = index;
            whenFocused.start();
            oldGuiIndex = guiIndex;
            timer.start();

            emit pager->focusChanged(index);
            if (user) {
                emit pager->pageClicked(index);
            }
        }
    }

    void PagerPrivate::last(bool user)
    {
        focus(pager->count() - 1, user);
    }

    void PagerPrivate::next(int delta, bool user)
    {
        focus(pager->current() + delta, user);
    }

    void PagerPrivate::previous(int delta, bool user)
    {
        focus(pager->current() - delta, user);
    }

    QSize PagerPrivate::clampedBoundingBox(bool ignoreScrollBars)
    {
        QSize requestedSize = pager->boundingBox();
        int maxHeight = pager->height() - marginTop - marginBottom - (drawLabels ? 12 : 0);
        if (!ignoreScrollBars && orientation == Qt::Horizontal && scrollBar->isVisible()) { maxHeight -= scrollBar->height(); }
        int maxWidth = pager->width() - marginLeft - marginRight;
        if (!ignoreScrollBars && orientation == Qt::Vertical && scrollBar->isVisible()) { maxWidth -= scrollBar->width(); }

        if (!requestedSize.isValid())
        {
            requestedSize = QSize(1, 1);
        }

        requestedSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
        return requestedSize;
    }

    void PagerPrivate::updateScrollBar()
    {
        // Update range
        scrollBar->setRange(0, images.size() - 1);

        // Hide if not needed
        bool fits = true;
        QSize box(clampedBoundingBox(true));
        if (orientation == Qt::Vertical) {
            int contentHeight = box.height() * images.size() + marginTop + marginBottom + (drawLabels ? 12 : 0);
            fits = contentHeight <= pager->height();
        } else {
            int contentWidth= box.width() * images.size() + marginLeft + marginRight;
            fits = contentWidth <= pager->width();
        }
        scrollBar->setVisible(!fits && images.size() > 1);
    }

    void PagerPrivate::onScrollBarValueChanged(int value)
    {
        focus(value, true);
    }

    /// Pager /////////////////////////////////////////////////////////////////////////////////////

    Pager::Pager(QWidget * parent)
        : QFrame(parent), d(new PagerPrivate(this, this, Qt::Horizontal))
    {
        initialise();
    }

    Pager::Pager(Qt::Orientation orientation, int count, QWidget * parent)
        : QFrame(parent), d(new PagerPrivate(this, this, orientation))
    {
        initialise();

        // Add placeholders for count pages
        for (int i = 0; i < count; ++i) {
            append();
        }
    }

    Pager::Pager(Qt::Orientation orientation, const QList< QPixmap > & images, QWidget * parent)
        : QFrame(parent), d(new PagerPrivate(this, this, orientation))
    {
        initialise();

        // Add images
        QListIterator< QPixmap > iter(images);
        while (iter.hasNext())
        {
            append(iter.next());
        }
    }

    Pager::~Pager()
    {}

    bool nearAsDamnIt(double d, int i)
    {
        return d + 0.01 > (double) i && d - 0.01 < (double) i;
    }

    void Pager::animate()
    {
        bool finished = true;

        // Do any index modifiers need changing?
        for (int i = 0; i < count(); ++i)
        {
            double preModifier = d->indexPreModifiers.value(i, 0.0);

            // Does the pre modifier need changing?
            if (!nearAsDamnIt(preModifier, 0))
            {
                // what reduction should be actioned?
                double preOffset = preModifier / 30.0;

                // reduce modifier
                d->indexPreModifiers.insert(i, preModifier - preOffset);

                finished = false;
            }
            else
            {
                // Remove modifier
                d->indexPreModifiers.remove(i);

                // Start transition timer if one exists
                if (d->transitionTimes.contains(i) && d->transitionTimes[i].isNull())
                {
                    d->transitionTimes[i].start();
                    finished = false;
                }
            }

            double postModifier = d->indexPostModifiers.value(i, 0.0);

            // Does the post modifier need changing?
            if (!nearAsDamnIt(postModifier, 0))
            {
                // what reduction should be actioned?
                double postOffset = postModifier / 30.0;

                // reduce modifier
                d->indexPostModifiers.insert(i, postModifier - postOffset);

                finished = false;
            }
            else
            {
                // Remove modifier
                d->indexPostModifiers.remove(i);
            }

            // Remove defunct transition timers
            if (d->transitionTimes.contains(i) && !d->transitionTimes[i].isNull())
            {
                if (d->transitionTimes[i].elapsed() > 500)
                {
                    d->transitionTimes.remove(i);
                }
                else
                {
                    finished = false;
                }
            }
        }

        // Does the guiIndex need changing?
        if (!d->whenFocused.isNull())
        {
            // Distance to travel since last focus
            double distance = d->currentIndex - d->oldGuiIndex;

            // Expected journey time (msecs)
            double allowance = 1000.0 * qMin(0.25 * sqrt(qAbs(distance)), 1.4);

            // Actual progress (msecs)
            int elapsed = d->whenFocused.elapsed();

            if (elapsed > allowance)
            {
                d->whenFocused = QTime();
                d->guiIndex = d->currentIndex;
            }
            else
            {
                // Calculated progress
                double progress = sin(_PI * elapsed / (allowance * 2.0));

                d->guiIndex = d->oldGuiIndex + progress * distance;
                finished = false;
            }
        }

        if (finished) d->timer.stop();

        update();
    }

    int Pager::append(const QPixmap & image)
    {
        d->images.append(image);
        d->updateScrollBar();
        d->labels.append(QString());
        d->transitionTimes[d->images.size() - 1].start();
        // Deal with null images FIXME
        d->timer.start();
        update();
        return d->images.size() - 1;
    }

    const QPixmap & Pager::at(int index) const
    {
        return d->images.at(clampIndex(index));
    }

    const QSize & Pager::boundingBox() const
    {
        return d->box;
    }

    QSize Pager::clampedBoundingBox() const
    {
        return d->clampedBoundingBox();
    }

    int Pager::clampIndex(int index, bool append) const
    {
        return qBound(0, index, append ? d->images.size() : d->images.size() - 1);
    }

    void Pager::clear()
    {
        d->currentIndex = 0;
        d->guiIndex = 0.0;
        d->whenFocused = QTime();
        d->oldGuiIndex = 0.0;
        d->images.clear();
        d->updateScrollBar();
        d->labels.clear();
        d->indexPreModifiers.clear();
        d->indexPostModifiers.clear();
        d->searchHits.clear();
        d->hasAnnotation.clear();
        d->transitionTimes.clear();
        d->timer.stop();
        update();
    }

    void Pager::clearAnnotation(int annotation)
    {
        d->hasAnnotation.remove(annotation);
        update();
    }

    int Pager::count() const
    {
        return d->images.count();
    }

    int Pager::current() const
    {
        return d->currentIndex;
    }

    void Pager::drawAnnotationTag(QPainter *painter, QRect rect)
    {
        painter->save();
        int cornerSize = 14;// rect.width() / 8;
        QPainterPath cornerPath;
        cornerPath.moveTo(rect.topRight() - QPoint(cornerSize-1, 0));
        cornerPath.lineTo(rect.topRight() + QPoint(1,0));
        cornerPath.lineTo(rect.topRight() + QPoint(1, cornerSize));
        cornerPath.lineTo(rect.topRight() - QPoint(cornerSize-1, 0));

        painter->setBrush(QColor(255,0,0,80));
        painter->setPen(Qt::NoPen);
        painter->drawPath(cornerPath);
        painter->restore();
    }

    bool Pager::drawLabels() const
    {
        return d->drawLabels;
    }

    void Pager::drawSearchHits(QPainter *painter, int numHits, QRect rect)
    {
        painter->save();
        painter->setCompositionMode(QPainter::CompositionMode_Multiply);

        painter->setBrush(QColor(0,0,0,100));
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);

        painter->restore();

        if (numHits > 0)
        {
            painter->save();

            QFont font = painter->font();
            font.setPointSize(9);
            painter->setFont(font);

            QFontMetrics fm(painter->font());
            QString hitcount = QString::number(numHits);

            QSize numSize = fm.boundingRect(hitcount).size();
            double radius = 1 + numSize.height() / 2.0;
            numSize += QSize(numSize.height(), 0);
            QRect numRect(0, 0, numSize.width() + 2, numSize.height() + 2);

            int margin = 3;

            numRect.translate(rect.bottomRight() - numRect.bottomRight() - QPoint(margin,margin));

            painter->setBrush(Qt::yellow);
            painter->setPen(QColor(140, 140, 0));
            painter->drawRoundedRect(numRect, radius, radius);

            painter->setPen(Qt::black);
            painter->drawText(numRect, Qt::AlignCenter, hitcount);

            painter->restore();
        }
    }

    void Pager::first()
    {
        d->focus(0);
    }

    void Pager::focus(int index)
    {
        d->focus(index);
    }

    void Pager::getContentsMargins(int * left, int * top, int * right, int * bottom) const
    {
        *left = d->marginLeft;
        *top = d->marginTop;
        *right = d->marginRight;
        *bottom = d->marginBottom;
    }

    void Pager::hideSpotlights(bool hide)
    {
        d->spotlightsHidden = hide;
        update();
    }

    void Pager::initialise()
    {
        d->currentIndex = 0;
        d->guiIndex = 0.0;
        d->timer.setInterval(30);
        d->spread = 0.1;
        d->marginTop = d->marginRight = d->marginBottom = d->marginLeft = 10;
        d->spotlightsHidden = false;
        d->drawLabels = false;

        // Scroll Bar
        d->layout = new QVBoxLayout(this);
        d->scrollBar = new QScrollBar();
        d->layout->setContentsMargins(0, 0, 0, 0);
        d->layout->setSpacing(0);
        d->layout->addWidget(d->scrollBar);
        setOrientation(d->orientation);
        connect(d->scrollBar, SIGNAL(valueChanged(int)), d, SLOT(onScrollBarValueChanged(int)));
        connect(this, SIGNAL(focusChanged(int)), d->scrollBar, SLOT(setValue(int)));

        // Animation timeout
        connect(&d->timer, SIGNAL(timeout()), this, SLOT(animate()));
    }

    int Pager::insert(int index, const QPixmap & image)
    {
        index = clampIndex(index, true);

        d->images.insert(index, image);
        d->updateScrollBar();
        d->labels.insert(index, QString());
        // Deal with null images FIXME

        // Modify current index if necessary
        if (index <= d->currentIndex && d->images.size() > 0)
        {
            d->currentIndex += 1;
            d->guiIndex += 1.0;
        }

        // Modify index modifiers
        QMap< int, double > updatedModifiers;
        QMutableMapIterator< int, double > iter(d->indexPreModifiers);
        iter.toBack();
        while (iter.hasPrevious())
        {
            iter.previous();
            int targetIndex = iter.key();
            if (targetIndex >= index)
            {
                double modifier = iter.value();
                iter.remove();
                updatedModifiers[targetIndex + 1] = modifier;
            }
            else
            {
                break;
            }
        }
        d->indexPreModifiers.unite(updatedModifiers);

        updatedModifiers.clear();
        iter = d->indexPostModifiers;
        iter.toBack();
        while (iter.hasPrevious())
        {
            iter.previous();
            int targetIndex = iter.key();
            if (targetIndex >= index)
            {
                double modifier = iter.value();
                iter.remove();
                updatedModifiers[targetIndex + 1] = modifier;
            }
            else
            {
                break;
            }
        }
        d->indexPostModifiers.unite(updatedModifiers);

        // Modify transition times
        QMap< int, QTime > updatedTimes;
        QMutableMapIterator< int, QTime > t_iter(d->transitionTimes);
        t_iter.toBack();
        while (t_iter.hasPrevious())
        {
            t_iter.previous();
            int targetIndex = t_iter.key();
            if (targetIndex >= index)
            {
                QTime time = t_iter.value();
                t_iter.remove();
                updatedTimes[targetIndex + 1] = time;
            }
            else
            {
                break;
            }
        }
        d->transitionTimes.unite(updatedTimes);

        // Modify Search Hits
        QMap< int, int > updatedSearchHits;
        QMutableMapIterator< int, int > iter2(d->searchHits);
        iter2.toBack();
        while (iter2.hasPrevious())
        {
            iter2.previous();
            int targetIndex = iter2.key();
            if (targetIndex >= index)
            {
                int hits = iter2.value();
                iter2.remove();
                updatedSearchHits[targetIndex + 1] = hits;
            }
            else
            {
                break;
            }
        }
        d->searchHits.unite(updatedSearchHits);

        // Modify Annotations
        QMap< int, int > updatedHasAnnotation;
        iter2 = d->hasAnnotation;
        iter2.toBack();
        while (iter2.hasPrevious())
        {
            iter2.previous();
            int targetIndex = iter2.key();
            if (targetIndex >= index)
            {
                int hits = iter2.value();
                iter2.remove();
                updatedHasAnnotation[targetIndex + 1] = hits;
            }
            else
            {
                break;
            }
        }
        d->hasAnnotation.unite(updatedHasAnnotation);

        // Mask newly insert page
        d->indexPreModifiers[index] = -1.0;

        // Set up transition
        d->transitionTimes[index];

        d->timer.start();
        update();
        return index;
    }

    QString Pager::labelAt(int index) const
    {
        return d->labels.at(clampIndex(index));
    }

    void Pager::last()
    {
        d->last();
    }

    void Pager::mouseReleaseEvent(QMouseEvent * event)
    {
        // Only if left mouse button
        if (event->button() == Qt::LeftButton)
        {
            QPoint pos = event->pos();

            // Within which bounding box?
            QSize box = clampedBoundingBox();
            double lower = (width() - d->marginLeft - d->marginRight - box.width()) / 2.0;
            double upper = (count() - 1) * box.width() - lower;
            double clamp = qBound(lower, d->guiIndex * box.width(), upper);
            QPoint normPos = pos + QPoint((int) clamp + (box.width() - width()) / 2, -d->marginTop);
            int index = normPos.x() / box.width();

            // Valid bounding box?
            if (index >= 0 && index < count())
            {
                // Within bounding box?
                QPoint localPos = normPos - QPoint(index * box.width(), 0);
                QSize localSize;
                if (d->images.at(index).isNull()) {
                    localSize = box;
                } else {
                    localSize = d->images.at(index).size();
                    localSize.scale(box, Qt::KeepAspectRatio);
                }
                QRect localBounds((box.width() - localSize.width()) / 2, box.height() - localSize.height(), localSize.width(), localSize.height());
                if (localBounds.contains(localPos)) {
                    d->focus(index, true);
                }
            }
        }
    }

    void Pager::next(int delta)
    {
        d->next(delta);
    }

    Qt::Orientation Pager::orientation() const
    {
        return d->orientation;
    }

    void Pager::paintEvent(QPaintEvent * event)
    {
        QFrame::paintEvent(event);

        if (d->images.size() == 0) { return; }

        // Maximum bounding box of a single page image
        QSize box = clampedBoundingBox();

        // In parametric equation of a circle, the offset from the centre is:
        //     x = a sin t
        // Spread determines how the index of page maps to the above t.

        double h = (double) box.height();
        double w = (double) box.width();
        double a = (width() - w - d->marginLeft - d->marginRight - (d->orientation == Qt::Vertical && d->scrollBar->isVisible() ? d->scrollBar->width() : 0)) / 2.0;
        double _1 = asin(w/a);
        double s = spread();
        double t1 = _1 + s * ((_PI / 2) - _1);

        int limitGrace = 5;
        //int upperLimit = (int) (d->guiIndex + _PI / (2.0 * t1)) + limitGrace + 1; // FIXME shuffle
        int upperLimit = (int) (d->guiIndex + width() / w + 1);
        //int lowerLimit = (int) (d->guiIndex - _PI / (2.0 * t1)) - limitGrace; // FIXME shuffle
        int lowerLimit = (int) (d->guiIndex - width() / w - 1);
        QPainter painter(this);
        painter.setClipRect(rect().adjusted(0, 1, -1, 0));
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        QFont f(painter.font());
        f.setPixelSize(9);
        painter.setFont(f);
        painter.translate(width() / 2, 0);
        painter.translate(-box.width() / 2, 0);
        // FIXME shuffle
        double lower = a;
        double upper = (count() - 1) * w - lower;
        double clamp = qBound(lower, d->guiIndex * w, upper);
        painter.translate(-clamp, 0);
        // end FIXME shuffle
        painter.translate(0, d->marginTop);

        painter.setPen(Qt::NoPen);
        if (isActiveWindow())
        {
            painter.setPen(QApplication::palette().highlight().color());
            painter.setBrush(QApplication::palette().highlight());
        }
        else
        {
            int gray = qGray(QApplication::palette().highlight().color().rgb());
            painter.setPen(QColor(gray, gray, gray));
            painter.setBrush(QColor(gray, gray, gray));
        }

        double radius = 4.0;
        double transientWidth = 0.0;
        double transientHeight = 0.0;
        double transientLabelWidth = 0.0;
        double transientLabelBottom = 0.0;
        if (d->images.size() == 1)
        {
            QSize imageSize = d->images[0].size();
            imageSize.scale(box, Qt::KeepAspectRatio);
            transientWidth = imageSize.width();
            transientHeight = imageSize.height();
            if (d->drawLabels && !labelAt(0).isEmpty())
            {
                QRect labelRect(QPoint((box.width() - transientWidth) / 2.0, radius), QPoint(box.width() - (box.width() - transientWidth) / 2.0, 12 + radius));
                QString elidedLabel = painter.fontMetrics().elidedText(labelAt(0), Qt::ElideMiddle, labelRect.width());
                QRect bb(painter.fontMetrics().boundingRect(labelRect, Qt::AlignCenter, elidedLabel));
                transientLabelWidth = (double) bb.width();
                transientLabelBottom = (double) bb.bottom();
            }
        }
        else
        {
            int previousIndex = clampIndex((int) floor(d->guiIndex));
            int nextIndex = clampIndex((int) ceil(d->guiIndex));
            QSize previousSize = box;
            if (!d->images[previousIndex].isNull())
            {
                previousSize = d->images[previousIndex].size();
                previousSize.scale(box, Qt::KeepAspectRatio);
            }
            QSize nextSize = box;
            if (!d->images[nextIndex].isNull())
            {
                nextSize = d->images[nextIndex].size();
                nextSize.scale(box, Qt::KeepAspectRatio);
            }
            transientWidth = previousSize.width() + (nextSize.width() - previousSize.width()) * (d->guiIndex - previousIndex);
            transientHeight = previousSize.height() + (nextSize.height() - previousSize.height()) * (d->guiIndex - previousIndex);
            if (d->drawLabels && !labelAt(previousIndex).isEmpty() && !labelAt(nextIndex).isEmpty())
            {
                double previousLabelWidth = 0.0;
                double previousLabelBottom = 0.0;
                {
                    QRect labelRect(0, box.height() + radius, previousSize.width(), 12);
                    QString elidedLabel = painter.fontMetrics().elidedText(labelAt(previousIndex), Qt::ElideMiddle, labelRect.width());
                    QRect bb(painter.fontMetrics().boundingRect(labelRect, Qt::AlignCenter, elidedLabel));
                    previousLabelWidth = (double) bb.width();
                    previousLabelBottom = (double) bb.bottom();
                }
                double nextLabelWidth = 0.0;
                double nextLabelBottom = 0.0;
                {
                    QRect labelRect(0, box.height() + radius, nextSize.width(), 12);
                    QString elidedLabel = painter.fontMetrics().elidedText(labelAt(nextIndex), Qt::ElideMiddle, labelRect.width());
                    QRect bb(painter.fontMetrics().boundingRect(labelRect, Qt::AlignCenter, elidedLabel));
                    nextLabelWidth = (double) bb.width();
                    nextLabelBottom = (double) bb.bottom();
                }
                transientLabelWidth = previousLabelWidth + (nextLabelWidth - previousLabelWidth) * (d->guiIndex - previousIndex);
                transientLabelBottom = previousLabelBottom + (nextLabelBottom - previousLabelBottom) * (d->guiIndex - previousIndex);
            }
        }
        painter.drawRoundedRect(QRectF(d->guiIndex * w + (box.width() - transientWidth) / 2.0 - radius, box.height() - transientHeight - radius, transientWidth + radius * 2, transientHeight + radius * 2), radius, radius);
        if (d->drawLabels && transientLabelWidth > 0.0 && transientLabelBottom > 0.0)
        {
            painter.drawRoundedRect(QRectF(d->guiIndex * w + (box.width() - transientLabelWidth) / 2.0 - radius, box.height(), transientLabelWidth + radius * 2, transientLabelBottom - box.height() + radius / 2.0), radius, radius);
        }

        painter.setPen(Qt::NoPen);
        QPen pen(QColor(115, 115, 115));
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setWidthF(0.5);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        static QVector< double > sin_ts; // FIXME Make a member, not a static variable
        sin_ts.resize(count());

        // Iterate through pages
        double offset = 0.0;
        for (int i = (int) ceil(d->guiIndex); i < count(); ++i)
        {
            if (i < lowerLimit || i > upperLimit) continue;

            double preModifier = d->indexPreModifiers.value(i, 0.0);
            double postModifier = d->indexPostModifiers.value(i, 0.0);
            offset += preModifier;
            if (preModifier >= 0.0)
            {
                double t = (i - (d->guiIndex - offset)) * t1;
                t = qBound(-_PI/2.0, t, _PI/2);
                sin_ts[i] = sin(t);
            }
            else
            {
                sin_ts[i] = 10.0;
            }
            offset += postModifier;
        }

        offset = 0.0;
        for (int i = (int) ceil(d->guiIndex) - 1; i >= 0; --i)
        {
            if (i < lowerLimit || i > upperLimit) continue;

            double preModifier = d->indexPreModifiers.value(i, 0.0);
            double postModifier = d->indexPostModifiers.value(i, 0.0);
            offset += postModifier;
            if (preModifier >= 0.0)
            {
                double t = (i - (d->guiIndex + offset)) * t1;
                t = qBound(-_PI/2.0, t, _PI/2);
                sin_ts[i] = sin(t);
            }
            else
            {
                sin_ts[i] = 10.0;
            }
            offset += preModifier;
        }

        for (int i = 0; i < d->guiIndex; ++i)
        {
            if (i < lowerLimit || i > upperLimit) continue;
            double sin_t = sin_ts.at(i);
            if (sin_t > 1.0) continue;

            double opacity = 1.0;
            if (d->transitionTimes.contains(i) && !d->transitionTimes[i].isNull())
            {
                opacity *= d->transitionTimes[i].elapsed() / 500.0;
            }
            if (i < lowerLimit + limitGrace)
            {
                // opacity *= (i - lowerLimit) / (double) limitGrace; // FIXME shuffle
            }
            painter.setOpacity(opacity);
            QPixmap image = d->images[i].isNull() ? QPixmap(":/images/pager_loading.png") : d->images[i];
            QSize size = image.size();
            size.scale(box, Qt::KeepAspectRatio);
            //double x = sin_t * (a + (w - size.width()) / 2.0) + (w - size.width()) / 2.0; // FIXME shuffle
            double x = w * i + (w - size.width()) / 2.0;
            double y = h - size.height();
            QRect rect((int) x, (int) y, size.width(), size.height());
            painter.drawPixmap(rect, image);
            painter.drawRect(rect);

            if (d->drawLabels)
            {
                QString label = labelAt(i);
                if (!label.isEmpty())
                {
                    QRect labelRect(rect.bottomLeft() + QPoint(0, radius), rect.bottomRight() + QPoint(0, 12 + radius));
                    QString elidedLabel = painter.fontMetrics().elidedText(label, Qt::ElideMiddle, rect.width());

                    painter.save();
                    painter.setPen(QColor(0, 0, 0));

                    QFontMetrics fm(painter.font());

                    QSize numSize = fm.boundingRect(elidedLabel).size();
                    double lozengeRadius = 1 + numSize.height() / 2.0;
                    numSize += QSize(numSize.height(), 0);
                    QRect numRect(0, 0, numSize.width() + 2, numSize.height() + 2);

                    numRect.moveCenter(rect.center());
                    numRect.moveBottom(rect.bottom() + radius + 1);

                    painter.setBrush(Qt::yellow);
                    painter.setPen(QColor(140, 140, 0));
                    painter.drawRoundedRect(numRect, lozengeRadius, lozengeRadius);

                    painter.setPen(Qt::black);
                    painter.drawText(numRect, Qt::AlignCenter, elidedLabel);

                    painter.restore();

                }
            }

            if (d->hasAnnotation.value(i, 0) > 0)
            {
                drawAnnotationTag(&painter, rect);
            }

            if (!d->spotlightsHidden && !d->searchHits.isEmpty())
            {
                drawSearchHits(&painter, d->searchHits.value(i, 0), rect);
            }

            painter.setOpacity(1.0);
        }

        for (int i = count() - 1; i >= d->guiIndex; --i)
        {
            if (i < lowerLimit || i > upperLimit) continue;
            double sin_t = sin_ts.at(i);
            if (sin_t > 1.0) continue;

            double opacity = 1.0;
            if (d->transitionTimes.contains(i) && !d->transitionTimes[i].isNull())
            {
                opacity *= d->transitionTimes[i].elapsed() / 500.0;
            }
            if (i > upperLimit - limitGrace)
            {
                // opacity *= (limitGrace + i - upperLimit) / (double) limitGrace; // FIXME shuffle
            }
            painter.setOpacity(opacity);
            QPixmap image = d->images[i].isNull() ? QPixmap(":/images/pager_loading.png") : d->images[i];
            QSize size = image.size();
            size.scale(box, Qt::KeepAspectRatio);
            //double x = sin_t * (a + (w - size.width()) / 2.0) + (w - size.width()) / 2.0; // FIXME shuffle
            double x = w * i + (w - size.width()) / 2.0;
            double y = h - size.height();
            QRect rect((int) x, (int) y, size.width(), size.height());
            painter.drawPixmap(rect, image);
            painter.drawRect(rect);

            if (d->drawLabels)
            {
                QString label = labelAt(i);
                if (!label.isEmpty())
                {
                    QRect labelRect(rect.bottomLeft() + QPoint(0, radius), rect.bottomRight() + QPoint(0, 12 + radius));
                    QString elidedLabel = painter.fontMetrics().elidedText(label, Qt::ElideMiddle, labelRect.width());
                    painter.save();
                    painter.setPen(QColor(200, 200, 200));
                    painter.setPen(QColor(0, 0, 0));
                    painter.drawText(labelRect, Qt::AlignCenter, elidedLabel);
                    painter.restore();
                }
            }

            if (d->hasAnnotation.value(i, 0) > 0)
            {
                drawAnnotationTag(&painter, rect);
            }

            if (!d->spotlightsHidden && !d->searchHits.isEmpty())
            {
                drawSearchHits(&painter, d->searchHits.value(i, 0), rect);
            }

            painter.setOpacity(1.0);
        }

        sin_ts.clear();
    }

    void Pager::previous(int delta)
    {
        d->previous();
    }

    bool Pager::remove(int index)
    {
        if (validIndex(index))
        {
            d->images.removeAt(index);
            d->updateScrollBar();
            d->labels.removeAt(index);
            d->searchHits.remove(index);
            d->hasAnnotation.remove(index);

            double oldPreModifier = d->indexPreModifiers.value(index, 0.0);
            double oldPostModifier = d->indexPostModifiers.value(index, 0.0);

            // Modify index modifiers
            QMap< int, double > updatedModifiers;
            QMutableMapIterator< int, double > iter(d->indexPreModifiers);
            iter.toBack();
            while (iter.hasPrevious())
            {
                iter.previous();
                int targetIndex = iter.key();
                if (targetIndex > index)
                {
                    double modifier = iter.value();
                    iter.remove();
                    updatedModifiers[targetIndex - 1] = modifier;
                }
                else
                {
                    if (targetIndex == index)
                    {
                        iter.remove();
                    }
                    break;
                }
            }
            if (index != d->currentIndex - 1) updatedModifiers.insert(index, updatedModifiers.value(index, 0.0) + 1.0);
            d->indexPreModifiers.unite(updatedModifiers);
            updatedModifiers.clear();
            iter = d->indexPostModifiers;
            iter.toBack();
            while (iter.hasPrevious())
            {
                iter.previous();
                int targetIndex = iter.key();
                if (targetIndex > index)
                {
                    double modifier = iter.value();
                    iter.remove();
                    updatedModifiers[targetIndex - 1] = modifier;
                }
                else
                {
                    if (targetIndex == index)
                    {
                        iter.remove();
                    }
                    break;
                }
            }
            if (index == d->currentIndex - 1 && index > 0) updatedModifiers.insert(index - 1, updatedModifiers.value(index - 1, 0.0) + 1.0);
            d->indexPostModifiers.unite(updatedModifiers);

            // Modify Search Hits
            QMap< int, int > updatedSearchHits;
            QMutableMapIterator< int, int > iter2(d->searchHits);
            iter2.toBack();
            while (iter2.hasPrevious())
            {
                iter2.previous();
                int targetIndex = iter2.key();
                if (targetIndex > index)
                {
                    int hits = iter2.value();
                    iter2.remove();
                    updatedSearchHits[targetIndex - 1] = hits;
                }
                else
                {
                    if (targetIndex == index)
                    {
                        iter2.remove();
                    }
                    break;
                }
            }
            d->searchHits.unite(updatedSearchHits);

            // Modify Annotations
            QMap< int, int > updatedHasAnnotation;
            iter2 = d->hasAnnotation;
            iter2.toBack();
            while (iter2.hasPrevious())
            {
                iter2.previous();
                int targetIndex = iter2.key();
                if (targetIndex > index)
                {
                    int annotations = iter2.value();
                    iter2.remove();
                    updatedHasAnnotation[targetIndex - 1] = annotations;
                }
                else
                {
                    if (targetIndex == index)
                    {
                        iter2.remove();
                    }
                    break;
                }
            }
            d->hasAnnotation.unite(updatedHasAnnotation);

            if (!nearAsDamnIt(oldPreModifier, 0))
            {
                d->indexPreModifiers.insert(index, d->indexPreModifiers.value(index, 0.0) + oldPreModifier);
            }
            if (!nearAsDamnIt(oldPostModifier, 0))
            {
                if (index > 0)
                {
                    d->indexPostModifiers.insert(index - 1, d->indexPostModifiers.value(index - 1, 0.0) + oldPostModifier);
                }
                else
                {
                    d->indexPreModifiers.insert(index, d->indexPostModifiers.value(index, 0.0) + oldPostModifier);
                }
            }

            if (index < d->currentIndex || d->currentIndex == count()) // FIXME
            {
                if (d->currentIndex == count())
                {
                    previous();
                }
                else
                {
                    d->currentIndex -= 1;
                    d->guiIndex -= 1.0;
                }
            }

            d->timer.start();
            update();

            return true;
        }

        return false;
    }

    bool Pager::rename(int index, const QString & label)
    {
        bool success = false;
        if (validIndex(index))
        {
            d->labels.replace(index, label);
            success = true;
            update();
        }
        return success;
    }

    bool Pager::replace(int index, const QPixmap & image)
    {
        bool success = false;
        if (validIndex(index))
        {
            d->images.replace(index, image);
            // Deal with null images FIXME
            success = true;
            update();
        }
        return success;
    }

    void Pager::resizeEvent(QResizeEvent * event)
    {
        QFrame::resizeEvent(event);
        d->updateScrollBar();
    }

    void Pager::setAnnotations(const QMap< int, int > & annotations)
    {
        d->hasAnnotation = annotations;
        update();
    }

    void Pager::setBoundingBox(const QSize & box)
    {
        d->box = box;
        update();
    }

    void Pager::setContentsMargins(int left, int top, int right, int bottom)
    {
        d->marginLeft = left;
        d->marginTop = top;
        d->marginRight = right;
        d->marginBottom = bottom;
        update();
    }

    void Pager::setDrawLabels(bool draw)
    {
        d->drawLabels = draw;
    }

    void Pager::setOrientation(Qt::Orientation orientation)
    {
        d->scrollBar->setOrientation(orientation);
        d->layout->itemAt(0)->setAlignment(orientation == Qt::Horizontal ? Qt::AlignBottom : Qt::AlignRight);
        update();
    }

    void Pager::setSpotlights(const QMap< int, int > & spotlights)
    {
        d->searchHits = spotlights;
        d->spotlightsHidden = false;
        update();
    }

    void Pager::setSpread(double spread)
    {
        d->spread = qBound(0.0, spread, 1.0);
        update();
    }

    double Pager::spread() const
    {
        return d->spread;
    }

    bool Pager::validIndex(int index, bool append) const
    {
        return index >= 0 && ((append && index <= d->images.size()) || (index < d->images.size()));
    }

    void Pager::wheelEvent(QWheelEvent * event)
    {
        if (event->delta() < 0) {
            d->next(1, true);
        } else {
            d->previous(1, true);
        }
    }

}
