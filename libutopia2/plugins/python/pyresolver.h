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

#include <papyro/resolver.h>
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
        : Athenaeum::Resolver(), PyExtension("utopia.citation.Resolver", extensionClassName), _ordering(0)
    {
        // Acquire Python's global interpreter lock
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Ensure the extension object instantiated correctly, then tailor this object
        if (extensionObject()) {
            // Get Weight
            if (PyObject * weightret = PyObject_CallMethod(extensionObject(), (char *) "weight", (char *) "")) {
                _ordering = convert(weightret).toInt();
                Py_XDECREF(weightret);
            }
            // Get Purpose
            if (PyObject_HasAttrString(extensionObject(), (char *) "purposes")) {
                if (PyObject * purposeret = PyObject_CallMethod(extensionObject(), (char *) "purposes", (char *) "")) {
                    QStringList purposes = convert(purposeret).toStringList();
                    if (purposes.isEmpty()) {
                        purposes << convert(purposeret).toString();
                    }
                    if (purposes.contains("expand")) { _purposes |= Athenaeum::Resolver::Expand; }
                    if (purposes.contains("identify")) { _purposes |= Athenaeum::Resolver::Identify; }
                    if (purposes.contains("dereference")) { _purposes |= Athenaeum::Resolver::Dereference; }
                    if (!_purposes) { // Not sure this ought to be the default
                        _purposes |= Athenaeum::Resolver::Dereference;
                    }
                    Py_XDECREF(purposeret);
                }
            }
        }

        // Release Python's global interpreter lock
        PyGILState_Release(gstate);
    }

    // Ensure the extension is cancelled
    void cancel()
    {
        PyExtension::cancel();
    }

    Athenaeum::Resolver::Purposes purposes()
    {
        return _purposes;
    }

    QVariantList resolve(const QVariantList & citations, Spine::DocumentHandle document = Spine::DocumentHandle())
    {
        QVariantList resolved(citations);

        makeCancellable();

        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        PyObject * method = PyString_FromString("execute_resolver");

        /* Get python wrapper of document */
        PyObject * pydoc = 0;
        if (document) {
            Document * doc = static_cast<Document *>(malloc(sizeof(Document)));
            doc->_doc = Spine::share_SpineDocument(document, 0);
            doc->_err = SpineError_NoError;
            pydoc = SWIG_NewPointerObj(static_cast<void *>(doc),
                                       SWIG_TypeQuery("_p_Document"),
                                       SWIG_POINTER_OWN);
        } else {
            pydoc = Py_None;
            Py_INCREF(pydoc);
        }

        PyObject * citationsObj = convert(resolved);
        /* Invoke method on extension */
        PyObject * ret = PyObject_CallMethodObjArgs(extensionObject(), method, citationsObj, pydoc, NULL);

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
            PyErr_PrintEx(0);
        } else {
            resolved = convert(citationsObj).toList();
            resolved.append(convert(ret).toList());
        }

        /*  Clean up */
        Py_XDECREF(ret);
        Py_XDECREF(citationsObj);
        Py_XDECREF(pydoc);
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
    Athenaeum::Resolver::Purposes _purposes;

};
