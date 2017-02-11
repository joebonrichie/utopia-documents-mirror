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

#ifndef Utopia_QT_FLOWBROWSER
#define Utopia_QT_FLOWBROWSER

#include <utopia2/qt/config.h>

#include <QGLWidget>
#include <boost/scoped_ptr.hpp>

class QEvent;
class QMouseEvent;
class QWheelEvent;

namespace Utopia
{

    class FlowBrowser;




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowserModel ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

	class FlowBrowserModelPrivate;
	class LIBUTOPIA_QT_API FlowBrowserModel : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

    public:
        FlowBrowserModel(FlowBrowser * parent, const QString & title);

        void append(const QVariant & id);
        void append(const QVariant & id, const QImage & image);
        const QVariant & at(int i) const;
        const QColor & backgroundColor() const;
        int center() const;
        void clear();
        int count() const;
        int current() const;
        const QVariant & first() const;
        int indexOf(const QVariant & id, int from = 0);
        void insert(int i, const QVariant & id);
        void insert(int i, const QVariant & id, const QImage & image);
        bool isEmpty() const;
        const QVariant & last() const;
        int lastIndexOf(const QVariant & id, int from = -1);
        qreal position() const;
        void prepend(const QVariant & id);
        void prepend(const QVariant & id, const QImage & image);
        void removeAt(int i);
        void requestUpdate(int i);
        void setBackgroundColor(const QColor & color);
        void setTitle(const QString & title);
        QString title() const;
        void update(int i, const QImage & image);

    public slots:
        void goTo(int i);
        void next(int delta = 1);
        void previous(int delta = 1);

    signals:
        void requiresUpdate(int i);
        void selected(int i);
        void updated();
        void emptinessChanged(bool e);

    protected:
        void adjustCurrent(int i, int offset);

    private:
        FlowBrowserModelPrivate * d;
        friend class FlowBrowser;
        friend class FlowBrowserPrivate;
    };




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // FlowBrowser ////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

	class FlowBrowserPrivate;
	class LIBUTOPIA_QT_API FlowBrowser : public QGLWidget
	{
		Q_OBJECT

		Q_PROPERTY(QColor defaultBackgroundColor READ defaultBackgroundColor WRITE setDefaultBackgroundColor)
		Q_PROPERTY(qreal separation READ separation WRITE setSeparation)
		Q_PROPERTY(qreal offset READ offset WRITE setOffset)

    public:
        // Constructor
        FlowBrowser(QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);
        FlowBrowser(QGLContext * context, QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);
        FlowBrowser(const QGLFormat & format, QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);
        ~FlowBrowser();

        FlowBrowserModel * addModel(const QString & title);
        const QColor & defaultBackgroundColor() const;
        void clear();
        FlowBrowserModel * currentModel() const;
        bool isEmpty() const;
        qreal offset() const;
        qreal separation() const;
        void setCurrentModel(FlowBrowserModel * model);
        void setDefaultBackgroundColor(const QColor & color);
        void setOffset(qreal offset);
        void setSeparation(qreal separation);
        using QGLWidget::update;

    protected:
        int indexAt(int x, int y);
        void initializeGL();
        void mouseDoubleClickEvent(QMouseEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void paintGL();
        void resizeGL(int w, int h);
        void wheelEvent(QWheelEvent * event);

    protected slots:
        void applyTexture();

    private:
        boost::scoped_ptr< FlowBrowserPrivate > d;

        void init();
	};

} // namespace Utopia

#endif // Utopia_QT_FLOWBROWSER
