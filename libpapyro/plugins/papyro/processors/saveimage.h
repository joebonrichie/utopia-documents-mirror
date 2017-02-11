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

#include <papyro/selectionprocessor.h>

#include <QFrame>
#include <QPropertyAnimation>
#include <QWidget>

namespace Papyro
{

    class ContextPreview : public QWidget
    {
        Q_OBJECT

    public:
        ContextPreview(QWidget * parent, const QPixmap & pixmap, const QVariantMap & params);
        ~ContextPreview();

    protected:
        void enterEvent(QEvent * event);
        void leaveEvent(QEvent * event);
        void resizeEvent(QResizeEvent * event);

    protected slots:
        void onSaveContextButtonClicked();
        void onShowContextButtonClicked();

    signals:
        void showPage(const QVariantMap & params);

    private:
        QPixmap pixmap;
        QVariantMap params;
        QPropertyAnimation slidingAnimation;
        QFrame * sliver;
        QColor sliverBackgroundColor;

    }; // class ContextPreview




    class SaveImageProcessor : public SelectionProcessor
    {
    public:
        int category() const;
        void processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos = QPoint());
        QString title() const;
        int weight() const;

    }; // class SaveImageProcessor




    class PopOutImageProcessor : public SelectionProcessor
    {
    public:
        int category() const;
        void processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos = QPoint());
        QString title() const;
        int weight() const;

    }; // class PopOutImageProcessor




    class ImagingProcessorFactory : public SelectionProcessorFactory
    {
    public:
        QList< boost::shared_ptr< SelectionProcessor > > selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor);

    }; // class ImagingProcessorFactory

} // namespace Papryo
