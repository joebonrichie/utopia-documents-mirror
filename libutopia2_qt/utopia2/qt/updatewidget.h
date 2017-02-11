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

#ifndef Utopia_QT_UPDATEWIDGET
#define Utopia_QT_UPDATEWIDGET

#include <utopia2/qt/config.h>

#include <utopia2/networkaccessmanager.h>

#include <QDialog>

class QEventLoop;

namespace Ui
{
    class UpdateWidget;
    class UpdateWidgetChecking;
    class UpdateWidgetUpToDate;
    class UpdateWidgetError;
    class UpdateWidgetPending;
}

namespace Utopia
{

    class LIBUTOPIA_QT_API UpdateWidget : public QDialog, public NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        typedef enum
        {
            update_ok,
            update_pending,
            update_error_format,
            update_error_network,
            update_error_unknown
        } UpdateStatus;

        // Constructor
        UpdateWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);
        // Destructor
        virtual ~UpdateWidget();

    public slots:
        UpdateStatus check();

    protected slots:
        void skip();

        void finished();

    private:
        Ui::UpdateWidget * _ui;
        Ui::UpdateWidgetChecking * _uiChecking;
        Ui::UpdateWidgetUpToDate * _uiUpToDate;
        Ui::UpdateWidgetError * _uiError;
        Ui::UpdateWidgetPending * _uiPending;

        QEventLoop * _checkingLoop;

        QWidget * _checking;
        QWidget * _pending;
        QWidget * _upToDate;
        QWidget * _error;

        QString _version;
        QString _currentVersion;

        UpdateStatus _status;

    }; // class UpdateWidget

} // namespace Utopia

#endif // Utopia_QT_UPDATEWIDGET
