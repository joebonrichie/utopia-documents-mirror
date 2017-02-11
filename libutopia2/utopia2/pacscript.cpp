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

#include <utopia2/pacscript.h>

#include <QDate>
#include <QDateTime>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QRegExp>
#include <QScriptEngine>

#include <QtDebug>



namespace Utopia
{

    namespace
    {

        QScriptValue isPlainHostName(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1)
            {
                return context->throwError("isPlainHostName() takes exactly one argument");
            }

            QString host = context->argument(0).toString();

            return QScriptValue(engine, !host.isEmpty() && !host.contains('.'));
        }

        QScriptValue dnsDomainIs(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 2)
            {
                return context->throwError("dnsDomainIs() takes exactly two arguments");
            }

            QString host = context->argument(0).toString();
            QString domain = context->argument(1).toString();

            return QScriptValue(engine, host != domain && domain.startsWith(".") && host.endsWith(domain));
        }

        QScriptValue localHostOrDomainIs(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 2)
            {
                return context->throwError("localHostOrDomainIs() takes exacly two arguments");
            }

            QString host = context->argument(0).toString();
            QString hostDomain = context->argument(1).toString();

            if (host.contains('.'))
            {
                return QScriptValue(engine, host == hostDomain);
            }
            else
            {
                return QScriptValue(engine, hostDomain.startsWith(host));
            }
        }

        QScriptValue isResolvable(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1)
            {
                return context->throwError("isResolvable() takes exactly one argument");
            }

            QString host = context->argument(0).toString();
            QHostInfo info = QHostInfo::fromName(host);

            return QScriptValue(engine, !info.addresses().isEmpty());
        }

        QScriptValue isInNet(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 3)
            {
                return context->throwError("isInNet() takes exactly three arguments");
            }

            QString host = context->argument(0).toString();
            QHostInfo info = QHostInfo::fromName(host);
            QHostAddress netaddr(context->argument(1).toString());
            QHostAddress netmask(context->argument(2).toString());

            QList< QHostAddress > addresses(info.addresses());
            QListIterator< QHostAddress > iter(addresses);
            while (iter.hasNext())
            {
                QHostAddress addr(iter.next());
                if ((netaddr.toIPv4Address() & netmask.toIPv4Address()) == (addr.toIPv4Address() & netmask.toIPv4Address()))
                {
                    return QScriptValue(engine, true);
                }
            }

            return QScriptValue(engine, false);
        }

        QScriptValue dnsResolve(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1)
            {
                return context->throwError("dnsResolve() takes exactly one argument");
            }

            QString host = context->argument(0).toString();
            QHostInfo info = QHostInfo::fromName(host);
            QList< QHostAddress > addresses(info.addresses());

            if (addresses.isEmpty())
            {
                return engine->nullValue(); // TODO: Should this be undefined or an exception? check other implementations
            }

            return QScriptValue(engine, addresses.first().toString());
        }

        QScriptValue myIpAddress(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 0)
            {
                return context->throwError("myIpAddress() takes no arguments");
            }

            foreach(QHostAddress address, QNetworkInterface::allAddresses())
            {
                if (address != QHostAddress::LocalHost &&
                    address != QHostAddress::LocalHostIPv6)
                {
                    return QScriptValue(engine, address.toString());
                }
            }

            return engine->undefinedValue(); // TODO: Should this be null or an exception? check other implementations
        }

        QScriptValue dnsDomainLevels(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1)
            {
                return context->throwError("dnsDomainLevels() takes exactly one argument");
            }

            QString host = context->argument(0).toString();

            return QScriptValue(engine, host.count('.'));
        }

        QScriptValue shExpMatch(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 2)
            {
                return context->throwError("shExpMatch() takes exactly two arguments");
            }

            QString host = context->argument(0).toString();
            QRegExp re(context->argument(1).toString(), Qt::CaseSensitive, QRegExp::Wildcard);

            return QScriptValue(engine, re.exactMatch(host));
        }

        QScriptValue weekdayRange(QScriptContext * context, QScriptEngine * engine)
        {
            QString wd1;
            QString wd2;
            bool gmt(false);

            wd1 = context->argument(0).toString();

            switch (context->argumentCount())
            {
            case 1:
                break;
            case 2:
                wd2 = context->argument(1).toString();
                if (wd2 == "GMT")
                {
                    gmt = true;
                    wd2 = QString();
                }
                break;
            case 3:
                wd2 = context->argument(1).toString();
                gmt = true;
                break;
            default:
                return context->throwError("weekdayRange() takes between one and three arguments");
            }

            QDateTime now(QDateTime::currentDateTime());
            if (gmt) { now = now.toUTC(); }
            QDate today(now.date());
            int dayOfWeek = today.dayOfWeek();

            if (wd2.isEmpty())
            {
                return QScriptValue(engine, wd1.compare(QDate::shortDayName(dayOfWeek), Qt::CaseInsensitive) == 0);
            }
            else
            {
                bool in = false;
                for (int i = 0; i < 14; ++i)
                {
                    int d = (i % 7) + 1;
                    if (!in && wd1.compare(QDate::shortDayName(d), Qt::CaseInsensitive) == 0)
                    {
                        in = true;
                    }
                    if (in)
                    {
                        if (QDate::shortDayName(dayOfWeek).compare(QDate::shortDayName(d), Qt::CaseInsensitive) == 0)
                        {
                            return QScriptValue(engine, true);
                        }
                        if (wd2.compare(QDate::shortDayName(d), Qt::CaseInsensitive) == 0)
                        {
                            break;
                        }
                    }
                }
            }

            return QScriptValue(engine, false);
        }

        QScriptValue dateRange(QScriptContext * context, QScriptEngine * engine)
        {
            // FIXME implement this
            return QScriptValue(engine, true);
        }

        QScriptValue timeRange(QScriptContext * context, QScriptEngine * engine)
        {
            // FIXME implement this
            return QScriptValue(engine, true);
        }

        QScriptValue debug(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1)
            {
                return context->throwError("debug() takes exactly one argument");
            }

            qDebug() << context->argument(0).toString();

            return engine->undefinedValue();
        }

    }



    class PACScriptPrivate
    {
    public:
        PACScriptPrivate(PACScript * pac)
            : pac(pac), engine(0), valid(false)
            {}

        bool isValid() const
            {
                return engine;
            }

        void reset(const QString & script_ = QString())
            {
                if (engine)
                {
                    delete engine;
                    engine = 0;
                    valid = false;
                }

                script = script_;

                if (!script.isEmpty())
                {
                    engine = new QScriptEngine(pac);

                    QScriptValue globalObject = engine->globalObject();
                    globalObject.setProperty(QString("debug"), engine->newFunction(debug));
                    globalObject.setProperty(QString("isPlainHostName"), engine->newFunction(isPlainHostName));
                    globalObject.setProperty(QString("dnsDomainIs"), engine->newFunction(dnsDomainIs));
                    globalObject.setProperty(QString("localHostOrDomainIs"), engine->newFunction(localHostOrDomainIs));
                    globalObject.setProperty(QString("isResolvable"), engine->newFunction(isResolvable));
                    globalObject.setProperty(QString("isInNet"), engine->newFunction(isInNet));
                    globalObject.setProperty(QString("dnsResolve"), engine->newFunction(dnsResolve));
                    globalObject.setProperty(QString("myIpAddress"), engine->newFunction(myIpAddress));
                    globalObject.setProperty(QString("dnsDomainLevels"), engine->newFunction(dnsDomainLevels));
                    globalObject.setProperty(QString("shExpMatch"), engine->newFunction(shExpMatch));
                    globalObject.setProperty(QString("weekdayRange"), engine->newFunction(weekdayRange));
                    globalObject.setProperty(QString("dateRange"), engine->newFunction(dateRange));
                    globalObject.setProperty(QString("timeRange"), engine->newFunction(timeRange));

                    engine->evaluate(script);
                    valid = !engine->hasUncaughtException();
                }
            }

        PACScript * pac;
        QString script;
        QScriptEngine * engine;
        bool valid;
    };




    PACScript::PACScript()
        : d(new PACScriptPrivate(this))
    {}

    PACScript::~PACScript()
    {}

    bool PACScript::isValid() const
    {
        return d->valid;
    }

    void PACScript::setScript(const QString & script)
    {
        d->reset(script);
    }

    QString PACScript::script() const
    {
        return d->script;
    }

    QString PACScript::findProxyForUrl(const QString &url, const QString &host)
    {
        if (d->isValid())
        {
            QScriptValue globalObject = d->engine->globalObject();
            QScriptValue fun = globalObject.property("FindProxyForURL");
            if (fun.isFunction())
            {
                QScriptValueList args;
                args << d->engine->toScriptValue(url) << d->engine->toScriptValue(host);

                QScriptValue val = fun.call(globalObject, args);

                return d->engine->hasUncaughtException() ? QString() : val.toString();
            }
        }

        return QString("DIRECT");
    }

}
