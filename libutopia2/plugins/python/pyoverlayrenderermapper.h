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

#include <papyro/overlayrenderermapper.h>
#include <string>
#include <iostream>

#include <QDebug>

class PyOverlayRendererMapper : public Papyro::OverlayRendererMapper, public PyExtension
{
public:
    PyOverlayRendererMapper(std::string extensionClassName)
        : PyExtension("utopia.document.OverlayRendererMapper", extensionClassName), _ordering(0)
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

    QString mapToId(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
    {
        QString id;

        if (extensionObject()) {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            /* Get python wrapper of annotation */
            Annotation * ann = static_cast<Annotation *>(malloc(sizeof(Annotation)));
            ann->_ann = Spine::share_SpineAnnotation(annotation, 0);
            ann->_err = SpineError_NoError;
            PyObject * pyann = SWIG_NewPointerObj(static_cast<void *>(ann),
                                                  SWIG_TypeQuery("_p_Annotation"),
                                                  SWIG_POINTER_OWN);

            /* Get python wrapper of document */
            Document * doc = static_cast<Document *>(malloc(sizeof(Document)));
            doc->_doc = Spine::share_SpineDocument(document, 0);
            doc->_err = SpineError_NoError;
            PyObject * pydoc = SWIG_NewPointerObj(static_cast<void *>(doc),
                                                  SWIG_TypeQuery("_p_Document"),
                                                  SWIG_POINTER_OWN);

            if (pyann && pydoc) {
                /* Invoke method on extension */
                PyObject * ret = PyObject_CallMethod(extensionObject(), (char *) "mapToId", (char *) "(OO)", pydoc, pyann);

                if (ret == 0) /* Exception*/ {
                    std::cerr << "Error in OverlayRendererMapper " << extensionTypeName() << std::endl;
                    PyErr_PrintEx(0);
                } else {
                    if (PyString_Check(ret)) {
                        id = PyString_AsString(ret);
                    } else if (PyUnicode_Check(ret)) {
                        PyObject *tempstring=PyUnicode_AsUTF16String(ret);
                        const uint16_t *utf16=(const uint16_t *)PyString_AsString(tempstring);
                        size_t length=PyString_Size(tempstring);
                        id = QString::fromUtf16(utf16 + 1, length/2 - 1);
                        Py_DECREF(tempstring);
                    }

                    Py_DECREF(ret);
                }
            }

            Py_XDECREF(pyann);
            Py_XDECREF(pydoc);

            PyGILState_Release(gstate);
        }

        return id;
    }

    std::string title()
    {
        return extensionDocString();
    }

    int weight() const
    {
        return _ordering;
    }

protected:
    int _ordering;
};





