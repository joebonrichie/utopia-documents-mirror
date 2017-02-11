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

#include <utopia2/configurable.h>
#include <utopia2/configuration.h>

#include <boost/python.hpp>
#include <boost/mpl/vector.hpp>

#include <QUuid>
#include <QDebug>

#include <string>
#include <vector>
#include <iostream>

namespace python = boost::python;
namespace mpl = boost::mpl;




class PyExtension : public virtual Utopia::Configurable
{
public:
    PyExtension(const std::string & extensionMetaType, const std::string & extensionTypeName)
        : _extensionMetaType(extensionMetaType),
          _extensionTypeName(extensionTypeName),
          _extensionObject(0),
          _extensionNamespace(0),
          _thread_id(0)
    {
        // Acquire Python's global interpreter lock
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Load the specified meta type's class and instantiate an object
        _extensionNamespace = PyModule_GetDict(PyImport_AddModule(extensionTypeName.substr(0, extensionTypeName.rfind('.')).c_str()));
        _extensionObject = PyRun_String((extensionMetaType + ".typeOf('" + extensionTypeName + "')()").c_str(), Py_eval_input, _extensionNamespace, _extensionNamespace);
        if (_extensionObject == 0) {
            PyErr_PrintEx(0);
        } else {
            // Get class' doc string
            PyObject * doc = PyObject_GetAttrString(_extensionObject, "__doc__");
            _extensionDocString = doc != Py_None ? PyString_AsString(doc) : "UNTITLED";
            Py_XDECREF(doc);

            // Ensure the extension object instantiated correctly, then tailor this object
            if (extensionObject()) {
                // Get UUID
                if (PyObject * uuidret = PyObject_CallMethod(extensionObject(), (char *) "uuid", NULL)) {
                    _uuid = PyString_AsString(uuidret);
                    Py_DECREF(uuidret);

                    // Use boost::python to attach a method to the extension instance
                    python::scope outer(python::object(python::handle<>(python::borrowed(extensionObject()))));
                    python::def("get_config", python::make_function(bind(&PyExtension::get_config, this, _1, python::object()), python::default_call_policies(), mpl::vector< python::object, python::object >()));
                    python::def("get_config", python::make_function(bind(&PyExtension::get_config, this, _1, _2), python::default_call_policies(), mpl::vector< python::object, python::object, python::object >()));
                    python::def("set_config", python::make_function(bind(&PyExtension::set_config, this, _1, _2), python::default_call_policies(), mpl::vector< void, python::object, python::object >()));
                    python::def("del_config", python::make_function(bind(&PyExtension::del_config, this, _1), python::default_call_policies(), mpl::vector< void, python::object >()));
                }
            }
        }

        // Release Python's global interpreter lock
        PyGILState_Release(gstate);
    }

    ~PyExtension()
    {
        if (_extensionObject) {
            // Acquire Python's global interpreter lock
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            // Release object for GC
            Py_DECREF(_extensionObject);

            // Release Python's global interpreter lock
            PyGILState_Release(gstate);
        }
    }

    void cancel()
    {
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        if (_thread_id > 0) {
            PyObject * exc = PyErr_NewException((char *) "utopia.Cancellation", 0, 0);
            PyThreadState_SetAsyncExc(_thread_id, exc);
            _thread_id = 0;
        }

        PyGILState_Release(gstate);
    }

    void makeCancellable()
    {
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Get current thread ID
        PyObject * threadName = PyString_FromString("thread");
        PyObject * thread = PyImport_Import(threadName);
        Py_DECREF(threadName);
        PyObject * get_ident = PyObject_GetAttrString(thread, "get_ident");
        PyObject * ident = PyObject_CallObject(get_ident, 0);
        Py_DECREF(get_ident);
        _thread_id = PyInt_AsLong(ident);
        Py_DECREF(ident);

        PyGILState_Release(gstate);
    }




    //**** Configuration methods ****//

    QUuid configurationId() const
    {
        return uuid().c_str();
    }

    void del_config(python::object key)
    {
        // Del value
        configuration()->del(convert(key).toString());
    }

    python::object get_config(python::object key, python::object def = python::object())
    {
        // Resolve key
        python::object value(def);
        PyObject * valueObj = convert(configuration()->get(convert(key).toString()));
        if (valueObj != Py_None) {
            value = python::object(python::handle<>(valueObj));
        }
        return value;
    }

    void set_config(python::object key, python::object value)
    {
        // Set value
        configuration()->set(convert(key).toString(), convert(value));
    }

protected:
    std::string extensionMetaType() const { return _extensionMetaType; }
    std::string extensionTypeName() const { return _extensionTypeName; }
    std::string extensionDocString() const { return _extensionDocString; }
    PyObject * extensionObject() const { return _extensionObject; }
    PyObject * extensionNamespace() const { return _extensionNamespace; }
    std::string uuid() const { return _uuid; }

private:
    std::string _extensionMetaType;
    std::string _extensionTypeName;
    std::string _extensionDocString;
    PyObject * _extensionObject;
    PyObject * _extensionNamespace;
    std::string _uuid;
    long _thread_id;
};
