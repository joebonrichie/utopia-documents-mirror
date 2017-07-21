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

#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <papyro/config.h>
#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Document.h>
#  include <spine/Cursor.h>
#  include <spine/TextSelection.h>
#  include <spine/Annotation.h>
#endif

#include <QColor>
#include <QImage>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QTime>

#include <QColor>
#include <QImage>
#include <QPainterPath>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <boost/scoped_ptr.hpp>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QWidget>

class QMenu;

namespace Utopia
{
    class Conversation;
}

namespace Papyro
{

    class MouseButtonEvent;
    class CommentData;
    class PageViewPrivate;

    class LIBPAPYRO_API PageView : public QWidget
    {
        Q_OBJECT

    public:
        enum PageDecoration
        {
            CreasedSpineLeft    = 0x000001,
            CreasedSpineRight   = 0x000002
        };
        Q_DECLARE_FLAGS(PageDecorations, PageDecoration);
        typedef enum
        {
            NoMargin        = 0x0,
            LeftMargin      = 0x1,
            RightMargin     = 0x2,
            TopMargin       = 0x4,
            BottomMargin    = 0x8
        } Margin;
        Q_DECLARE_FLAGS(Margins, Margin);

        PageView(QWidget * parent = 0, Qt::WindowFlags f = 0);
        PageView(Spine::DocumentHandle document, int page, QWidget * parent = 0, Qt::WindowFlags f = 0);
        ~PageView();

        Spine::TextExtentHandle activeSpotlight() const;
        void clear();
        void clearActiveSpotlight();
        void clearSpotlights();
        void clearTemporaryFocus();
        void copySelectedText();
        Spine::CursorHandle cursorAt(const QPointF & point, Spine::DocumentElement element_ = Spine::ElementCharacter) const;
        Spine::DocumentHandle document() const;
        void focusExtent(Spine::Area area);
        void focusExtent(Spine::TextExtentHandle extent);
        void hideSpotlights(bool hide = true);
        double horizontalZoom() const;
        Spine::CursorHandle imageCursorAt(const QPointF & point) const;
        bool isNull() const;
        QRectF mediaRect(bool transformed = false) const;
        QSizeF mediaSize(bool transformed = false) const;
        Spine::CursorHandle newCursor() const;
        const Spine::Page * page() const;
        int pageNumber() const;
        QRectF pageRect(bool transformed = false) const;
        QSizeF pageSize(bool transformed = false) const;
        void populateContextMenuAt(QMenu * menu, const QPoint & pos);
        void resizeToHeight(int h);
        void resizeToSize(const QSize & size);
        void resizeToWidth(int w);
        int rotation() const;
        void setActiveSpotlight(Spine::TextExtentHandle extent);
        void setSpotlights(const Spine::TextExtentSet & extents);
        const Spine::TextExtentSet & spotlights() const;
        Spine::CursorHandle textCursorAt(const QPointF & point, Spine::DocumentElement element_ = Spine::ElementCharacter) const;
        QPoint transformFromPage(const QPointF & point) const;
        QRect transformFromPage(const QRectF & rect) const;
        QSize transformFromPage(const QSizeF & size) const;
        QPointF transformToPage(const QPoint & point) const;
        QRectF transformToPage(const QRect & rect) const;
        QSizeF transformToPage(const QSize & size) const;
        QTransform userTransform() const;
        double verticalZoom() const;

        // Public static helpers methods
        static QPainterPath asPath(const Spine::TextExtentHandle & extent, int pageNumber);
        static QPainterPath asPath(const Spine::TextSelection & selection, int pageNumber);

    public slots:
        void setHorizontalZoom(double zoom);
        void setPage(Spine::DocumentHandle document, int page);
        void setPage(int page);
        void setRotation(int degrees);
        void setVerticalZoom(double zoom);
        void setZoom(double zoom);
        void updateAnnotations(const std::string & name, const Spine::AnnotationSet & annotations, bool added);
        void updateAreaSelection(const std::string & name, const Spine::AreaSet & areas, bool added);
        void updateTextSelection(const std::string & name, const Spine::TextExtentSet & extents, bool added);

    signals:
        void pageSelected(const Spine::Page *);
        void manageSelection();
        void manageExpandedSelection();
        void exploreSelection();
        void publishChanges();
        void urlRequested(const QUrl & url, const QString & target);
        void pageRotated();

    protected:
        void contextMenuEvent(QContextMenuEvent * event);
        void dropEvent (QDropEvent * event);
        void dragEnterEvent(QDragEnterEvent * event);
        void dragMoveEvent(QDragMoveEvent * event);
        bool event(QEvent * event);
        void leaveEvent(QEvent * event);
        void recomputeDarkness();
        void recomputeTemporaryFocus();
        void resizeEvent(QResizeEvent * event);
        QPixmap pageImage(QSize size, QColor paperColour = Qt::white);
        void paintEvent(QPaintEvent * event);

    protected Q_SLOTS:
        void deleteAnnotation(const QString & uri);
        void copyEmailAddress();
        void executePhraseLookup(int idx);
        void onMousePressTimeout();
        void renderThreadFinished();
        void saveImageAs();

    private:
        // Private
        PageViewPrivate * d;

        void _initialise();

    }; // class PageView

} // namespace PAPYRO

Q_DECLARE_OPERATORS_FOR_FLAGS(Papyro::PageView::Margins);

#endif
