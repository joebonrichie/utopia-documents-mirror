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

#include "networkingpreferencespane.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QRadioButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>

NetworkingPreferencesPane::NetworkingPreferencesPane(QWidget * parent, Qt::WindowFlags f)
    : Utopia::PreferencesPane(parent, f)
{
    QVBoxLayout * dialogLayout = new QVBoxLayout(this);
    QGroupBox * proxyGroupBox = new QGroupBox("Configure proxies to access the Internet");
    dialogLayout->addWidget(proxyGroupBox);
    QVBoxLayout * optionsLayout = new QVBoxLayout(proxyGroupBox);
    optionsLayout->setSpacing(8);
    noProxyRadioButton = new QRadioButton("No proxy");
    connect(noProxyRadioButton, SIGNAL(toggled(bool)), this, SLOT(onOptionChanged(bool)));
    noProxyRadioButton->setProperty("proxy", "NONE");
    optionsLayout->addWidget(noProxyRadioButton);
    systemProxyRadioButton = new QRadioButton("Use system settings");
    connect(systemProxyRadioButton, SIGNAL(toggled(bool)), this, SLOT(onOptionChanged(bool)));
    systemProxyRadioButton->setProperty("proxy", "SYSTEM");
    optionsLayout->addWidget(systemProxyRadioButton);
    manualProxyRadioButton = new QRadioButton("Manual configuration:");
    connect(manualProxyRadioButton, SIGNAL(toggled(bool)), this, SLOT(onOptionChanged(bool)));
    manualProxyRadioButton->setProperty("proxy", "MANUAL");
    optionsLayout->addWidget(manualProxyRadioButton);
    proxyWidget = new QWidget;
    optionsLayout->addWidget(proxyWidget);
    QGridLayout * proxyLayout = new QGridLayout(proxyWidget);
    proxyLayout->setSpacing(4);
    proxyLayout->setContentsMargins(20, 4, 0, 4);
    proxyLayout->setColumnStretch(0, 0);
    proxyLayout->setColumnStretch(1, 1);
    proxyLayout->setColumnStretch(2, 0);
    proxyLayout->setColumnStretch(3, 0);
    httpHostLineEdit = new QLineEdit;
    httpHostLineEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+"), httpHostLineEdit));
    connect(httpHostLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onHttpHostChanged(const QString &)));
    httpPortSpinBox = new QSpinBox;
    httpPortSpinBox->setRange(0, 65535);
    httpPortSpinBox->setSpecialValueText(" ");
    httpPortSpinBox->setFixedWidth(70);
    httpPortSpinBox->setAlignment(Qt::AlignRight);
    connect(httpPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHttpPortChanged(int)));
    proxyLayout->addWidget(new QLabel("HTTP Proxy:"), 0, 0, Qt::AlignRight);
    proxyLayout->addWidget(httpHostLineEdit, 0, 1);
    proxyLayout->addWidget(new QLabel("Port:"), 0, 2, Qt::AlignRight);
    proxyLayout->addWidget(httpPortSpinBox, 0, 3);
    overrideCheckBox = new QCheckBox("Use the above proxy for all connections");
    connect(overrideCheckBox, SIGNAL(toggled(bool)), this, SLOT(onGlobalProxyChanged(bool)));
    proxyLayout->addWidget(overrideCheckBox, 1, 1, 1, 3);
    httpsHostLineEdit = new QLineEdit;
    httpsHostLineEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+"), httpsHostLineEdit));
    connect(httpsHostLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onHttpsHostChanged(const QString &)));
    httpsPortSpinBox = new QSpinBox;
    httpsPortSpinBox->setRange(0, 65535);
    httpsPortSpinBox->setSpecialValueText(" ");
    httpsPortSpinBox->setFixedWidth(70);
    httpsPortSpinBox->setAlignment(Qt::AlignRight);
    connect(httpsPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHttpsPortChanged(int)));
    httpsProxyHostLabel = new QLabel("SSL Proxy:");
    proxyLayout->addWidget(httpsProxyHostLabel, 2, 0, Qt::AlignRight);
    proxyLayout->addWidget(httpsHostLineEdit, 2, 1);
    httpsProxyPortLabel = new QLabel("Port:");
    proxyLayout->addWidget(httpsProxyPortLabel, 2, 2, Qt::AlignRight);
    proxyLayout->addWidget(httpsPortSpinBox, 2, 3);
    ftpHostLineEdit = new QLineEdit;
    ftpHostLineEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+"), ftpHostLineEdit));
    connect(ftpHostLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onFtpHostChanged(const QString &)));
    ftpPortSpinBox = new QSpinBox;
    ftpPortSpinBox->setRange(0, 65535);
    ftpPortSpinBox->setSpecialValueText(" ");
    ftpPortSpinBox->setFixedWidth(70);
    ftpPortSpinBox->setAlignment(Qt::AlignRight);
    connect(ftpPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onFtpPortChanged(int)));
    ftpProxyHostLabel = new QLabel("FTP Proxy:");
    proxyLayout->addWidget(ftpProxyHostLabel, 3, 0, Qt::AlignRight);
    proxyLayout->addWidget(ftpHostLineEdit, 3, 1);
    ftpProxyPortLabel = new QLabel("Port:");
    proxyLayout->addWidget(ftpProxyPortLabel, 3, 2, Qt::AlignRight);
    proxyLayout->addWidget(ftpPortSpinBox, 3, 3);
    proxyLayout->addItem(new QSpacerItem(8, 8), 4, 1, 1, 3);
    proxyLayout->addWidget(new QLabel("Exclude proxy for:"), 5, 0, Qt::AlignRight);
    excludeLineEdit = new QLineEdit;
    excludeLineEdit->setValidator(new QRegExpValidator(QRegExp("(\\s*[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+\\s*)([,;]\\s*[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+\\s*)+"), excludeLineEdit));
    connect(excludeLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onExcludeChanged(const QString &)));
    proxyLayout->addWidget(excludeLineEdit, 5, 1, 1, 3);
    proxyLayout->addWidget(new QLabel("<span>Example:</span> <em>example.com, 192.168.0.0</em>"), 6, 1, 1, 3);
    autoProxyRadioButton = new QRadioButton("Automatic configuration URL:");
    connect(autoProxyRadioButton, SIGNAL(toggled(bool)), this, SLOT(onOptionChanged(bool)));
    autoProxyRadioButton->setProperty("proxy", "AUTO");
    optionsLayout->addWidget(autoProxyRadioButton);
    autoProxyUrlLineEdit = new QLineEdit;
    connect(autoProxyUrlLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onAutoConfUrlChanged(const QString &)));
    QVBoxLayout * autoProxyUrlLayout = new QVBoxLayout;
    autoProxyUrlLayout->addWidget(autoProxyUrlLineEdit);
    autoProxyUrlLayout->setContentsMargins(20, 0, 0, 0);
    optionsLayout->addLayout(autoProxyUrlLayout);
    dialogLayout->addStretch(1);

    load();
}

bool NetworkingPreferencesPane::apply()
{
    save();
    return true;
}

void NetworkingPreferencesPane::discard()
{
    load();
}

QIcon NetworkingPreferencesPane::icon() const
{
    return QIcon(":/preferences/networking/icon.png");
}

void NetworkingPreferencesPane::load()
{
    // Populate from settings
    QSettings conf;
    conf.sync();
    conf.beginGroup("Networking");
    conf.beginGroup("Proxies");

    config["Method"] = conf.value("Method");
    config["HTTP Proxy"] = conf.value("HTTP Proxy");
    config["HTTPS Proxy"] = conf.value("HTTPS Proxy");
    config["FTP Proxy"] = conf.value("FTP Proxy");
    config["Use HTTP Proxy For All Protocols"] = conf.value("Use HTTP Proxy For All Protocols");
    config["Exclude List"] = conf.value("Exclude List");
    config["PAC"] = conf.value("PAC");

    noProxyRadioButton->setChecked(config["Method"].toString() == "NONE");
    systemProxyRadioButton->setChecked(config["Method"].toString() == "SYSTEM");
    manualProxyRadioButton->setChecked(config["Method"].toString() == "MANUAL");
    autoProxyRadioButton->setChecked(config["Method"].toString() == "AUTO");

    httpHostLineEdit->setText(config["HTTP Proxy"].toString().section(":", 0, 0).trimmed());
    httpPortSpinBox->setValue(config["HTTP Proxy"].toString().section(":", 1, 1).toInt());
    httpsHostLineEdit->setText(config["HTTPS Proxy"].toString().section(":", 0, 0).trimmed());
    httpsPortSpinBox->setValue(config["HTTPS Proxy"].toString().section(":", 1, 1).toInt());
    ftpHostLineEdit->setText(config["FTP Proxy"].toString().section(":", 0, 0).trimmed());
    ftpPortSpinBox->setValue(config["FTP Proxy"].toString().section(":", 1, 1).toInt());

    overrideCheckBox->setChecked(config["Use HTTP Proxy For All Protocols"].toBool());

    excludeLineEdit->setText(config["Exclude List"].toString().trimmed());

    autoProxyUrlLineEdit->setText(config["PAC"].toString());
}

void NetworkingPreferencesPane::onAutoConfUrlChanged(const QString & text)
{
    setValue("PAC", text);
}

void NetworkingPreferencesPane::onExcludeChanged(const QString & text)
{
    setValue("Exclude List", text);
}

void NetworkingPreferencesPane::onFtpHostChanged(const QString & text)
{
    setValue("FTP Proxy", QString("%1:%2").arg(text).arg(ftpPortSpinBox->value()));
}

void NetworkingPreferencesPane::onFtpPortChanged(int port)
{
    setValue("FTP Proxy", QString("%1:%2").arg(ftpHostLineEdit->text().trimmed()).arg(port));
}

void NetworkingPreferencesPane::onGlobalProxyChanged(bool toggled)
{
    httpsHostLineEdit->setEnabled(!toggled);
    httpsPortSpinBox->setEnabled(!toggled);
    httpsProxyHostLabel->setEnabled(!toggled);
    httpsProxyPortLabel->setEnabled(!toggled);
    ftpHostLineEdit->setEnabled(!toggled);
    ftpPortSpinBox->setEnabled(!toggled);
    ftpProxyHostLabel->setEnabled(!toggled);
    ftpProxyPortLabel->setEnabled(!toggled);

    setValue("Use HTTP Proxy For All Protocols", toggled);
}

void NetworkingPreferencesPane::onHttpHostChanged(const QString & text)
{
    setValue("HTTP Proxy", QString("%1:%2").arg(text.trimmed()).arg(httpPortSpinBox->value()));
}

void NetworkingPreferencesPane::onHttpPortChanged(int port)
{
    setValue("HTTP Proxy", QString("%1:%2").arg(httpHostLineEdit->text().trimmed()).arg(port));
}

void NetworkingPreferencesPane::onHttpsHostChanged(const QString & text)
{
    setValue("HTTPS Proxy", QString("%1:%2").arg(text.trimmed()).arg(httpsPortSpinBox->value()));
}

void NetworkingPreferencesPane::onHttpsPortChanged(int port)
{
    setValue("HTTPS Proxy", QString("%1:%2").arg(httpsHostLineEdit->text().trimmed()).arg(port));
}

void NetworkingPreferencesPane::onOptionChanged(bool toggled)
{
    if (toggled)
    {
        QString option = sender()->property("proxy").toString();

        proxyWidget->setEnabled(option == "MANUAL");
        autoProxyUrlLineEdit->setEnabled(option == "AUTO");

        setValue("Method", option);
    }
}

void NetworkingPreferencesPane::save()
{
    // Save to settings
    QSettings conf;
    conf.sync();
    conf.beginGroup("Networking");
    conf.beginGroup("Proxies");

    conf.setValue("Method", config.value("Method"));
    conf.setValue("HTTP Proxy", config.value("HTTP Proxy"));
    conf.setValue("HTTPS Proxy", config.value("HTTPS Proxy"));
    conf.setValue("FTP Proxy", config.value("FTP Proxy"));
    conf.setValue("Use HTTP Proxy For All Protocols", config.value("Use HTTP Proxy For All Protocols"));
    conf.setValue("Exclude List", config.value("Exclude List"));
    conf.setValue("PAC", config.value("PAC"));
}

void NetworkingPreferencesPane::setValue(const QString & key, const QVariant & value)
{
    if (config[key] != value) {
        config[key] = value;
        setModified(true);
    }
}

QString NetworkingPreferencesPane::title() const
{
    return "Networking";
}

int NetworkingPreferencesPane::weight() const
{
    return 0;
}




NetworkingPreferencesDefaults::NetworkingPreferencesDefaults()
    : Utopia::Initializer()
{}

void NetworkingPreferencesDefaults::init()
{
    QSettings conf;
    conf.beginGroup("Networking");
    conf.beginGroup("Proxies");
    if (!conf.contains("Method")) {
        conf.setValue("Method", QString("SYSTEM"));
    }
}

QString NetworkingPreferencesDefaults::description()
{
    return "Networking defaults";
}

