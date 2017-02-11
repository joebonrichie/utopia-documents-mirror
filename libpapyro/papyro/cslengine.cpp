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

#include <papyro/cslengine.h>
#include <utopia2/global.h>

#include <boost/weak_ptr.hpp>

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <QResource>
#include <QScriptEngine>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <QDebug>

namespace Papyro
{

    namespace
    {

        QVariant elementToVariant(const QDomElement & element)
        {
            QVariantMap obj;
            obj["name"] = element.tagName();
            QVariantMap attrs;
            QDomNamedNodeMap attributes(element.attributes());
            for (int i = 0; i < attributes.count(); ++i) {
                QDomAttr attrNode(attributes.item(i).toAttr());
                attrs[attrNode.name()] = attrNode.value();
            }
            obj["attrs"] = attrs;
            QVariantList children;
            if (element.childNodes().size() == 0 && element.tagName() == "term") {
                children << QString("");
            }
            for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (n.isText()) {
                    children << n.toText().data();
                } else if (n.isElement()) {
                    children << elementToVariant(n.toElement());
                }
            }
            obj["children"] = children;
            return obj;
        }

        QString xmlToJson(const QString & xml)
        {
            QVariantMap map;
            QDomDocument doc;
            doc.setContent(xml.toUtf8());

            QDomElement elem = doc.documentElement();
            QVariant converted = elementToVariant(elem);

            return QString::fromUtf8(QJsonDocument::fromVariant(converted).toJson());
        }

        QScriptValue retrieveLocale(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1) {
                return context->throwError("retrieveLocale() takes exactly one argument");
            }

            QString lang = context->argument(0).toString();
            QString xml;

            QFile citeproc(Utopia::resource_path() + "/citeproc/locales/" + lang + ".json");
            if (citeproc.open(QIODevice::ReadOnly)) {
                QByteArray raw(citeproc.readAll());
                xml = QString::fromUtf8(raw.constData(), raw.size());
            }

            return engine->evaluate("(" + xml + ")", Utopia::resource_path() + "/citeproc/locales/" + lang + ".json");
        }

        QScriptValue debug(QScriptContext * context, QScriptEngine * engine)
        {
            if (context->argumentCount() != 1) {
                return context->throwError("debug() takes exactly one argument");
            }

            qDebug() << context->argument(0).toString();

            return engine->undefinedValue();
        }

        QString resource(const QString & path, bool warn = true)
        {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray raw(file.readAll());
                return QString::fromUtf8(raw.constData(), raw.size());
            } else if (warn) {
                qDebug() << "ERROR in ::resource(): Cannot load " + path;
            }
            return QString();
        }

        QScriptValue evaluate(QScriptEngine * engine, const QString & script, const QString & fileName = QString())
        {
            QScriptValue ret = engine->evaluate(script, fileName);
            if (engine->hasUncaughtException()) {
                qDebug() << "EXCEPTION";
                qDebug() << engine->uncaughtException().toString();
                foreach (QString line, engine->uncaughtExceptionBacktrace()) {
                    qDebug() << line;
                }
                engine->clearExceptions();
            }
            return ret;
        }

    }




    class CSLEnginePrivate
    {
    public:
        CSLEnginePrivate()
            : mutex(QMutex::Recursive)
        {
            // Populate from settings
            QSettings conf;
            conf.sync();
            conf.beginGroup("CSLEngine");
            defaultStyle = conf.value("Default Style", "apa").toString();

            // Provide the environment with helper methods
            QScriptValue globalObject = engine.globalObject();
            globalObject.setProperty(QString("retrieveLocale"), engine.newFunction(retrieveLocale));
            globalObject.setProperty(QString("debug"), engine.newFunction(debug));

            // Install javascript control code
            QStringList sources;
            sources << ":/citeproc/xmljson.js";
            sources << ":/citeproc/citeproc.js";
            sources << ":/cslengine.js";
            foreach (const QString & source, sources) {
                evaluate(&engine, resource(source), source);
            }

            // Grab handle for admin functions
            QScriptValue installLocaleFn = globalObject.property("installLocale");
            QScriptValue installStyleFn = globalObject.property("installStyle");

            // Install locales
            QVariantMap localeMap(evaluate(&engine, "(" + resource(Utopia::resource_path() + "/citeproc/locales.json") + ")", Utopia::resource_path() + "/citeproc/locales.json").toVariant().toMap());
            QMapIterator< QString, QVariant > localeMapIter(localeMap);
            while (localeMapIter.hasNext()) {
                localeMapIter.next();
                QString code = localeMapIter.key();
                QString description = localeMapIter.value().toString();
                if (code != "description") {
                    QString locale = resource(Utopia::resource_path() + "/citeproc/locales/" + code + ".json", false);
                    if (!locale.isEmpty()) {
                        QScriptValueList args;
                        args << engine.toScriptValue(code);
                        args << engine.toScriptValue(description);
                        args << engine.evaluate("(" + locale + ")");
                        installLocaleFn.call(globalObject, args);
                        qDebug() << "CSLEngine: Loaded locale" << code;
                    }
                }
            }

            // Install styles
            QDir stylesDir(Utopia::resource_path() + "/citeproc/styles");
            QDir userStylesDir(Utopia::profile_path() + "/csl");
            QStringList filters;
            filters << "*.json" << "*.csl";
            stylesDir.setNameFilters(filters);
            userStylesDir.setNameFilters(filters);
            QFileInfoList styleFiles = stylesDir.entryInfoList() + userStylesDir.entryInfoList();
            foreach (const QFileInfo & styleFileInfo, styleFiles) {
                QString styleFile = styleFileInfo.fileName();
                QString code = styleFile.section(".", 0, 0);
                QString description(code);
                description = description.replace(QRegExp("[^a-zA-Z0-9]+"), " ").trimmed();
                QString style = resource(styleFileInfo.filePath());
                if (styleFileInfo.suffix() == "csl") {
                    style = xmlToJson(style);
                }
                if (!style.isEmpty()) {
                    QScriptValueList args;
                    args << engine.toScriptValue(code);
                    args << engine.toScriptValue(description);
                    args << engine.evaluate("(" + style + ")");
                    installStyleFn.call(globalObject, args);
                    qDebug() << "CSLEngine: Loaded style" << styleFile;
                } else {
                    qDebug() << "CSLEngine: Could not load style" << styleFile;
                }
            }
        }

        ~CSLEnginePrivate()
        {}

        QString defaultStyle;
        QScriptEngine engine;
        QMutex mutex;

    }; // class CSLEnginePrivate




    CSLEngine::CSLEngine(QObject * parent)
        : QObject(parent), d(new CSLEnginePrivate)
    {}

    CSLEngine::~CSLEngine()
    {
        delete d;
    }

    QVariantMap CSLEngine::availableLocales() const
    {
        QMutexLocker guard(&d->mutex);
        QVariantMap locales;
        QScriptValue globalObject = d->engine.globalObject();
        QScriptValue getLocalesFn = globalObject.property("getLocales");
        if (getLocalesFn.isFunction()) {
            QVariantMap map(getLocalesFn.call(globalObject).toVariant().toMap());
            if (!d->engine.hasUncaughtException()) {
                QMapIterator< QString, QVariant > iter(map);
                while (iter.hasNext()) {
                    iter.next();
                    locales[iter.key()] = iter.value();
                }
            } else {
                qDebug() << "EXCEPTION";
                qDebug() << d->engine.uncaughtException().toString();
                foreach (QString line, d->engine.uncaughtExceptionBacktrace()) {
                    qDebug() << line;
                }
                d->engine.clearExceptions();
            }
        }
        return locales;
    }

    QVariantMap CSLEngine::availableStyles() const
    {
        QMutexLocker guard(&d->mutex);
        QVariantMap styles;
        QScriptValue globalObject = d->engine.globalObject();
        QScriptValue getStylesFn = globalObject.property("getStyles");
        if (getStylesFn.isFunction()) {
            QVariantMap map(getStylesFn.call(globalObject).toVariant().toMap());
            if (!d->engine.hasUncaughtException()) {
                QMapIterator< QString, QVariant > iter(map);
                while (iter.hasNext()) {
                    iter.next();
                    styles[iter.key()] = iter.value();
                }
            } else {
                qDebug() << "EXCEPTION";
                qDebug() << d->engine.uncaughtException().toString();
                foreach (QString line, d->engine.uncaughtExceptionBacktrace()) {
                    qDebug() << line;
                }
                d->engine.clearExceptions();
            }
        }
        return styles;
    }

    const QString & CSLEngine::defaultStyle() const
    {
        QMutexLocker guard(&d->mutex);
        return d->defaultStyle;
    }

    QString CSLEngine::format(const QVariantMap & metadata, const QString & style)
    {
        QMutexLocker guard(&d->mutex);
        QString formatted;
        QScriptValue globalObject = d->engine.globalObject();
        QScriptValue formatFn = globalObject.property("format");
        if (formatFn.isFunction()) {
            QScriptValueList args;
            args << d->engine.toScriptValue(metadata);
            args << d->engine.toScriptValue(style);
            args << d->engine.toScriptValue(defaultStyle());

            QScriptValue val = formatFn.call(globalObject, args);
            if (!d->engine.hasUncaughtException()) {
                formatted = val.toString();
            } else {
                qDebug() << "EXCEPTION ---" << val.toString();;
                qDebug() << d->engine.uncaughtException().toString();
                foreach (QString line, d->engine.uncaughtExceptionBacktrace()) {
                    qDebug() << line;
                }
                d->engine.clearExceptions();
            }
        } else {
            qDebug() << "ERROR: format doesn't seem to be a function object";
        }
        return formatted.trimmed();
    }

    boost::shared_ptr< CSLEngine > CSLEngine::instance()
    {
        static boost::weak_ptr< CSLEngine > singleton;
        boost::shared_ptr< CSLEngine > shared(singleton.lock());
        if (singleton.expired()) {
            shared = boost::shared_ptr< CSLEngine >(new CSLEngine());
            singleton = shared;
        }
        return shared;
    }

    void CSLEngine::setDefaultStyle(const QString & defaultStyle)
    {
        QMutexLocker guard(&d->mutex);
        d->defaultStyle = defaultStyle;
    }

} // namespace Papyro
