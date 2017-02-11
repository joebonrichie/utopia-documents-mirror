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

#include <utopia2/qt/flowbrowser.h>
#include <utopia2/qt/flowbrowser_p.h>

#include <boost/weak_ptr.hpp>

#include <QTimeLine>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QScrollBar>
#include <QList>
#include <QMap>
#include <QVariant>

#include <QVector>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QPair>

#ifdef Q_OS_MACX
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

#include <math.h>
#define PI 3.14159265358979323846
#define DEG2RAD(DEG) ((DEG)*((PI)/(180.0)))

#include <QDebug>

namespace Utopia
{

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserItem ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    FlowBrowserItem::FlowBrowserItem(QGLWidget * qglwidget, QVariant id)
        : id(id), texture(0), aspectRatio(1), loaded(false), x(0), width(1), height(1), qglwidget(qglwidget)
    {
        setImage(QImage(":/images/flowbrowser_loading.png"));
        loaded = false;
    }

    FlowBrowserItem::~FlowBrowserItem()
    {
        deleteTexture();
    }

    void FlowBrowserItem::bindTexture(const QImage & image)
    {
        deleteTexture();

        qglwidget->makeCurrent();
        texture = qglwidget->bindTexture(image);
        qglwidget->doneCurrent();
    }

    void FlowBrowserItem::deleteTexture()
    {
        if (texture)
        {
            qglwidget->makeCurrent();
            qglwidget->deleteTexture(texture);
            qglwidget->doneCurrent();
            texture = 0;
        }
    }

    void FlowBrowserItem::setImage(const QImage & image)
    {
        if (!image.isNull() && image.width() > 0 && image.height() > 0)
        {
            bindTexture(image);

            aspectRatio = (qreal) image.width() / (qreal) image.height();
            if (aspectRatio < 1.0)
            {
                x = (1.0 - aspectRatio) / 2.0;
                width = aspectRatio;
                height = 1.0;
            }
            else
            {
                x = 0.0;
                width = 1.0;
                height = 1 / aspectRatio;
            }
        }
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserItemUpdateQueue /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    FlowBrowserItemUpdateQueue::FlowBrowserItemUpdateQueue()
        : QThread(0), looping(true)
    {
        start();
    }

    FlowBrowserItemUpdateQueue::~FlowBrowserItemUpdateQueue()
    {
        stopLooping();
        wait();
    }

    void FlowBrowserItemUpdateQueue::run()
    {
        while (isLooping())
        {
            inputQueueNotEmptyMutex.lock();
            if (isInputQueueEmpty())
            {
                inputQueueNotEmpty.wait(&inputQueueNotEmptyMutex);
            }
            inputQueueNotEmptyMutex.unlock();

            if (!isLooping())
            {
                break;
            }

            FlowBrowserItem * item;
            QImage image;

            {
                QMutexLocker guard(&inputQueueMutex);
                QPair< FlowBrowserItem *, QImage > job = inputQueue.takeFirst();
                item = job.first;
                image = job.second;

                // Remove old duplicates
                QMutableListIterator< QPair< FlowBrowserItem *, QImage > > iter(inputQueue);
                while (iter.hasNext())
                {
                    if (iter.next().first == item)
                    {
                        iter.remove();
                    }
                }
            }

            // Processing
            image = image.scaled(QSize(256, 256), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            //image = image.scaled(QSize(512, 512), Qt::KeepAspectRatio, Qt::SmoothTransformation);

            {
                QMutexLocker guard(&outputQueueMutex);

                // Remove old duplicates
                QMutableListIterator< QPair< FlowBrowserItem *, QImage > > iter(outputQueue);
                while (iter.hasNext())
                {
                    if (iter.next().first == item)
                    {
                        iter.remove();
                    }
                }

                outputQueue.append(qMakePair< FlowBrowserItem *, QImage >(item, image));

                emit outputReady();
            }
        }
    }

    void FlowBrowserItemUpdateQueue::prependToInputQueue(FlowBrowserItem * item, const QImage & image)
    {
        {
            QMutexLocker guard(&inputQueueMutex);
            inputQueue.prepend(qMakePair< FlowBrowserItem *, QImage >(item, image));
        }
        inputQueueNotEmptyMutex.lock();
        inputQueueNotEmpty.wakeOne();
        inputQueueNotEmptyMutex.unlock();
    }

    QPair< FlowBrowserItem *, QImage > FlowBrowserItemUpdateQueue::popFromOutputQueue()
    {
        QMutexLocker guard(&outputQueueMutex);
        return outputQueue.takeFirst();
    }

    bool FlowBrowserItemUpdateQueue::isIdle()
    {
        QMutexLocker guard(&inputQueueMutex);
        QMutexLocker guard2(&outputQueueMutex);
        return inputQueue.isEmpty() && outputQueue.isEmpty();
    }

    bool FlowBrowserItemUpdateQueue::isLooping()
    {
        QMutexLocker guard(&loopingMutex);
        return looping;
    }

    void FlowBrowserItemUpdateQueue::stopLooping()
    {
        {
            QMutexLocker guard(&loopingMutex);
            looping = false;
        }

        inputQueueNotEmptyMutex.lock();
        inputQueueNotEmpty.wakeOne();
        inputQueueNotEmptyMutex.unlock();
    }

    bool FlowBrowserItemUpdateQueue::isOutputQueueEmpty()
    {
        QMutexLocker guard(&outputQueueMutex);
        return outputQueue.isEmpty();
    }

    boost::shared_ptr< FlowBrowserItemUpdateQueue > FlowBrowserItemUpdateQueue::instance()
    {
        static boost::weak_ptr< FlowBrowserItemUpdateQueue > singleton;
        boost::shared_ptr< FlowBrowserItemUpdateQueue > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< FlowBrowserItemUpdateQueue >(new FlowBrowserItemUpdateQueue());
            singleton = shared;
        }
        return shared;
    }

    bool FlowBrowserItemUpdateQueue::isInputQueueEmpty()
    {
        QMutexLocker guard(&inputQueueMutex);
        return inputQueue.isEmpty();
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserModelPrivate ////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    FlowBrowserModelPrivate::FlowBrowserModelPrivate(QObject * parent, QGLWidget * qglwidget, const QString & title)
        : QObject(parent), qglwidget(qglwidget), position(0), backgroundColor(0, 0, 0), title(title), dragging(false)
    {
        animation.from = 0.0;
        animation.to = 0.0;
        animation.timeLine.setCurveShape(QTimeLine::LinearCurve);
        animation.timeLine.setUpdateInterval(10);
        connect(&animation.timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(onValueChanged(qreal)));
        connect(&animation.timeLine, SIGNAL(finished()), this, SLOT(onFinished()));

        updateQueue = FlowBrowserItemUpdateQueue::instance();
    }

    FlowBrowserModelPrivate::~FlowBrowserModelPrivate()
    {
        foreach (FlowBrowserItem * item, images)
        {
            delete item;
        }
    }

    void FlowBrowserModelPrivate::animateTo(int i)
    {
        //qDebug() << "---" << position << i;
        if (position != i && (animation.timeLine.state() != QTimeLine::Running || animation.to != i))
        {
            animation.timeLine.stop();

            animation.from = position;
            animation.to = i;

            if (animation.from < 0 || animation.from > images.count() - 1)
            {
                animation.timeLine.setDuration(150);
            }
            else
            {
                animation.timeLine.setDuration(qMin(400 + abs((int)(animation.to - animation.from)) * 40, 1200));
            }
            animation.timeLine.setCurrentTime(0);
            animation.timeLine.start();
        }
    }

    void FlowBrowserModelPrivate::onFinished()
    {
        position = animation.to;
        emit updated();
    }

    void FlowBrowserModelPrivate::onValueChanged(qreal v)
    {
        //qDebug() << "+++" << v << animation.timeLine.state();
        emit updated();
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserModel ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    FlowBrowserModel::FlowBrowserModel(FlowBrowser * parent, const QString & title)
        : QObject(parent), d(new FlowBrowserModelPrivate(this, parent, title))
    {
        connect(d, SIGNAL(updated()), this, SIGNAL(updated()));
    }

    void FlowBrowserModel::adjustCurrent(int i, int offset)
    {
        if (count() == 0)
        {
            goTo(0);
        }
        else if (current() >= count())
        {
            goTo(count() - 1);
        }
        else if (i < current())
        {
            goTo(current() + offset);
        }
    }

    void FlowBrowserModel::append(const QVariant & id)
    {
        bool empty = d->images.isEmpty();
        d->images.append(new FlowBrowserItem(d->qglwidget, id));
        adjustCurrent(count() - 1, 1);
        if (empty) {
            emit emptinessChanged(false);
        }
    }

    void FlowBrowserModel::append(const QVariant & id, const QImage & image)
    {
        append(id);
        update(count() - 1, image);
    }

    const QVariant & FlowBrowserModel::at(int i) const
    {
        return d->images.at(i)->id;
    }

    const QColor & FlowBrowserModel::backgroundColor() const
    {
        return d->backgroundColor;
    }

    int FlowBrowserModel::center() const
    {
        return qRound(d->position);
    }

    void FlowBrowserModel::clear()
    {
        bool empty = d->images.isEmpty();
        d->images.clear();
        goTo(0);
        if (!empty) {
            emit emptinessChanged(true);
        }
    }

    int FlowBrowserModel::count() const
    {
        return d->images.count();
    }

    int FlowBrowserModel::current() const
    {
        if (d->animation.timeLine.state() == QTimeLine::Running)
        {
            return d->animation.to;
        }
        else
        {
            return center();
        }
    }

    const QVariant & FlowBrowserModel::first() const
    {
        return d->images.first()->id;
    }

    void FlowBrowserModel::goTo(int i)
    {
        if (!d->dragging)
        {
            d->animateTo(qBound(0, i, count() - 1));
            emit updated();
        }
    }

    int FlowBrowserModel::indexOf(const QVariant & id, int from)
    {
        QVectorIterator< FlowBrowserItem * > iter(d->images);
        int i = 0;
        while (iter.hasNext())
        {
            if (from <= i && iter.next()->id == id)
            {
                return i;
            }
            ++i;
        }
        return -1;
    }

    void FlowBrowserModel::insert(int i, const QVariant & id)
    {
        bool empty = d->images.isEmpty();
        d->images.insert(i, new FlowBrowserItem(d->qglwidget, id));
        adjustCurrent(i, 1);
        if (empty) {
            emit emptinessChanged(false);
        }
    }

    void FlowBrowserModel::insert(int i, const QVariant & id, const QImage & image)
    {
        insert(i, id);
        update(i, image);
    }

    bool FlowBrowserModel::isEmpty() const
    {
        return d->images.isEmpty();
    }

    const QVariant & FlowBrowserModel::last() const
    {
        return d->images.last()->id;
    }

    int FlowBrowserModel::lastIndexOf(const QVariant & id, int from)
    {
        QVectorIterator< FlowBrowserItem * > iter(d->images);
        iter.toBack();
        int i = 0;
        if (from == -1)
        {
            from = d->images.count() - 1;
        }
        while (iter.hasPrevious())
        {
            if (from >= i && iter.previous()->id == id)
            {
                return i;
            }
            ++i;
        }
        return -1;
    }

    void FlowBrowserModel::next(int delta)
    {
        goTo(current() + delta);
    }

    qreal FlowBrowserModel::position() const
    {
        if (d->animation.timeLine.state() == QTimeLine::Running)
        {
            return d->animation.from + (d->animation.to - d->animation.from) * sin(d->animation.timeLine.currentValue() * PI / 2.0);
        }
        else
        {
            return d->position;
        }
    }

    void FlowBrowserModel::prepend(const QVariant & id)
    {
        bool empty = d->images.isEmpty();
        d->images.prepend(new FlowBrowserItem(d->qglwidget, id));
        adjustCurrent(0, 1);
        if (empty) {
            emit emptinessChanged(false);
        }
    }

    void FlowBrowserModel::prepend(const QVariant & id, const QImage & image)
    {
        prepend(id);
        update(0, image);
    }

    void FlowBrowserModel::previous(int delta)
    {
        goTo(current() - delta);
    }

    void FlowBrowserModel::removeAt(int i)
    {
        bool empty = d->images.isEmpty();
        d->images.remove(i);
        adjustCurrent(i, -1);
        if (!empty && d->images.isEmpty()) {
            emit emptinessChanged(true);
        }
    }

    void FlowBrowserModel::requestUpdate(int i)
    {
        emit requiresUpdate(i);
    }

    void FlowBrowserModel::setBackgroundColor(const QColor & color)
    {
        d->backgroundColor = color;
        emit updated();
    }

    void FlowBrowserModel::setTitle(const QString & title)
    {
        d->title = title;
        emit updated();
    }

    QString FlowBrowserModel::title() const
    {
        return d->title;
    }

    void FlowBrowserModel::update(int i, const QImage & image)
    {
        d->updateQueue->prependToInputQueue(d->images.at(i), image);
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserPrivate /////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    FlowBrowserPrivate::FlowBrowserPrivate(FlowBrowser * parent)
        : QObject(parent),
          flowBrowser(parent),
          defaultBackgroundColor(Qt::black),
          dragFromPosition(0.0),
          offset(0.6),
          separation(0.2)
    {
        updateQueue = FlowBrowserItemUpdateQueue::instance();
        connect(updateQueue.get(), SIGNAL(outputReady()), flowBrowser, SLOT(applyTexture()));

        renderer.selectBuffer = 0;
    }

    void FlowBrowserPrivate::render(bool names)
    {
        if (currentModel)
        {
            //qDebug() << "FlowBrowserRenderer::render";
            // Grab one thing from the queue if any there
            if (!updateQueue->isOutputQueueEmpty())
            {
                QTimer::singleShot(10, flowBrowser, SLOT(applyTexture()));
            }

            if (currentModel->count() == 0) { return; }

            qreal position = currentModel->d->position = currentModel->position();
            bool updateRequested(false);

            // Should these be floor() round() or ceil() ?FIXME?
            int from = (int) qMax(position - renderer.scope, 0.0);
            int to = (int) qMin(position + renderer.scope + 1.0, (qreal) currentModel->count());

            for (int j = from; j < to; ++j)
            {
                int i = j;
                if (position < 0)
                {
                    i = to - j - 1;
                }
                else if (j > (int) position)
                {
                    i = to - (j - (int) position);
                }

                //qDebug() << position << ":" << from << "(" << j << "-" << i << ")" << to << currentModel->count();

                if (i < 0)
                {
                    qDebug() << "render failed";
                    return;
                }

                FlowBrowserItem * item = currentModel->d->images[i];
                qreal itemPosition = i - position;
                qreal shade = 1.0;

                //qDebug() << "texture =" << item->texture;
                if (!item->loaded)
                {
                    if (updateRequested)
                    {
                        QTimer::singleShot(30, flowBrowser, SLOT(update()));
                    }
                    else
                    {
                        item->loaded = true;
                        currentModel->requestUpdate(i);
                        updateRequested = true;
                    }
                }

                glPushMatrix();

                glTranslatef(0, -0.5, -2);
                if (itemPosition > 1.0)
                {
                    qreal index = itemPosition - 1.0;
                    glTranslatef((index * separation) + 1.4, 0, -offset);
                    glRotatef(-70, 0, 1, 0);
                    glTranslatef(item->x - 1, 0, 0);
                    shade = (itemPosition >= (renderer.scope - 2)) ? (0.7 - (itemPosition - (renderer.scope - 2)) * 0.35) : 0.7;
                }
                else if (itemPosition < -1.0)
                {
                    qreal index = itemPosition + 1.0;
                    glTranslatef((index * separation) - 1.4, 0, -offset);
                    glRotatef(70, 0, 1, 0);
                    glTranslatef(-item->x, 0, 0);
                    shade = (itemPosition < (2 - renderer.scope)) ? (0.7 - (-itemPosition - (renderer.scope - 2)) * 0.35) : 0.7;
                }
                else if (itemPosition > 0)
                {
                    glTranslatef((itemPosition - 1) * item->x, 0, 0);
                    glTranslatef(0.5 + (1.4 - 0.5) * itemPosition, 0, -offset * itemPosition);
                    glRotatef(itemPosition * -70, 0, 1, 0);
                    glTranslatef(item->x - 1, 0, 0);
                    shade = 1 - itemPosition * 0.3;
                }
                else
                {
                    glTranslatef((itemPosition + 1) * item->x, 0, 0);
                    glTranslatef((1.4 - 0.5) * itemPosition - 0.5, 0,  offset * itemPosition);
                    glRotatef(itemPosition * -70, 0, 1, 0);
                    glTranslatef(-item->x, 0, 0);
                    shade = 1 + (itemPosition * 0.3);
                }

                if (names)
                {
                    glPushName(i);

                    glBegin(GL_QUADS);
                    glVertex3f(item->x + item->width, item->height, 0);
                    glVertex3f(item->x, item->height, 0);
                    glVertex3f(item->x, -item->height, 0);
                    glVertex3f(item->x + item->width, -item->height, 0);
                    glEnd();

                    glPopName();
                }
                else
                {
                    float r(defaultBackgroundColor.redF());
                    float g(defaultBackgroundColor.greenF());
                    float b(defaultBackgroundColor.blueF());

                    qreal shade_reflection = shade * 0.3;
                    qreal share_reflection_bottom = shade_reflection * (1.0 - item->height);

                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDepthMask(GL_FALSE);
                    glBegin(GL_QUADS);
                        glColor4f(r, g, b, 1.0);
                        glVertex3f(item->x + item->width, 0, 0);
                        glVertex3f(item->x, 0, 0);
                        glVertex3f(item->x, -item->height, 0);
                        glVertex3f(item->x + item->width, -item->height, 0);
                    glEnd();
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glBindTexture(GL_TEXTURE_2D, item->texture);
                    glBegin(GL_QUADS);
                        glColor4f(1.0, 1.0, 1.0, shade_reflection);
                        glTexCoord2f(1, 0);
                        glVertex3f(item->x + item->width, 0, 0);
                        glColor4f(1.0, 1.0, 1.0, shade_reflection);
                        glTexCoord2f(0, 0);
                        glVertex3f(item->x, 0, 0);
                        glColor4f(1.0, 1.0, 1.0, share_reflection_bottom);
                        glTexCoord2f(0, 1);
                        glVertex3f(item->x, -item->height, 0);
                        glColor4f(1.0, 1.0, 1.0, share_reflection_bottom);
                        glTexCoord2f(1, 1);
                        glVertex3f(item->x + item->width, -item->height, 0);
                    glEnd();
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glBegin(GL_LINE_LOOP);
                        glColor4f(0.5, 0.5, 0.5, shade_reflection);
                        glVertex3f(item->x + item->width, 0, 0);
                        glColor4f(0.5, 0.5, 0.5, shade_reflection);
                        glVertex3f(item->x, 0, 0);
                        glColor4f(0.5, 0.5, 0.5, share_reflection_bottom);
                        glVertex3f(item->x, -item->height, 0);
                        glColor4f(0.5, 0.5, 0.5, share_reflection_bottom);
                        glVertex3f(item->x + item->width, -item->height, 0);
                    glEnd();

                    glBegin(GL_QUADS);
                        glColor4f(r, g, b, 1.0);
                        glVertex3f(item->x + item->width, item->height, 0);
                        glVertex3f(item->x, item->height, 0);
                        glVertex3f(item->x, 0, 0);
                        glVertex3f(item->x + item->width, 0, 0);
                    glEnd();
                    glBindTexture(GL_TEXTURE_2D, item->texture);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glBegin(GL_QUADS);
                        glColor4f(1.0, 1.0, 1.0, shade);
                        glTexCoord2f(1, 1);
                        glVertex3f(item->x + item->width, item->height, 0);
                        glTexCoord2f(0, 1);
                        glVertex3f(item->x, item->height, 0);
                        glTexCoord2f(0, 0);
                        glVertex3f(item->x, 0, 0);
                        glTexCoord2f(1, 0);
                        glVertex3f(item->x + item->width, 0, 0);
                    glEnd();
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glBegin(GL_LINE_LOOP);
                        glColor4f(0.5, 0.5, 0.5, shade);
                        glVertex3f(item->x + item->width, item->height, 0);
                        glVertex3f(item->x, item->height, 0);
                        glVertex3f(item->x, 0, 0);
                        glVertex3f(item->x + item->width, 0, 0);
                    glEnd();

                    glDepthMask(GL_TRUE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glBegin(GL_QUADS);
                        glVertex3f(item->x + item->width, item->height, 0);
                        glVertex3f(item->x, item->height, 0);
                        glVertex3f(item->x, -item->height, 0);
                        glVertex3f(item->x + item->width, -item->height, 0);
                    glEnd();
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                }

                glPopMatrix();
            }
        }
    }

    void FlowBrowserPrivate::removeModel(QObject * model)
    {
        models.removeAll((FlowBrowserModel *) model);
    }

    void FlowBrowserPrivate::resize(int w, int h)
    {
        renderer.sceneToWindow = (qreal) h * tan(DEG2RAD(30)) / tan(DEG2RAD(40));
        renderer.scope = qMax((((qreal) w / renderer.sceneToWindow - 1.0) / 2.0) / separation, 3.0);
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowser ////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    FlowBrowser::FlowBrowser(QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
        : QGLWidget(parent, shareWidget, f), d(new FlowBrowserPrivate(this))
    {
        init();
    }

    FlowBrowser::FlowBrowser(QGLContext * context, QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
        : QGLWidget(context, parent, shareWidget, f), d(new FlowBrowserPrivate(this))
    {
        init();
    }

    FlowBrowser::FlowBrowser(const QGLFormat & format, QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
        : QGLWidget(format, parent, shareWidget, f), d(new FlowBrowserPrivate(this))
    {
        init();
    }

    FlowBrowser::~FlowBrowser()
    {
        clear();
        if (d->renderer.selectBuffer)
        {
            delete [] d->renderer.selectBuffer;
        }
    }

    FlowBrowserModel * FlowBrowser::addModel(const QString & title)
    {
        FlowBrowserModel * model = new FlowBrowserModel(this, title);
        d->models.append(model);
        if (d->currentModel == 0)
        {
            d->currentModel = model;
        }
        connect(model, SIGNAL(destroyed(QObject*)), d.get(), SLOT(removeModel(QObject*)));
        connect(model, SIGNAL(updated()), this, SLOT(update()));
        return model;
    }

    void FlowBrowser::applyTexture()
    {
        //qDebug() << "FlowBrowser::applyTexture" << d->updateQueue->isOutputQueueEmpty();
        if (!d->updateQueue->isOutputQueueEmpty())
        {
            QPair< FlowBrowserItem *, QImage > item(d->updateQueue->popFromOutputQueue());
            item.first->setImage(item.second);
            update();
        }
    }

    void FlowBrowser::clear()
    {
        foreach(FlowBrowserModel * model, d->models)
        {
            delete model;
        }
    }

    FlowBrowserModel * FlowBrowser::currentModel() const
    {
        return d->currentModel.data();
    }

    const QColor & FlowBrowser::defaultBackgroundColor() const
    {
        return d->defaultBackgroundColor;
    }

    int FlowBrowser::indexAt(int x, int y)
    {
        makeCurrent();

        glSelectBuffer(d->renderer.maxNameStackDepth, d->renderer.selectBuffer);

        glRenderMode(GL_SELECT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        gluPickMatrix(x, viewport[3] - y, 1, 1, viewport);

        gluPerspective(40, (qreal) width() / (qreal) height(), 1, 10);
        glMatrixMode(GL_MODELVIEW);
        glInitNames();

        glColorMask(0, 0, 0, 0);
        glDepthMask(0);

        d->render(true);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glFlush();

        GLuint hits = glRenderMode(GL_RENDER);
        int index = -1;

        if (hits > 0)
        {
            GLuint i;
            GLuint names, *ptr, minZ, *ptrNames, numberOfNames;

            ptr = d->renderer.selectBuffer;
            minZ = 0xffffffff;
            for (i = 0; i < hits; i++)
            {
                names = *ptr;
                ptr++;
                if (*ptr < minZ)
                {
                    numberOfNames = names;
                    minZ = *ptr;
                    ptrNames = ptr+2;
                }

                ptr += names+2;
            }
            ptr = ptrNames;
            index = ptr[numberOfNames - 1];
        }

        glColorMask(1, 1, 1, 1);
        glDepthMask(1);

        doneCurrent();

        return index;
    }

    void FlowBrowser::init()
    {
        setAutoFillBackground(false);
        setMouseTracking(true);
    }

    void FlowBrowser::initializeGL()
    {
        qglClearColor(defaultBackgroundColor());
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColorMaterial(GL_FRONT, GL_AMBIENT);
        glEnable(GL_COLOR_MATERIAL);

        GLint maxNameStackDepth;
        glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, &maxNameStackDepth);
        d->renderer.maxNameStackDepth = maxNameStackDepth;
        d->renderer.selectBuffer = new GLuint[maxNameStackDepth];

#ifdef Q_OS_MACX
        GLint swap = 1;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swap);
#endif
    }

    bool FlowBrowser::isEmpty() const
    {
        int count = 0;
        foreach (FlowBrowserModel * model, d->models)
        {
            count += model->count();
        }
        return count == 0;
    }

    void FlowBrowser::mouseDoubleClickEvent(QMouseEvent * event)
    {
        FlowBrowserModel * model(currentModel());
        if (model)
        {
            if (event->button() == Qt::LeftButton)
            {
                int index = indexAt(event->x(), event->y());
                if (index == model->center())
                {
                    emit model->selected(index); // FIXME
                }
            }
        }

        event->ignore();
    }

    void FlowBrowser::mouseMoveEvent(QMouseEvent * event)
    {
        FlowBrowserModel * model(currentModel());
        if (model)
        {
            if (event->buttons().testFlag(Qt::LeftButton))
            {
                if (!model->d->dragging &&
                    (d->dragFrom - event->pos()).manhattanLength() > 3)
                {
                    model->d->dragging = true;
                }

                if (model->d->dragging)
                {
                    model->d->position = d->dragFromPosition + (qreal) ((d->dragFrom.x() - event->pos().x()) / (d->renderer.sceneToWindow * d->separation));
                    update();
                }
            }
        }
    }

    void FlowBrowser::mousePressEvent(QMouseEvent * event)
    {
        FlowBrowserModel * model(currentModel());
        if (model)
        {
            d->dragFrom = event->pos();
            d->dragFromPosition = model->position();
            model->d->animation.timeLine.stop();
            event->accept();
        }
    }

    void FlowBrowser::mouseReleaseEvent(QMouseEvent * event)
    {
        FlowBrowserModel * model(currentModel());
        if (model)
        {
            if (model->d->dragging)
            {
                model->d->dragging = false;
                model->goTo(model->center());
            }
            else
            {
                int index = indexAt(event->x(), event->y());
                if (index >= 0)
                {
                    model->goTo(index);
                }
                else
                {
                    model->goTo(model->center());
                }
            }
        }
    }

    qreal FlowBrowser::offset() const
    {
        return d->offset;
    }

    void FlowBrowser::paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d->render();
    }

    void FlowBrowser::resizeGL(int w, int h)
    {
        w = width(); h = height();
        glViewport(0, 0, (GLsizei) w, (GLsizei) h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(40.0, (qreal) w / (qreal) h, 1, 10);
        glMatrixMode(GL_MODELVIEW);

        d->resize(w, h); // FIXME
    }

    qreal FlowBrowser::separation() const
    {
        return d->separation;
    }

    void FlowBrowser::setCurrentModel(FlowBrowserModel * model)
    {
        d->currentModel = model;
    }

    void FlowBrowser::setDefaultBackgroundColor(const QColor & color)
    {
        d->defaultBackgroundColor = color;
        update();
    }

    void FlowBrowser::setOffset(qreal offset)
    {
        d->offset = offset;
        update();
    }

    void FlowBrowser::setSeparation(qreal separation)
    {
        d->separation = separation;
        d->resize(width(), height()); // FIXME
        update();
    }

    void FlowBrowser::wheelEvent(QWheelEvent * event)
    {
        FlowBrowserModel * model(currentModel());
        if (model)
        {
            model->previous(event->delta() > 0 ? 1 : -1);
            event->accept();
        }
    }

} // namespace Utopia
