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

#include <papyro/remotequery.h>
#include <string>
#include <iostream>

#include <boost/python.hpp>
#include <boost/mpl/vector.hpp>

#include "conversion.h"

#include <QRunnable>
#include <QWeakPointer>

#include <QDebug>



namespace python = boost::python;
namespace mpl = boost::mpl;



class PyRemoteQuery : public Athenaeum::RemoteQuery, public PyExtension
{
public:
    PyRemoteQuery(std::string extensionClassName);
    ~PyRemoteQuery();

    bool fetch(const QVariantMap & query, int offset, int limit);
    QString description();
    void run();
    QString title();

    void del_property(python::object key);
    python::object get_property(python::object key, python::object def = python::object());
    void set_property(python::object key, python::object value);

protected:
    QVariantMap _query;
    int _offset;
    int _limit;
};




PyRemoteQuery::PyRemoteQuery(std::string extensionClassName)
    : Athenaeum::RemoteQuery(), PyExtension("utopia.library.RemoteQuery", extensionClassName)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    if (extensionObject()) {
        // Use boost::python to attach a method to the extension instance
        python::scope outer(python::object(python::handle<>(python::borrowed(extensionObject()))));
        python::def("get_property", python::make_function(bind(&PyRemoteQuery::get_property, this, _1, python::object()), python::default_call_policies(), mpl::vector< python::object, python::object >()));
        python::def("get_property", python::make_function(bind(&PyRemoteQuery::get_property, this, _1, _2), python::default_call_policies(), mpl::vector< python::object, python::object, python::object >()));
        python::def("set_property", python::make_function(bind(&PyRemoteQuery::set_property, this, _1, _2), python::default_call_policies(), mpl::vector< void, python::object, python::object >()));
        python::def("del_property", python::make_function(bind(&PyRemoteQuery::del_property, this, _1), python::default_call_policies(), mpl::vector< void, python::object >()));
    }

    PyGILState_Release(gstate);
}

PyRemoteQuery::~PyRemoteQuery()
{
    cancel();

    wait();
}

QString PyRemoteQuery::description()
{
    // FIXME get from somewhere else
    return QString::fromStdString(extensionDocString());
}

bool PyRemoteQuery::fetch(const QVariantMap & query, int offset, int limit)
{
    bool success = false;
    if (extensionObject())
    {
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Make sure fetch() is present and callable
        if (PyObject_HasAttrString(extensionObject(), "fetch") && PyCallable_Check(PyObject_GetAttrString(extensionObject(), "fetch"))) {
            _query = query;
            _offset = offset;
            _limit = limit;
            start();
            success = true;
        }

        PyGILState_Release(gstate);
    }
    return success;
}

void PyRemoteQuery::run()
{
    // Only bother to execute the query if the calling object still exists
    Athenaeum::RemoteQueryResultSet resultSet;

    bool success = false;
    if (extensionObject())
    {
        makeCancellable();

        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Make sure fetch() is present and callable
        if (PyObject_HasAttrString(extensionObject(), "fetch") && PyCallable_Check(PyObject_GetAttrString(extensionObject(), "fetch"))) {
            /* Invoke method on extension */
            PyObject * pyquery = convert(_query);
            if (pyquery) {
                PyObject * ret = PyObject_CallMethod(extensionObject(), (char *) "fetch", (char *) "(Oii)", pyquery, _offset, _limit);

                if (ret == 0) /* Exception*/ {
                    std::cerr << "Error in remote query " << extensionTypeName() << std::endl;
                    PyErr_PrintEx(0);
                } else {
                    PyObject * results;
                    if (ret == Py_None) {
                        success = true;
                    } else if (PyArg_ParseTuple(ret, "iiiO", &resultSet.offset, &resultSet.limit, &resultSet.count, &results)) {
                        resultSet.results = convert(results).toList();
                        success = true;
                    } else {
                        // FIXME error
                    }

                    Py_DECREF(ret);
                }
                Py_DECREF(pyquery);
            }
        }

        PyGILState_Release(gstate);

        // Only bother to notify of the query's completion if the calling object still exists
        if (success) {
            qRegisterMetaType< Athenaeum::RemoteQueryResultSet >();
            emit fetched(resultSet);
        } else {
            // FIXME
        }
    }
}

void PyRemoteQuery::del_property(python::object key)
{
    //qDebug() << "del_property" << _thread_id << this;
    // Del value
    setPersistentProperty(convert(key).toString(), QVariant());
}

python::object PyRemoteQuery::get_property(python::object key, python::object def)
{
    //qDebug() << "get_property" << _thread_id << this;
    // Get value
    python::object value(def);
    PyObject * valueObj = convert(persistentProperty(convert(key).toString()));
    if (valueObj != Py_None) {
        value = python::object(python::handle<>(valueObj));
    }
    return value;
}

void PyRemoteQuery::set_property(python::object key, python::object value)
{
    //qDebug() << "set_property" << _thread_id << this;
    // Set value
    setPersistentProperty(convert(key).toString(), convert(value));
}

QString PyRemoteQuery::title()
{
    return QString::fromStdString(extensionDocString());
}
