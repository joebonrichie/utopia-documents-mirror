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

#ifndef UTOPIA_PACPROXYFACTORY_P_H
#define UTOPIA_PACPROXYFACTORY_P_H

#include <QDialog>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QNetworkProxyQuery>
#include <QPair>
#include <boost/scoped_ptr.hpp>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QWaitCondition>

class QLineEdit;
class QShowEvent;

namespace Utopia
{

    struct Attempt
    {
        QString user;
        QString password;
        int first;

        bool isValid()
        {
            return !user.isNull() && !password.isNull();
        }
    };

    class CredentialDialog : public QDialog
    {
    public:
        CredentialDialog();

        QLineEdit * userNameLineEdit;
        QLineEdit * passwordLineEdit;

    protected:
        void showEvent(QShowEvent * event);
    };

    class PACScript;

    class PACProxyFactory;
    class PACProxyFactoryPrivate : public QObject
    {
        Q_OBJECT

    public:
        PACProxyFactoryPrivate(PACProxyFactory * factory);
        ~PACProxyFactoryPrivate();

        PACProxyFactory * factory;
        PACScript * script;
        QUrl url;
        QMutex mutex;

        QMutex authMutex;
        QWaitCondition authCondition;

        QMap< QString, Attempt > credentialCacheByRealm;
        QMap< QString, Attempt > credentialCacheByHost;

        QStringList no_proxy;

        bool usingPAC();

    signals:
        void requestNewCredentials(QString realm, QString host);

    public slots:
        void doRequestNewCredentials(QString realm, QString host);

        friend class PACProxyFactory;
    };

}

#endif // UTOPIA_PACPROXYFACTORY_P_H
