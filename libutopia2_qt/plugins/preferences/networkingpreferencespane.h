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

#include <utopia2/initializer.h>
#include <utopia2/qt/preferencespane.h>

#include <QVariantMap>

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QWidget;

class NetworkingPreferencesPane : public Utopia::PreferencesPane
{
    Q_OBJECT

public:
    NetworkingPreferencesPane(QWidget * parent = 0, Qt::WindowFlags f = 0);

    bool apply();
    void discard();
    QIcon icon() const;
    QString title() const;
    int weight() const;

protected slots:
    void onAutoConfUrlChanged(const QString & text);
    void onExcludeChanged(const QString & text);
    void onFtpHostChanged(const QString & text);
    void onFtpPortChanged(int port);
    void onGlobalProxyChanged(bool toggled);
    void onHttpHostChanged(const QString & text);
    void onHttpPortChanged(int port);
    void onHttpsHostChanged(const QString & text);
    void onHttpsPortChanged(int port);
    void onOptionChanged(bool toggled);

protected:
    QCheckBox * overrideCheckBox;
    QRadioButton * noProxyRadioButton;
    QRadioButton * systemProxyRadioButton;
    QRadioButton * manualProxyRadioButton;
    QRadioButton * autoProxyRadioButton;
    QLineEdit * httpHostLineEdit;
    QSpinBox * httpPortSpinBox;
    QLineEdit * httpsHostLineEdit;
    QSpinBox * httpsPortSpinBox;
    QLabel * httpsProxyHostLabel;
    QLabel * httpsProxyPortLabel;
    QLineEdit * ftpHostLineEdit;
    QSpinBox * ftpPortSpinBox;
    QLabel * ftpProxyHostLabel;
    QLabel * ftpProxyPortLabel;
    QLineEdit * excludeLineEdit;
    QLineEdit * autoProxyUrlLineEdit;
    QWidget * proxyWidget;

    QVariantMap config;

    void load();
    void save();
    void setValue(const QString & key, const QVariant & value);
};

class NetworkingPreferencesDefaults : public Utopia::Initializer
{
public:
    // Constructor
    NetworkingPreferencesDefaults();

    // Initialize!
    void init();
    QString description();

};
