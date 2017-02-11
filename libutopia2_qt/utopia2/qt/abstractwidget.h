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

#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

#include <utopia2/qt/config.h>
#include <utopia2/extension.h>

#include <utopia2/node.h>

#include <QObject>
#include <QString>
#include <QWidget>

#include <QtDebug>

namespace Utopia
{

    class LIBUTOPIA_QT_API AbstractWidgetInterface
    {
    public:
        virtual ~AbstractWidgetInterface() {};

        // Interface
        virtual bool load(Node * model) = 0;
        virtual bool supports(Node * model) const = 0;

    };

    class LIBUTOPIA_QT_API AbstractWidget : public QObject, public AbstractWidgetInterface
    {
        Q_OBJECT

    public:
        AbstractWidget();
        virtual ~AbstractWidget() {};

        // Helpers
        virtual QWidget * asWidget() const = 0;

    public slots:
        virtual bool load(Node * model) = 0;
        virtual bool supports(Node * model) const = 0;

    protected slots:
        virtual void subordinateDestroyed() = 0;

    };

    template< class WidgetClass >
    class LIBUTOPIA_QT_API AbstractWidgetBackend : public AbstractWidget
    {
    public:
        typedef AbstractWidget API;

        AbstractWidgetBackend(WidgetClass * widget = new WidgetClass)
            : AbstractWidget(), _widget(widget)
        {
            connect(widget, SIGNAL(destroyed()), this, SLOT(subordinateDestroyed()));
        }

        ~AbstractWidgetBackend()
        {
            if (this->_widget)
            {
                QWidget * doomed = this->_widget;
                this->_widget = 0;
                delete doomed;
            }
        }

        // Interface
        bool load(Node * model)
        {
            if (this->_widget)
            {
                return this->_widget->load(model);
            }
            else
            {
                return false;
            }
        }

        bool supports(Node * model) const
        {
            if (this->_widget)
            {
                return this->_widget->supports(model);
            }
            else
            {
                return false;
            }
        }

        void subordinateDestroyed()
        {
            if (this->_widget)
            {
                this->_widget = 0;
                this->deleteLater();
            }
        }

        // Helpers
        QWidget * asWidget() const
        {
            return static_cast< QWidget * >(this->_widget);
        }

    private:
        // WIdget
        WidgetClass * _widget;
    };

} // namespace Utopia

UTOPIA_DECLARE_EXTENSION_CLASS(LIBUTOPIA_QT, Utopia::AbstractWidget)

#endif // ABSTRACTWIDGET_H
