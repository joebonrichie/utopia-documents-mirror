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

#include <papyro/searchbar.h>
#include <utopia2/qt/hidpi.h>

#include <QAction>
#include <QShowEvent>

#include <QDebug>

namespace Papyro
{
    SearchBar::SearchBar(QWidget * parent)
        : QToolBar(parent)
    {
        setFloatable(false);
        setMovable(false);
        setIconSize(QSize(20, 20) * Utopia::hiDPIScaling());
		
        _resultsLabel = new QLabel("");
        QFont font(_resultsLabel->font());
        font.setPointSizeF(font.pointSizeF() * 0.8);
        _resultsLabel->setFont(font);
        QFontMetrics fm(font);
        _resultsLabel->setMinimumWidth(fm.width("99999 results"));
        _resultsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        _resultsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        _searchLineEdit = new QLineEdit;
        _searchLineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);
        _searchLineEdit->setMaximumWidth(240 * Utopia::hiDPIScaling());
        _searchLineEdit->setMinimumWidth(240 * Utopia::hiDPIScaling());

        {
            addAction(QPixmap(":/icons/remove.png").scaled(16 * Utopia::hiDPIScaling(), 16 * Utopia::hiDPIScaling(), Qt::KeepAspectRatio, Qt::SmoothTransformation),
                      "Close Search Bar", this, SLOT(hide()))->setShortcut(QKeySequence(Qt::Key_Escape));

            addWidget(_resultsLabel);

            _previousAction = addAction(QPixmap(":/icons/previous.png").scaled(16 * Utopia::hiDPIScaling(), 16 * Utopia::hiDPIScaling(), Qt::KeepAspectRatio, Qt::SmoothTransformation),
										"Find Previous", this, SIGNAL(previous()));
            _previousAction->setShortcut(QKeySequence::FindPrevious);
            _previousAction->setEnabled(false);
            widgetForAction(_previousAction)->setFixedSize(QSize(16 * Utopia::hiDPIScaling(), 16 * Utopia::hiDPIScaling()));

            _nextAction = addAction(QPixmap(":/icons/next.png").scaled(16 * Utopia::hiDPIScaling(), 16 * Utopia::hiDPIScaling(), Qt::KeepAspectRatio, Qt::SmoothTransformation),
									"Find Next", this, SIGNAL(next()));
            _nextAction->setShortcut(QKeySequence::FindPrevious);
            _nextAction->setEnabled(false);
            widgetForAction(_nextAction)->setFixedSize(QSize(16 * Utopia::hiDPIScaling(), 16 * Utopia::hiDPIScaling()));

            QWidget * sp1 = new QWidget;
            sp1->resize(6 * Utopia::hiDPIScaling(), 6 * Utopia::hiDPIScaling());
            addWidget(sp1);

            addWidget(_searchLineEdit);

            QWidget * sp2 = new QWidget;
            sp2->resize(6 * Utopia::hiDPIScaling(), 6 * Utopia::hiDPIScaling());
            addWidget(sp2);
        }

        //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

        _textUpdateTimer.setSingleShot(true);

        connect(_searchLineEdit, SIGNAL(textChanged(const QString)), this, SLOT(textEdited()));
        connect(&_textUpdateTimer, SIGNAL(timeout()), this, SLOT(timerComplete()));
    }

    void SearchBar::hideEvent(QHideEvent * event)
    {
        if (!event->spontaneous())
        {
            Q_EMIT clearSearch();
        }
    }

    void SearchBar::failed(int position)
    {
        QPalette p(_searchLineEdit->palette());
        p.setColor(QPalette::Base, QColor(255, 215, 215));
        _searchLineEdit->setPalette(p);
        _searchLineEdit->setCursorPosition(position);
    }

    void SearchBar::focus()
    {
        show();
        raise();
        _searchLineEdit->setFocus(Qt::OtherFocusReason);
        _searchLineEdit->selectAll();
        textEdited();
    }

    void SearchBar::searchReturned(int numberOfMatches)
    {
        QPalette p(_searchLineEdit->palette());
        p.setColor(QPalette::Base, QColor(255, 255, 255));
        _searchLineEdit->setPalette(p);

        // Update buttons and text
        _numberOfMatches = numberOfMatches;
        if (_numberOfMatches == 0)
        {
            _resultsLabel->setText("No results ");
            _previousAction->setEnabled(false);
            _nextAction->setEnabled(false);
        }
        else
        {
            if (_numberOfMatches == 1)
            {
                _resultsLabel->setText("1 result ");
                _previousAction->setEnabled(false);
                _nextAction->setEnabled(false);
            }
            else
            {
                _resultsLabel->setText(QString("%1 results ").arg(_numberOfMatches));
                _previousAction->setEnabled(true);
                _nextAction->setEnabled(true);
            }
        }

    }

    void SearchBar::showEvent(QShowEvent * event)
    {
        if (!event->spontaneous())
        {
            _resultsLabel->setText("");
            _searchLineEdit->setFocus(Qt::OtherFocusReason);
            _searchLineEdit->selectAll();
            textEdited();
        }
    }

    // Provide a delay on triggering a search
    void SearchBar::textEdited()
    {
        if (_searchLineEdit->text().isEmpty())
        {
            _resultsLabel->setText("");
            Q_EMIT searchForText(QString());
        }
        else
        {
            // If it's started a regexp, don't Q_EMIT until the regexp is finished
            // FIXME there should be something to show that it is a regexp, and to validate
            // the regexp
            QString term(_searchLineEdit->text());
            if (!term.startsWith('/') || (term.endsWith('/') && term.size() > 2))
            {
                _textUpdateTimer.start(term.size() > 3 ? 500 : 750);
            }
        }
    }

    void SearchBar::timerComplete()
    {
        qDebug() << "Requesting search";
        Q_EMIT searchForText(_searchLineEdit->text());
    }
}
