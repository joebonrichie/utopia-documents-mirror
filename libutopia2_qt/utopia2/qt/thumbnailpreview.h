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

#ifndef Utopia_QT_THUMBNAILPREVIEW_H
#define Utopia_QT_THUMBNAILPREVIEW_H

#include <boost/scoped_ptr.hpp>
#include <QLabel>

namespace Utopia
{

    class ThumbnailChooser;

    class ThumbnailPreviewPrivate;
    class ThumbnailPreview : public QLabel
    {
        Q_OBJECT

    public:
        ThumbnailPreview(ThumbnailChooser * chooser, QWidget * parent = 0, Qt::WindowFlags f = 0);
        ~ThumbnailPreview();

    public slots:
        void updateThumbnail();

    protected:
        bool event(QEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);

        boost::scoped_ptr< ThumbnailPreviewPrivate > d;
    };

}

#endif // Utopia_QT_THUMBNAILPREVIEW_H
