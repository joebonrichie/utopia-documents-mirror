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

#ifndef PAPYRO_EMBEDDEDFRAME_H
#define PAPYRO_EMBEDDEDFRAME_H

#include <papyro/config.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#endif

#include <QSize>
#include <QWidget>

namespace Papyro
{

    class EmbeddedFramePrivate;

    class EmbeddedFrame : public QWidget
    {
        Q_OBJECT

    public:
        virtual ~EmbeddedFrame();

        Spine::AnnotationHandle annotation() const;
        const QRectF & bounds() const;
        QSize controlSize() const;

        static EmbeddedFrame * probe(Spine::AnnotationHandle annotation, const QRectF & bounds, QWidget * parent = 0);
        static QWidget * probe(Spine::AnnotationHandle annotation);

    public Q_SLOTS:
        void showControls(int msecs = 800);

    protected Q_SLOTS:
        void hideControls();
        void launchPane(int idx);
        void onCloseClicked();
        void onLaunchClicked();
        void onMagnifyClicked();
        void onPlayClicked();

        void onGraphClicked();

    Q_SIGNALS:
        void flip();

    protected:
        EmbeddedFrame(Spine::AnnotationHandle annotation, const QRectF & bounds, QWidget * parent = 0);

        void enterEvent(QEvent * event);
        void leaveEvent(QEvent * event);
        void paintEvent(QPaintEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void remask();
        void resizeEvent(QResizeEvent * event);

    private:
        EmbeddedFramePrivate * d;

    };

}

#endif // PAPYRO_EMBEDDEDFRAME_H
