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

#include <papyro/visualiser.h>
#include <papyro/capabilities.h>
#include <string>
#include <iostream>

#include <QRunnable>
#include <QThreadPool>

#include <QDebug>

class PyVisualiser : public Papyro::Decorator, public Papyro::Visualiser, public PyExtension
{
public:
    PyVisualiser(std::string extensionClassName)
        : PyExtension("utopia.document.Visualiser", extensionClassName), _capability(new Papyro::VisualiserCapability(this))
    {}

    QList< boost::shared_ptr< Spine::Capability > > decorate(Spine::AnnotationHandle annotation)
    {
        QList< boost::shared_ptr< Spine::Capability > > capabilities;
        if (extensionObject()) {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            // Make sure visualiable is present and callable
            if (PyObject_HasAttrString(extensionObject(), "visualisable") && PyCallable_Check(PyObject_GetAttrString(extensionObject(), "visualisable"))) {
                /* Get python wrapper of document */
                Annotation * ann = static_cast<Annotation *>(malloc(sizeof(Annotation)));
                ann->_ann = Spine::share_SpineAnnotation(annotation, 0);
                ann->_err = SpineError_NoError;
                PyObject * pyann = SWIG_NewPointerObj(static_cast<void *>(ann),
                                                      SWIG_TypeQuery("_p_Annotation"),
                                                      SWIG_POINTER_OWN);

                if (pyann) {
                    /* Invoke method on extension */
                    PyObject * ret = PyObject_CallMethod(extensionObject(), (char *) "visualisable", (char *) "(O)", pyann);

                    if (ret == 0) /* Exception*/ {
                        std::cerr << "Error in decorator " << extensionTypeName() << std::endl;
                        PyErr_PrintEx(0);
                    } else {
                        if (PyObject_IsTrue(ret)) {
                            capabilities.append(_capability);
                        }

                        Py_DECREF(ret);
                    }

                    Py_DECREF(pyann);
                }
            }

            PyGILState_Release(gstate);
        }
        return capabilities;
    }

    QStringList visualise(Spine::AnnotationHandle annotation)
    {
        QStringList divs;

        if (extensionObject()) {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            /* Get python wrapper of document */
            Annotation * ann = static_cast<Annotation *>(malloc(sizeof(Annotation)));
            ann->_ann = Spine::share_SpineAnnotation(annotation, 0);
            ann->_err = SpineError_NoError;
            PyObject * pyann = SWIG_NewPointerObj(static_cast<void *>(ann),
                                                  SWIG_TypeQuery("_p_Annotation"),
                                                  SWIG_POINTER_OWN);

            if (pyann) {
                /* Invoke method on extension */
                PyObject * ret = PyObject_CallMethod(extensionObject(), (char *) "visualise", (char *) "(O)", pyann);

                if (ret == 0) /* Exception*/ {
                    std::cerr << "Error in visualiser " << extensionTypeName() << std::endl;
                    PyErr_PrintEx(0);
                } else {
                    if (PyString_Check(ret) || PyUnicode_Check(ret)) {
                        PyObject * wrapped = PyTuple_New(1);
                        PyTuple_SetItem(wrapped, 0, ret);
                        ret = wrapped;
                    }
                    if (PySequence_Check(ret)) {
                        for (Py_ssize_t i = 0; i < PySequence_Size(ret); ++i) {
                            PyObject * item = PySequence_GetItem(ret, i);
                            if (PyString_Check(item)) {
                                divs << PyString_AsString(item);
                            } else if (PyUnicode_Check(item)) {
                                PyObject *tempstring=PyUnicode_AsUTF16String(item);
                                const uint16_t *utf16=(const uint16_t *)PyString_AsString(tempstring);
                                size_t length=PyString_Size(tempstring);
                                divs << QString::fromUtf16(utf16 + 1, length/2 - 1);
                                Py_DECREF(tempstring);
                            }
                            Py_DECREF(item);
                        }
                    }

                    Py_DECREF(ret);
                }

                Py_DECREF(pyann);
            }

            PyGILState_Release(gstate);
        }

        return divs;
    }

    std::string title()
    {
        return extensionDocString();
    }

protected:
    Spine::CapabilityHandle _capability;
};





