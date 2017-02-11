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

#ifndef EMBEDDEDWIDGET_H
#define EMBEDDEDWIDGET_H

#include <utopia2/qt/config.h>

#include <QFrame>

class QEvent;
class QMouseEvent;
class QPaintEvent;

namespace Utopia
{

    class AbstractWidget;
    class Node;

    class LIBUTOPIA_QT_API HeadUpDisplay : public QWidget
    {
        Q_OBJECT

    public:
        HeadUpDisplay(QWidget * parent = 0, Qt::WindowFlags f = 0);
        ~HeadUpDisplay();

        bool eventFilter(QObject * obj, QEvent * event);

    signals:
        void visibilityToggled(bool);
        void hover(bool);

    protected:
        void enterEvent(QEvent * event);
        void leaveEvent(QEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);

    private:
        bool _visible;

    }; // HeadUpDisplay

    class LIBUTOPIA_QT_API EmbeddedWidget : public QFrame
    {
        Q_OBJECT

    public:
        // Constructor
        EmbeddedWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);
        EmbeddedWidget(Node * model, QWidget * parent = 0, Qt::WindowFlags f = 0);
        // Destructor
        ~EmbeddedWidget();

    public slots:
        // Set model for this
        void load(Node * model);
        void showData(bool visible);
        void hover(bool hover);

    private:
        // Previews available
        AbstractWidget * _abstractWidget;
        QString _abstractWidgetName;
        HeadUpDisplay * _hud;
        Node * _model;
        // Embedded / Hidden
        bool _visible;

        // Construction
        void initialise(Node * model);

    }; // class EmbeddedWidget

} // namespace FINDOMATIC

#endif // EMBEDDEDWIDGET_H
