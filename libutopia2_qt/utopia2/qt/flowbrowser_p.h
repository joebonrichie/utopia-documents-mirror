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

#ifndef Utopia_QT_FLOWBROWSER_P_H
#define Utopia_QT_FLOWBROWSER_P_H

#include <boost/shared_ptr.hpp>

#include <QColor>
#include <QGLWidget>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QThread>
#include <QTimeLine>
#include <QVariant>
#include <QVector>
#include <QWaitCondition>

#ifdef Q_OS_MACX
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

class QGLWidget;

namespace Utopia
{

    class FlowBrowser;
    class FlowBrowserModel;




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserItem ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class FlowBrowserItem : public QObject
    {
        Q_OBJECT

    public:
        FlowBrowserItem(QGLWidget * qglwidget, QVariant id = QVariant());
        ~FlowBrowserItem();

        void setImage(const QImage & pixmap);

        QVariant id;
        GLuint texture;
        qreal aspectRatio;
        bool loaded;
        qreal x;
        qreal width;
        qreal height;

    private:
        QGLWidget * qglwidget;

        void deleteTexture();
        void bindTexture(const QImage & image);
    };




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserItemUpdateQueue /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class FlowBrowserItemUpdateQueue : public QThread
    {
        Q_OBJECT

    public:
        FlowBrowserItemUpdateQueue();
        ~FlowBrowserItemUpdateQueue();
        void run();
        void prependToInputQueue(FlowBrowserItem * item, const QImage & image);
        QPair< FlowBrowserItem *, QImage > popFromOutputQueue();
        bool isIdle();
        bool isLooping();
        void stopLooping();
        bool isOutputQueueEmpty();

        static boost::shared_ptr< FlowBrowserItemUpdateQueue > instance();

    protected:
        bool isInputQueueEmpty();

    signals:
        void outputReady();

    private:
        QList< QPair< FlowBrowserItem *, QImage > > inputQueue;
        QMutex inputQueueMutex;
        QList< QPair< FlowBrowserItem *, QImage > > outputQueue;
        QMutex outputQueueMutex;

        QWaitCondition inputQueueNotEmpty;
        QMutex inputQueueNotEmptyMutex;

        bool looping;
        QMutex loopingMutex;
    };




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserModelPrivate ////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class FlowBrowserModelPrivate : public QObject
    {
        Q_OBJECT

    public:
        FlowBrowserModelPrivate(QObject * parent, QGLWidget * qglwidget, const QString & title);
        ~FlowBrowserModelPrivate();

        QGLWidget * qglwidget;
        QVector< FlowBrowserItem * > images;
        qreal position;
        QColor backgroundColor;
        QString title;
        bool dragging;

        struct
        {
            QTimeLine timeLine;
            qreal from;
            int to;
        } animation;

        void animateTo(int i);

        boost::shared_ptr< FlowBrowserItemUpdateQueue > updateQueue;

    signals:
        void updated();

    protected slots:
        void onFinished();
        void onValueChanged(qreal);
    };




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserPrivate /////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class FlowBrowserPrivate : public QObject
    {
        Q_OBJECT

    public:
        FlowBrowserPrivate(FlowBrowser * parent);

        FlowBrowser * flowBrowser;
        QColor defaultBackgroundColor;
        QPoint dragFrom;
        qreal dragFromPosition;
        bool dragging;
        qreal offset;
        qreal separation;

        struct
        {
            GLint maxNameStackDepth;
            GLuint * selectBuffer;
            qreal sceneToWindow;
            qreal scope;
        } renderer;

        boost::shared_ptr< FlowBrowserItemUpdateQueue > updateQueue;

        QPointer< FlowBrowserModel > currentModel;
        QList< FlowBrowserModel * > models;

        void render(bool names = false);
        void resize(int w, int h);

    public slots:
        void removeModel(QObject * model);
    };

}

#endif // Utopia_QT_FLOWBROWSER_P_H
