/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <utopia2/qt/updatewidget.h>

#include <utopia2/global.h>

#include "utopia2/qt/ui_updatewidget.h"
#include "utopia2/qt/ui_updatewidgetchecking.h"
#include "utopia2/qt/ui_updatewidgetuptodate.h"
#include "utopia2/qt/ui_updatewidgeterror.h"
#include "utopia2/qt/ui_updatewidgetpending.h"

#include <QBuffer>
#include <QDateTime>
#include <QDomDocument>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QSignalMapper>

#include <QtDebug>

#define UPDATEWIDGET_DATETIME_FORMAT "ddd d MMM yy, hh:mm"
#define UPDATEREPORT_FORMAT_VERSION "1.2"

namespace Utopia
{

    UpdateWidget::UpdateWidget(QWidget * parent, Qt::WindowFlags f)
        : QDialog(parent, f), _checking(0)
    {
        this->_ui = new Ui::UpdateWidget;
        this->_uiChecking = new Ui::UpdateWidgetChecking;
        this->_uiUpToDate = new Ui::UpdateWidgetUpToDate;
        this->_uiError = new Ui::UpdateWidgetError;
        this->_uiPending = new Ui::UpdateWidgetPending;

        // Set up UI
        this->_ui->setupUi(this);
        this->_ui->skipButton->hide();
        this->_ui->lastCheckedLabel->setEnabled(false);
        QSettings settings;
        if (settings.contains("Software Update/lastChecked"))
        {
            this->_ui->lastCheckedLabel->setText(settings.value("Software Update/lastChecked").toDateTime().toString(UPDATEWIDGET_DATETIME_FORMAT));
        }
        else
        {
            this->_ui->lastCheckedLabel->setText("");
        }

        // Set up spinner pane
        this->_checking = new QWidget(this);
        this->_uiChecking->setupUi(this->_checking);
        // Insert into layout
        this->_ui->verticalLayout->insertWidget(0, this->_checking, 1);
        this->_checking->hide();

        // Set up success pane
        this->_upToDate = new QWidget(this);
        this->_uiUpToDate->setupUi(this->_upToDate);
        // Insert into layout
        this->_ui->verticalLayout->insertWidget(0, this->_upToDate, 1);
        this->_upToDate->hide();

        // Set up error pane
        this->_error = new QWidget(this);
        this->_uiError->setupUi(this->_error);
        // Insert into layout
        this->_ui->verticalLayout->insertWidget(0, this->_error, 1);
        this->_error->hide();

        // Set up pending pane
        this->_pending = new QWidget(this);
        this->_uiPending->setupUi(this->_pending);
        // Insert into layout
        this->_ui->verticalLayout->insertWidget(0, this->_pending, 1);
        this->_pending->hide();

        // Close and check when button press
        connect(this->_ui->checkNowButton, SIGNAL(clicked()), this, SLOT(check()));
        connect(this->_ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
        connect(this->_ui->skipButton, SIGNAL(clicked()), this, SLOT(skip()));

        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
        this->_currentVersion = Utopia::versionString();
    }

    UpdateWidget::~UpdateWidget()
    {
        delete this->_ui;
        delete this->_uiChecking;
        delete this->_uiUpToDate;
        delete this->_uiError;
        delete this->_uiPending;
    }

    bool lessThan(const QString & lhs_string, const QString & rhs_string)
    {
        QStringList lhs_list(lhs_string.split("."));
        QStringList rhs_list(rhs_string.split("."));

        int idx = 0;
        while (true)
        {
            QString lhs(idx < lhs_list.size() ? lhs_list.at(idx) : QString());
            QString rhs(idx < rhs_list.size() ? rhs_list.at(idx) : QString());

            if (!lhs.isEmpty() && !rhs.isEmpty())
            {
                if (lhs < rhs) return true;
                else if (lhs > rhs) return false;
            }
            else if (lhs.isEmpty() && !rhs.isEmpty())
            {
                return true;
            }
            else if (!lhs.isEmpty() && rhs.isEmpty())
            {
                return false;
            }
            else if (lhs.isEmpty() && rhs.isEmpty())
            {
                break;
            }
            ++idx;
        }

        return false;
    }

    UpdateWidget::UpdateStatus UpdateWidget::check()
    {
        // Get defaults
        QVariantMap defaults(Utopia::defaults());
        QString defaultUrl = defaults.value("updates").toString();

        if (defaultUrl != "prevent") {
            // Disable buttons
            this->_ui->checkNowButton->setEnabled(false);
            this->_ui->closeButton->setEnabled(false);
            this->_ui->skipButton->hide();
            this->_ui->lastCheckedLabel->setText("Checking now...");

            // Hide other panes
            this->_pending->hide();
            this->_error->hide();
            this->_upToDate->hide();
            // Set up spinner pane
            this->_checking->show();
            this->_uiChecking->spinner->start();

            // Network stuff for getting update information
            QUrl url(defaultUrl);
            if (url.isValid()) {
                QList< QPair< QString, QString > > params;
                params << QPair< QString, QString >("v", UPDATEREPORT_FORMAT_VERSION);
                url.setQueryItems(params);
                QNetworkRequest request(url);
                QNetworkReply * reply = networkAccessManager()->get(request);
                QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
            }
        }

        return this->_status;
    }

    void UpdateWidget::finished()
    {
        QNetworkReply * reply = static_cast< QNetworkReply * >(sender());

        reply->deleteLater();

        this->_status = update_error_unknown;

        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray replyData(reply->readAll());
            QString errStr;
            int errLine, errColumn;
            // Deal with result
            QDomDocument doc;
            if (doc.setContent(replyData, &errStr, &errLine, &errColumn))
            {
                QDomElement root = doc.documentElement();
                this->_status = update_error_format;
                if (root.tagName() == "latest" &&
                    root.attribute("xmlns") == "http://utopia.cs.manchester.ac.uk/softwareupdate/" &&
                    root.attribute("version") == UPDATEREPORT_FORMAT_VERSION)
                {
                    // Looks ok so far!

                    QString id("utopia.documents");

                    // Get each item, check for right id
                    QDomNodeList items = root.elementsByTagName("component");
                    for (int i = 0; i < items.count(); ++i) {
                        // Only accept the required id
                        QDomElement element(items.item(i).toElement());
                        if (element.attribute("id") == id) {
                            this->_status = update_ok;

                            QSettings settings;
                            QDateTime lastUpdated;
                            if (settings.contains("Software Update/lastUpdated"))
                            {
                                lastUpdated = settings.value("Software Update/lastUpdated").toDateTime();
                            }

                            QDateTime itemLastUpdated = QDateTime::fromString(element.attribute("lastUpdated"), "yyyyMMddHHmmss");
                            QString itemName = element.firstChildElement("name").text();
                            QString itemVersion = this->_itemVersion = element.firstChildElement("version").text();
                            QString itemUrl = element.firstChildElement("url").text();

                            if (itemLastUpdated.isValid() && !itemName.isEmpty() && !itemVersion.isEmpty() && !itemUrl.isEmpty())
                            {
                                if (lessThan(this->_currentVersion, itemVersion) && itemVersion != settings.value("Software Update/skipVersion").toString()) // Check if there is something pending
                                {
                                    this->_status = update_pending;
                                    QString msg("<p>An update of this software is available (v"+itemVersion+").</p>");

#ifdef Q_OS_LINUX
                                    msg+="<p>Install using your system package manager.</p>";
#else
                                    msg+="<p>Download from: <a href=\""+itemUrl+"\">"+itemUrl+"</a></p>";
#endif

                                    this->_uiPending->label->setText(msg);
                                    this->_ui->skipButton->show();
                                }

                                // Remember last check date!
                                QDateTime now = QDateTime::currentDateTime();
                                settings.setValue("Software Update/lastChecked", now);
                                this->_ui->lastCheckedLabel->setText("Last checked: " + now.toString(UPDATEWIDGET_DATETIME_FORMAT));
                            }
                        }
                    }
                }
            } else {
                qDebug() << "----" << errStr << errLine << errColumn;
            }
        }
        else
        {
            this->_status = update_error_network;
        }

        // Enable buttons
        this->_ui->checkNowButton->setEnabled(true);
        this->_ui->closeButton->setEnabled(true);

        switch (this->_status)
        {
        case update_pending:
            // Show correct pane
            this->_checking->hide();
            this->_pending->show();
            this->exec();

            break;
        case update_ok:
            // Show correct pane
            this->_checking->hide();
            this->_upToDate->show();

            break;
        case update_error_format:
        case update_error_unknown:
        case update_error_network:
        default:
            // Provide link to website in case something has gone wrong!
            this->_checking->hide();
            this->_error->show();

            break;
        }
    }

    void UpdateWidget::skip()
    {
        // Save the version to skip
        QSettings settings;
        settings.setValue("Software Update/skipVersion", this->_itemVersion);

        // Close widget
        this->lower();
        this->close();
    }

} // namespace Utopia
