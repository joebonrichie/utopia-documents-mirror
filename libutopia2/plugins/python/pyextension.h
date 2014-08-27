/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <string>
#include <iostream>

class PyExtension
{
public:
    PyExtension(const std::string & extensionMetaType, const std::string & extensionTypeName)
        : _extensionMetaType(extensionMetaType), _extensionTypeName(extensionTypeName), _extensionObject(0), _extensionNamespace(0)
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

protected:
    std::string extensionMetaType() const { return _extensionMetaType; }
    std::string extensionTypeName() const { return _extensionTypeName; }
    std::string extensionDocString() const { return _extensionDocString; }
    PyObject * extensionObject() const { return _extensionObject; }
    PyObject * extensionNamespace() const { return _extensionNamespace; }

private:
    std::string _extensionMetaType;
    std::string _extensionTypeName;
    std::string _extensionDocString;
    PyObject * _extensionObject;
    PyObject * _extensionNamespace;
};
