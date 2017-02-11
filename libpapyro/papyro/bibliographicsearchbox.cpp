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

#include <papyro/bibliographicsearchbox_p.h>
#include <papyro/bibliographicsearchbox.h>

#include <utopia2/qt/hidpi.h>

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QToolButton>

#include <QDebug>

Q_DECLARE_METATYPE(Athenaeum::BibliographicSearchBox::SearchDomain);

namespace Athenaeum
{

    BibliographicSearchBoxPrivate::BibliographicSearchBoxPrivate(QObject * parent)
        : QObject(parent), searchDomain(BibliographicSearchBox::InvalidSearch), searchDelay(500)
    {}

    BibliographicSearchBoxPrivate::~BibliographicSearchBoxPrivate()
    {}

    bool BibliographicSearchBoxPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        if (obj == lineEdit) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent * ke = static_cast< QKeyEvent * >(event);
                if (ke->key() == Qt::Key_Backspace && lineEdit->cursorPosition() == 0 && searchDomain != BibliographicSearchBox::SearchAll) {
                    actionSearchAll->trigger();
                } else if (ke->key() == Qt::Key_Escape) {
                    if (lineEdit->text().isEmpty()) {
                        if (actionSearchAll->isChecked()) {
                            emit cancelRequested();
                        } else {
                            actionSearchAll->trigger();
                        }
                    } else {
                        lineEdit->clear();
                    }
                }
            }
            // The following would allow automatic domain setting through prepending a keyword+colon
            // but this is being removed in favour of a more expressive syntax (at some point in the
            // not-do-distant future)
            /* else if (event->type() == QEvent::KeyRelease) {
                QKeyEvent * ke = static_cast< QKeyEvent * >(event);
                if (ke->key() == Qt::Key_Colon) {
                    QMapIterator< QAction *, QStringList > iter(searchDomainActions);
                    while (iter.hasNext()) {
                        iter.next();
                        if (iter.value().contains(lineEdit->text().mid(0, lineEdit->cursorPosition() - 1).toLower())) {
                            lineEdit->setText(lineEdit->text().mid(lineEdit->cursorPosition()));
                            lineEdit->setCursorPosition(0);
                            iter.key()->trigger();
                            break;
                        }
                    }
                }
            } */
        }

        return QObject::eventFilter(obj, event);
    }

    void BibliographicSearchBoxPrivate::onCancelPressed()
    {
        bool wasEmpty = lineEdit->text().isEmpty();
        lineEdit->clear();
        if (wasEmpty) {
            actionSearchAll->trigger();
        } else {
            emit filterRequested(lineEdit->text(), searchDomain);
        }
    }

    void BibliographicSearchBoxPrivate::onReturnPressed()
    {
        emit filterRequested(lineEdit->text(), searchDomain);
        emit searchRequested(lineEdit->text(), searchDomain);
    }

    void BibliographicSearchBoxPrivate::onSearchDomainChanged(QAction * action)
    {
        BibliographicSearchBox::SearchDomain chosenDomain = action->property("searchDomain").value< BibliographicSearchBox::SearchDomain >();
        if (chosenDomain != searchDomain) {
            searchDomain = chosenDomain;
            switch (searchDomain) {
            case BibliographicSearchBox::SearchAll:
                searchDomainLabel->hide();
                break;
            case BibliographicSearchBox::SearchTitle:
            case BibliographicSearchBox::SearchAuthors:
            case BibliographicSearchBox::SearchAbstract:
                searchDomainLabel->setText(searchDomainActions.value(action).first() + ":");
                searchDomainLabel->show();
                break;
            default:
                // FIXME handle InvalidSearch?
                break;
            }
            emit searchDomainChanged(chosenDomain);
            timer.start(0);
        }
        toggleCancelButton();
    }

    void BibliographicSearchBoxPrivate::onTextChanged(const QString & text)
    {
        toggleCancelButton();
        timer.start(text.isEmpty() ? 0 : searchDelay);
    }

    void BibliographicSearchBoxPrivate::onTimeout()
    {
        emit filterRequested(lineEdit->text(), searchDomain);
    }

    void BibliographicSearchBoxPrivate::toggleCancelButton()
    {
        // Only show the clear button when the text isn't empty
        clearButton->setVisible(!lineEdit->text().isEmpty() || searchDomain != BibliographicSearchBox::SearchAll);
    }




    BibliographicSearchBox::BibliographicSearchBox(QWidget * parent)
        : QFrame(parent), d(new BibliographicSearchBoxPrivate(this))
    {
        // Buttons
        QHBoxLayout * buttonLayout = new QHBoxLayout(this);
        buttonLayout->setSpacing(0);
        buttonLayout->setContentsMargins(0, 0, 0, 0);

        d->searchDomainButton = new QToolButton;
        d->searchDomainButton->setIconSize(QSize(22, 14) * Utopia::hiDPIScaling());
        d->searchDomainButton->setCursor(Qt::ArrowCursor);
        d->searchDomainButton->setObjectName("searchDomainButton");
        d->searchDomainButton->setIcon(QPixmap(":/icons/all-search.png"));
        buttonLayout->addWidget(d->searchDomainButton, 0);

        d->searchDomainLabel = new QLabel;
        buttonLayout->addWidget(d->searchDomainLabel, 0, Qt::AlignCenter);
        d->searchDomainLabel->hide();

        d->lineEdit = new QLineEdit;
        d->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        d->lineEdit->installEventFilter(d);
        buttonLayout->addWidget(d->lineEdit, 1);
        setFocusProxy(d->lineEdit);

        d->clearButton = new QToolButton;
        d->clearButton->setIcon(QIcon(":/icons/clear-searchbox.png"));
        d->clearButton->setIconSize(QSize(16, 16) * Utopia::hiDPIScaling());
        d->clearButton->setCursor(Qt::ArrowCursor);
        //d->clearButton->setFlat(true);
        d->clearButton->setObjectName("clearButton");
        connect(d->clearButton, SIGNAL(clicked()), d, SLOT(onCancelPressed()));
        buttonLayout->addWidget(d->clearButton, 0, Qt::AlignCenter);
        d->clearButton->hide();

        // Populate search domain menu
        d->searchDomainMenu = new QMenu(this);
        QActionGroup * actionGroup = new QActionGroup(this);
        connect(actionGroup, SIGNAL(triggered(QAction*)), d, SLOT(onSearchDomainChanged(QAction*)));
        {
            d->actionSearchAll = new QAction("All", actionGroup);
            d->actionSearchAll->setProperty("searchDomain", QVariant::fromValue(BibliographicSearchBox::SearchAll));
            d->actionSearchAll->setCheckable(true);
            d->searchDomainMenu->addAction(d->actionSearchAll);
            d->actionSearchAll->trigger();
        }
        {
            QAction * action = new QAction("Title", actionGroup);
            action->setProperty("searchDomain", QVariant::fromValue(BibliographicSearchBox::SearchTitle));
            action->setCheckable(true);
            d->searchDomainMenu->addAction(action);
            d->searchDomainActions[action] << "title";
        }
        {
            QAction * action = new QAction("Authors", actionGroup);
            action->setProperty("searchDomain", QVariant::fromValue(BibliographicSearchBox::SearchAuthors));
            action->setCheckable(true);
            d->searchDomainMenu->addAction(action);
            d->searchDomainActions[action] << "authors" << "author" << "by";
        }
        {
            QAction * action = new QAction("Abstract", actionGroup);
            action->setProperty("searchDomain", QVariant::fromValue(BibliographicSearchBox::SearchAbstract));
            action->setCheckable(true);
            d->searchDomainMenu->addAction(action);
            d->searchDomainActions[action] << "abstract";
        }

        d->searchDomainButton->setMenu(d->searchDomainMenu);
        d->searchDomainButton->setPopupMode(QToolButton::InstantPopup);

        d->timer.setSingleShot(true);

        // Hook up signals
        connect(d, SIGNAL(cancelRequested()),
                this, SIGNAL(cancelRequested()));
        connect(d, SIGNAL(filterRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)),
                this, SIGNAL(filterRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)));
        connect(d, SIGNAL(searchDomainChanged(Athenaeum::BibliographicSearchBox::SearchDomain)),
                this, SIGNAL(searchDomainChanged(Athenaeum::BibliographicSearchBox::SearchDomain)));
        connect(d, SIGNAL(searchRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)),
                this, SIGNAL(searchRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)));
        connect(d->lineEdit, SIGNAL(textChanged(const QString &)),
                d, SLOT(onTextChanged(const QString &)));
        connect(d->lineEdit, SIGNAL(returnPressed()),
                d, SLOT(onReturnPressed()));
        connect(&d->timer, SIGNAL(timeout()), d, SLOT(onTimeout()));

        // Expand box for hiDPI
        setFixedHeight(d->lineEdit->sizeHint().height() * Utopia::hiDPIScaling());
    }

    BibliographicSearchBox::~BibliographicSearchBox()
    {}

    void BibliographicSearchBox::clear()
    {
        d->onCancelPressed();
    }

    void BibliographicSearchBox::filter(const QString & text)
    {
        d->lineEdit->setText(text);
    }

    void BibliographicSearchBox::search(const QString & text)
    {
        filter(text);
        d->onReturnPressed();
    }

    int BibliographicSearchBox::searchDelay() const
    {
        return d->searchDelay;
    }

    BibliographicSearchBox::SearchDomain BibliographicSearchBox::searchDomain() const
    {
        return d->searchDomain;
    }

    void BibliographicSearchBox::setSearchDelay(int msecs)
    {
        d->searchDelay = msecs;
    }

    void BibliographicSearchBox::setSearchDomain(SearchDomain domain)
    {
        foreach (QAction * action, d->searchDomainMenu->actions()) {
            if (action->property("searchDomain").value< BibliographicSearchBox::SearchDomain >() == domain) {
                action->trigger();
                break;
            }
        }
    }

    QString BibliographicSearchBox::text() const
    {
        return d->lineEdit->text();
    }

    void BibliographicSearchBox::resizeEvent(QResizeEvent * event)
    {
        setStyleSheet(QString("Papyro--BibliographicSearchBox { border-radius: %1px }").arg(event->size().height() / 2));
    }

} // namespace Athenaeum
