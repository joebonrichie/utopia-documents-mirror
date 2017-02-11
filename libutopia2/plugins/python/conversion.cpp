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

#include "conversion.h"
#include <datetime.h>

#include <QDateTime>

PyObject * convert(const QVariant & variant)
{
    PyObject * result = 0;
    if (!variant.isNull()) {
        switch (variant.type()) {
        case QVariant::DateTime: {
            QDateTime dateTime(variant.toDateTime());
            result = PyDateTime_FromDateAndTime(dateTime.date().year(),
                                                dateTime.date().month(),
                                                dateTime.date().day(),
                                                dateTime.time().hour(),
                                                dateTime.time().minute(),
                                                dateTime.time().second(),
                                                dateTime.time().msec());
            break;
        }
        case QVariant::Time: {
            QTime time(variant.toTime());
            result = PyTime_FromTime(time.hour(),
                                     time.minute(),
                                     time.second(),
                                     time.msec());
            break;
        }
        case QVariant::Date: {
            QDate date(variant.toDate());
            result = PyDate_FromDate(date.year(),
                                     date.month(),
                                     date.day());
            break;
        }
        case QVariant::Bool:
            result = variant.toBool() ? Py_True : Py_False;
            Py_INCREF(result);
            break;
        case QVariant::Char: {
            QByteArray character(QString(variant.toChar()).toUtf8());
            result = PyUnicode_DecodeUTF8(character.constData(), character.size(), 0);
            break;
        }
        case QVariant::Double:
            result = PyFloat_FromDouble(variant.toDouble());
            break;
        case QVariant::Int:
            result = PyInt_FromLong(variant.toInt());
            break;
        case QVariant::LongLong:
            result = PyLong_FromLongLong(variant.toLongLong());
            break;
        case QVariant::StringList:
        case QVariant::List: {
            result = PyList_New(variant.toList().size());
            Py_ssize_t i = 0;
            foreach (const QVariant & item, variant.toList()) {
                PyList_SetItem(result, i++, convert(item));
            }
            break;
        }
        case QVariant::Map: {
            result = PyDict_New();
            QVariantMap map(variant.toMap());
            QMutableMapIterator< QString, QVariant > iter(map);
            while (iter.hasNext()) {
                iter.next();
                PyObject * key = convert(iter.key());
                PyObject * value = convert(iter.value());
                PyDict_SetItem(result, key, value);
                Py_DECREF(key);
                Py_DECREF(value);
            }
            break;
        }
        case QVariant::String: {
            QByteArray utf8(variant.toString().toUtf8());
            result = PyUnicode_DecodeUTF8(utf8.constData(), utf8.size(), 0);
            break;
        }
        default:
            break;
        }
    }
    if (result == 0) {
        result = Py_None;
        Py_INCREF(result);
    }
    return result;
}

QVariant convert(PyObject * object)
{
    // Ensure the datetime module is imported (used in conversion.cpp)
    PyDateTime_IMPORT;

    QVariant result;
    if (object && object != Py_None) {
        if (PyBool_Check(object)) {
            result = (object == Py_True);
        } else if (PyInt_Check(object)) {
            result = (qlonglong) PyInt_AS_LONG(object);
        } else if (PyLong_Check(object)) {
            result = PyLong_AsLongLong(object);
        } else if (PyFloat_Check(object)) {
            result = PyFloat_AS_DOUBLE(object);
        } else if (PyString_Check(object)) {
            result = QString(PyString_AS_STRING(object));
        } else if (PyUnicode_Check(object)) {
            PyObject * tempstring = PyUnicode_AsUTF8String(object);
            const char * utf8 = PyString_AsString(tempstring);
            Py_ssize_t length = PyString_Size(tempstring);
            result = QString::fromUtf8(utf8, length);
            Py_XDECREF(tempstring);
        } else if (PyTuple_Check(object) || PyList_Check(object)) {
            QVariantList list;
            Py_ssize_t length = PySequence_Size(object);
            for (Py_ssize_t i = 0; i < length; ++i) {
                PyObject * item = PySequence_GetItem(object, i);
                list.append(convert(item));
                Py_XDECREF(item);
            }
            result = list;
        } else if (PyDict_Check(object)) {
            QVariantMap map;
            PyObject * key = 0;
            PyObject * value = 0;
            Py_ssize_t pos = 0;
            while (PyDict_Next(object, &pos, &key, &value)) {
                map[convert(key).toString()] = convert(value);
            }
            result = map;
        } else if (PyDateTime_Check(object)) {
            result = QDateTime(QDate(PyDateTime_GET_YEAR(object),
                                     PyDateTime_GET_MONTH(object),
                                     PyDateTime_GET_DAY(object)),
                               QTime(PyDateTime_DATE_GET_HOUR(object),
                                     PyDateTime_DATE_GET_MINUTE(object),
                                     PyDateTime_DATE_GET_SECOND(object),
                                     PyDateTime_DATE_GET_MICROSECOND(object)));
        } else if (PyTime_Check(object)) {
            result = QTime(PyDateTime_TIME_GET_HOUR(object),
                           PyDateTime_TIME_GET_MINUTE(object),
                           PyDateTime_TIME_GET_SECOND(object),
                           PyDateTime_TIME_GET_MICROSECOND(object));
        } else if (PyDate_Check(object)) {
            result = QDate(PyDateTime_GET_YEAR(object),
                           PyDateTime_GET_MONTH(object),
                           PyDateTime_GET_DAY(object));
        }
    }
    return result;
}

QVariant convert(const boost::python::object & object)
{
    return convert(object.ptr());
}

