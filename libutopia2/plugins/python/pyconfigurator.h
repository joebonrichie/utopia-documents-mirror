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

#include <Python.h>

#include <utopia2/qt/configurator.h>

#include "conversion.h"

#include <QRegExp>
#include <QUrl>

#include <QDebug>

class PyConfigurator : public Utopia::Configurator, public PyExtension
{
public:
    PyConfigurator(std::string extensionClassName)
        : PyExtension("utopia.Configurator", extensionClassName)
    {
        // Acquire Python's global interpreter lock
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Ensure the extension object instantiated correctly, then tailor this object
        if (extensionObject()) {
            // Get configuration title
            if (PyObject * titleret = PyObject_CallMethod(extensionObject(), (char *) "title", (char *) "")) {
                _title = convert(titleret).toString();
                Py_XDECREF(titleret);
            }

            // Get Icon URL
            if (PyObject * iconret = PyObject_CallMethod(extensionObject(), (char *) "icon", (char *) "")) {
                QUrl iconUrl = QString(PyString_AsString(iconret));
                Py_XDECREF(iconret);
                QRegExp r("data:([^;,]+)?(?:;charset=([^;,]+))?(?:;(base64))?,(.*)");
                if (r.exactMatch(iconUrl.toString())) {
                    QString mime(r.cap(1));
                    QString charset(r.cap(2));
                    QString encoding(r.cap(3));
                    QString data(r.cap(4));
                    if (encoding == "base64") {
                        _icon = QImage::fromData(QByteArray::fromBase64(data.toUtf8()));
                    }
                }
            }
        }

        // Release Python's global interpreter lock
        PyGILState_Release(gstate);

        // Assign default values
        QVariantMap defaultConfig(defaults());
        Utopia::Configuration * conf(configuration());
        QMapIterator< QString, QVariant > iter(defaultConfig);
        while (iter.hasNext()) {
            iter.next();
            if (!conf->contains(iter.key())) {
                conf->set(iter.key(), iter.value());
            }
        }
    }

    QVariantMap defaults() const
    {
        QVariantMap defaultConfig;

        if (extensionObject())
        {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            if (PyObject_HasAttrString(extensionObject(), "defaults")) {
                PyObject * output = PyObject_CallMethod(extensionObject(), (char *) "defaults", (char *) "()");
                if (output)
                {
                    defaultConfig = convert(output).toMap();
                    Py_DECREF(output);
                }
                else
                {
                    PyErr_PrintEx(0);
                }
            }

            PyGILState_Release(gstate);
        }

        return defaultConfig;
    }

    QString form() const
    {
        QString html;

        if (extensionObject())
        {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            PyObject * output = PyObject_CallMethod(extensionObject(), (char *) "form", (char *) "()");
            if (output)
            {
                html = convert(output).toString();
                Py_DECREF(output);
            }
            else
            {
                PyErr_PrintEx(0);
            }

            PyGILState_Release(gstate);
        }

        return html;
    }

    QImage icon() const
    {
        return _icon;
    }

    QString title() const
    {
        return _title;
    }

    QUuid configurationId() const
    {
        return uuid().c_str();
    }

protected:
    QString _title;
    QImage _icon;
};
