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

#ifndef ATHENAEUM_BIBLIOGRAPHICSEARCHBOX_P_H
#define ATHENAEUM_BIBLIOGRAPHICSEARCHBOX_P_H

#include <papyro/bibliographicsearchbox.h>

#include <QMap>
#include <QObject>
#include <QTimer>

class QAction;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QToolButton;

namespace Athenaeum
{

    class BibliographicSearchBoxPrivate : public QObject
    {
        Q_OBJECT

    public:
        BibliographicSearchBoxPrivate(QObject * parent);
        virtual ~BibliographicSearchBoxPrivate();

        BibliographicSearchBox::SearchDomain searchDomain;
        QMenu * searchDomainMenu;
        QToolButton * searchDomainButton;
        QLabel * searchDomainLabel;
        QLineEdit * lineEdit;
        QToolButton * clearButton;
        QTimer timer;
        int searchDelay;
        QAction * actionSearchAll;
        QMap< QAction *, QStringList > searchDomainActions;
        int previousCursorPosition;

    public slots:
        void onCancelPressed();
        void onReturnPressed();
        void onSearchDomainChanged(QAction * action);
        void onTextChanged(const QString & text);
        void onTimeout();

    signals:
        void cancelRequested();
        void filterRequested(const QString & text, Athenaeum::BibliographicSearchBox::SearchDomain domain);
        void searchDomainChanged(Athenaeum::BibliographicSearchBox::SearchDomain domain);
        void searchRequested(const QString & text, Athenaeum::BibliographicSearchBox::SearchDomain domain);

    protected:
        bool eventFilter(QObject * obj, QEvent * event);
        void toggleCancelButton();
    }; // class BibliographicSearchBoxPrivate

} // namespace Athenaeum

#endif // ATHENAEUM_BIBLIOGRAPHICSEARCHBOX_P_H
