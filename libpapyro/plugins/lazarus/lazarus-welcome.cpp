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

#include <utopia2/qt/updatewidget.h>
#include <utopia2/qt/webview.h>
#include <utopia2/qt/hidpi.h>
#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>
#include <utopia2/configuration.h>

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPointer>
#include <QMetaObject>
#include <QPushButton>
#include <QVBoxLayout>

#include "lazarus-welcome.h"

//
// LazarusWelcomeInitializer
//

LazarusWelcomeInitializer::LazarusWelcomeInitializer()
    : Utopia::Initializer()
{
    welcomeWidget = new QDialog();
    welcomeWidget->setWindowFlags(welcomeWidget->windowFlags() | Qt::WindowStaysOnTopHint);
    welcomeWidget->resize(600 * Utopia::hiDPIScaling(), 500 * Utopia::hiDPIScaling());

    QVBoxLayout * layout = new QVBoxLayout(welcomeWidget);
    const int margin = 8 * Utopia::hiDPIScaling();
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->setSpacing(margin);
    Utopia::WebView * webView = new Utopia::WebView;
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(webView->page(), SIGNAL(linkClicked(QUrl)),
            this, SLOT(onLinkClicked(QUrl)));
    webView->load(QUrl("http://utopiadocs.com/lazarus/welcome"));
    layout->addWidget(webView);

    QDialogButtonBox * buttonBox = new QDialogButtonBox;

    buttonBox->addButton("No thanks", QDialogButtonBox::RejectRole);
    buttonBox->addButton("Yes please!", QDialogButtonBox::AcceptRole);

    connect(buttonBox, SIGNAL(rejected()), welcomeWidget, SLOT(reject()));
    connect(buttonBox, SIGNAL(accepted()), welcomeWidget, SLOT(accept()));

    layout->addWidget(buttonBox);
}

void LazarusWelcomeInitializer::init()
{
    if (Utopia::Configuration * configuration = Utopia::Configuration::instance(QUuid("{a5ab0e7d-0210-4be4-bd69-51f946e16ff2}"))) {
        bool permission = configuration->get("permission", false).toBool();
        bool nowelcome = configuration->get("nowelcome", false).toBool();

        if (!permission && !nowelcome) {
            this->setMessage("Launching welcome window...");

            bool accepted = (welcomeWidget->exec() == QDialog::Accepted);

            configuration->set("permission", accepted);

            if (!accepted) {
                // Let the user know it can be activated at any time
                // FIXME
                configuration->set("nowelcome", true);
            }
        }
    }
}

void LazarusWelcomeInitializer::final()
{
    delete welcomeWidget;
}

QString LazarusWelcomeInitializer::description()
{
    return "Lazarus welcome";
}

void LazarusWelcomeInitializer::onLinkClicked(QUrl url)
{
    QDesktopServices::openUrl(url);
}

