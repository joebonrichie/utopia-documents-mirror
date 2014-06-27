/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <athenaeum/resolver.h>
#include <spine/Annotation.h>
#include <spine/Document.h>
#include <spine/spineapi.h>
#include <spine/spineapi_internal.h>
#include <string>
#include <iostream>

#include "conversion.h"

#include "spine/pyspineapi.h"

class PyResolver : public Athenaeum::Resolver, public PyExtension
{
public:
    PyResolver(std::string extensionClassName)
        : Athenaeum::Resolver(), PyExtension("utopia.library.Resolver", extensionClassName), _ordering(0)
    {
        // Acquire Python's global interpreter lock
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Ensure the extension object instantiated correctly, then tailor this object
        if (extensionObject()) {
            // Get Weight
            if (PyObject * weightret = PyObject_CallMethod(extensionObject(), (char *) "weight", (char *) "")) {
                _ordering = (int) PyInt_AS_LONG(weightret);
                Py_XDECREF(weightret);
            }
        }

        // Release Python's global interpreter lock
        PyGILState_Release(gstate);
    }

    QVariantMap resolve(const QVariantMap & metadata)
    {
        QVariantMap resolved;

        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        PyObject *method = PyString_FromString("resolve");

        PyObject * metadataObj = convert(metadata);

        /* Invoke method on extension */
        PyObject * ret = PyObject_CallMethodObjArgs(extensionObject(), method, metadataObj, NULL);

        if (ret == 0) { /* Exception*/
            PyObject * ptype = 0;
            PyObject * pvalue = 0;
            PyObject * ptraceback = 0;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            // Set this annotator's error message
            if (pvalue) {
                PyObject * msg = PyObject_Str(pvalue);
                setErrorString(PyString_AsString(msg));
                Py_DECREF(msg);
            } else if (ptype) {
                PyObject * msg = PyObject_Str(ptype);
                setErrorString(PyString_AsString(msg));
                Py_DECREF(msg);
            } else {
                setErrorString("An unknown error occurred");
            }
            PyErr_Restore(ptype, pvalue, ptraceback);
            PyErr_Print();
        } else {
            resolved = convert(ret).toMap();
        }

        /*  Clean up */
        Py_XDECREF(ret);
        Py_XDECREF(metadataObj);
        Py_DECREF(method);

        PyGILState_Release(gstate);

        return resolved;
    }

    std::string title()
    {
        return extensionDocString();
    }

    int weight()
    {
        return _ordering;
    }

private:
    int _ordering;

};
