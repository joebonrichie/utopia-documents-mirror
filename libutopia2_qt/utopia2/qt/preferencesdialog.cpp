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

#include <utopia2/qt/preferencesdialog.h>
#include <utopia2/qt/preferencesdialog_p.h>
#include <utopia2/qt/preferencespane.h>
#include <utopia2/extension.h>

#include <boost/weak_ptr.hpp>

#include <QActionGroup>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalMapper>
#include <QStackedLayout>
#include <QToolBar>
#include <QVBoxLayout>

#include <set>

#include <QtDebug>

namespace Utopia
{

    PreferencesDialogPrivate::PreferencesDialogPrivate(PreferencesDialog * dialog)
        : QObject(dialog), dialog(dialog), previousPane(0)
    {}

    bool PreferencesDialogPrivate::apply()
    {
        PreferencesPane * pane = currentPane();
        bool success = !pane || !pane->isModified() || pane->apply();
        if (!success) {
            if (PreferencesPane * pane = currentPane()) {
                QString errorMessage = pane->errorMessage();
                if (!errorMessage.isEmpty()) {
                    // Show error message
                }
            }
        } else {
            pane->setModified(false);
        }
        return success;
    }

    PreferencesPane * PreferencesDialogPrivate::currentPane() const
    {
        QWidget * widget = stackedLayout->currentWidget();
        return qobject_cast< PreferencesPane * >(widget);
    }

    void PreferencesDialogPrivate::discard()
    {
        if (PreferencesPane * pane = currentPane()) {
            pane->discard();
            pane->setModified(false);
        }
    }

    void PreferencesDialogPrivate::onAccept()
    {
        PreferencesPane * pane = currentPane();
        if (pane && pane->isModified()) {
            switch (PreferencesDialog::requestModifiedAction()) {
            case QMessageBox::Apply:
                if (apply()) {
                    dialog->accept();
                }
                break;
            case QMessageBox::Discard:
                discard();
                dialog->reject();
                break;
            default:
                // Cancel
                break;
            }
        } else {
            dialog->accept();
        }
    }

    void PreferencesDialogPrivate::onApplyClicked()
    {
        // FIXME on error?
        apply();
    }

    void PreferencesDialogPrivate::onCurrentPaneChanged(QWidget * newPane)
    {
        bool allow = false;
        PreferencesPane * pane = currentPane();
        if (pane && pane->isModified()) {
            switch (PreferencesDialog::requestModifiedAction()) {
            case QMessageBox::Apply:
                if (apply() && newPane) {
                    allow = true;
                }
                break;
            case QMessageBox::Discard:
                discard();
                if (newPane) {
                    allow = true;
                }
                break;
            default:
                // Cancel and return to dirty pane
                if (previousPane) {
                    int idx = stackedLayout->indexOf(previousPane);
                    toolBar->actions().at(idx)->setChecked(true);
                }
                break;
            }
        } else if (newPane) {
            allow = true;
        }

        if (allow) {
            previousPane = pane;
            stackedLayout->setCurrentWidget(newPane);
        }
    }

    void PreferencesDialogPrivate::onDiscard()
    {
        discard();
        dialog->reject();
    }

    void PreferencesDialogPrivate::onModifiedChanged(bool modified)
    {
        applyButton->setEnabled(modified);
    }




    PreferencesDialog::PreferencesDialog(QWidget * parent, Qt::WindowFlags f)
        : QDialog(parent, f), d(new PreferencesDialogPrivate(this))
    {
        setWindowTitle("Preferences");

        // Layout
        QVBoxLayout * mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        d->toolBar = new QToolBar;
        d->toolBar->setFloatable(false);
        d->toolBar->setMovable(false);
        d->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        mainLayout->addWidget(d->toolBar, 0);
        d->stackedLayout = new QStackedLayout;
        d->stackedLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->addLayout(d->stackedLayout, 1);
        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch(1);
        d->applyButton = new QPushButton("Apply");
        d->applyButton->setEnabled(false);
        connect(d->applyButton, SIGNAL(clicked()), d, SLOT(onApplyClicked()));
        buttonLayout->addWidget(d->applyButton);
        QPushButton * closeButton = new QPushButton("Close");
        closeButton->setDefault(true);
        buttonLayout->addWidget(closeButton);
        buttonLayout->setContentsMargins(12, 4, 12, 12);
        buttonLayout->setSpacing(8);
        connect(closeButton, SIGNAL(clicked()), d, SLOT(onAccept()));
        mainLayout->addLayout(buttonLayout, 0);

        // Control
        QSignalMapper * mapper = new QSignalMapper(this);
        connect(mapper, SIGNAL(mapped(QWidget*)), d, SLOT(onCurrentPaneChanged(QWidget*)));

        // Discover preference panes
        QActionGroup * actionGroup = new QActionGroup(this);
        actionGroup->setExclusive(true);
        QList< PreferencesPane * > panes;
        foreach (PreferencesPane * pane, instantiateAllExtensions< PreferencesPane >()) {
            if (panes.size() == 0) {
                panes.append(pane);
            } else {
                if (pane->isValid()) {
                    int place = 0;
                    foreach (PreferencesPane * existing, panes) {
                        if (pane->weight() > existing->weight() || (pane->weight() == existing->weight() && pane->title() > existing->title())) {
                            break;
                        }
                        ++place;
                    }
                    panes.insert(place, pane);
                } else {
                    delete pane;
                }
            }
        }
        foreach (PreferencesPane * pane, panes)
        {
            QIcon icon(pane->icon());
            QString title(pane->title());
            QAction * action = d->toolBar->addAction(icon, title, mapper, SLOT(map()));
            action->setCheckable(true);
            if (pane == panes.first()) {
                action->setChecked(true);
                d->previousPane = pane;
            }
            actionGroup->addAction(action);
            mapper->setMapping(action, pane);
            d->stackedLayout->addWidget(pane);
            connect(pane, SIGNAL(modifiedChanged(bool)), d, SLOT(onModifiedChanged(bool)));
        }

        // Adjust size
        resize(minimumSizeHint());
    }

    PreferencesDialog::~PreferencesDialog()
    {}

    bool PreferencesDialog::focusPane(const QString & paneName, const QVariant & params)
    {
        bool success = false;
        if (d->stackedLayout && !paneName.isEmpty()) {
            foreach (QAction * action, d->toolBar->actions()) {
                if (action->text().toLower() == paneName.toLower()) {
                    action->trigger();
                    success = true;
                }
            }
            if (success) {
                for (int i = 0; i < d->stackedLayout->count(); ++i) {
                    if (PreferencesPane * pane = qobject_cast< PreferencesPane * >(d->stackedLayout->widget(i))) {
                        if (pane->title().toLower() == paneName.toLower()) {
                            pane->show(params);
                        }
                    }
                }
            }
        }
        return success;
    }

    boost::shared_ptr< PreferencesDialog > PreferencesDialog::instance()
    {
        static boost::weak_ptr< PreferencesDialog > singleton;
        boost::shared_ptr< PreferencesDialog > shared(singleton.lock());
        if (singleton.expired()) {
            shared = boost::shared_ptr< PreferencesDialog >(new PreferencesDialog());
            singleton = shared;
        }
        return shared;
    }

    QMessageBox::Button PreferencesDialog::requestModifiedAction(const QString & message, QWidget * parent)
    {
        static QString defaultMessage(
            "There are unapplied changes to these preferences. You can "
            "choose to apply or discard the changes, or cancel the "
            "request to check things over."
            );

        return QMessageBox::warning(parent ? parent : instance().get(),
            "Changes to apply...",
            message.isEmpty() ? defaultMessage : message,
            QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Apply,
            QMessageBox::Apply);
    }

}
