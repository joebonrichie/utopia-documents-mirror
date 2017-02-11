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

#include <papyro/citationfinder.h>
#include <papyro/capabilities.h>
#include <string>
#include <iostream>

#include <QRunnable>
#include <QThreadPool>

#include <QDebug>

class PyLinkFinder : public Papyro::Decorator, public Papyro::CitationFinder, public PyExtension
{
public:
    PyLinkFinder(std::string extensionClassName)
        : PyExtension("utopia.document.LinkFinder", extensionClassName), _capability(new Papyro::CitationFinderCapability(this))
    {}

    QList< boost::shared_ptr< Spine::Capability > > decorate(Spine::AnnotationHandle annotation)
    {
        QList< boost::shared_ptr< Spine::Capability > > capabilities;
        if (extensionObject())
        {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            // Make sure visualiable is present and callable
            if (PyObject_HasAttrString(extensionObject(), "findable") && PyCallable_Check(PyObject_GetAttrString(extensionObject(), "findable"))) {
                /* Get python wrapper of document */
                Annotation * ann = static_cast<Annotation *>(malloc(sizeof(Annotation)));
                ann->_ann = Spine::share_SpineAnnotation(annotation, 0);
                ann->_err = SpineError_NoError;
                PyObject * pyann = SWIG_NewPointerObj(static_cast<void *>(ann),
                                                      SWIG_TypeQuery("_p_Annotation"),
                                                      SWIG_POINTER_OWN);

                if (pyann) {
                    /* Invoke method on extension */
                    PyObject * ret = PyObject_CallMethod(extensionObject(), (char *) "findable", (char *) "(O)", pyann);

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

    QList< QPair< QString, QString > > find(Spine::AnnotationHandle annotation)
    {
        QString encoded;
        QList< QPair< QString, QString > > links;

        if (extensionObject())
        {
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
                PyObject * ret = PyObject_CallMethod(extensionObject(), (char *) "findLink", (char *) "(O)", pyann);

                if (ret == 0) /* Exception*/ {
                    std::cerr << "Error in linkFinder " << extensionTypeName() << std::endl;
                    PyErr_PrintEx(0);
                } else {
                    if (PySequence_Check(ret)) {
                        Py_ssize_t size = PySequence_Size(ret);
                        for (Py_ssize_t i = 0; i < size; ++i) {
                            PyObject * item = PySequence_GetItem(ret, i);
                            if (PyTuple_Check(item) && PyTuple_Size(item) == 2) {
                                PyObject * titleObj = PyTuple_GetItem(item, 0);
                                QString title;
                                PyObject * urlObj = PyTuple_GetItem(item, 1);
                                QString url;

                                if (PyString_Check(titleObj)) {
                                    title = PyString_AsString(titleObj);
                                } else if (PyUnicode_Check(titleObj)) {
                                    PyObject * tempstring = PyUnicode_AsUTF16String(titleObj);
                                    const uint16_t * utf16 = (const uint16_t *) PyString_AsString(tempstring);
                                    size_t length = PyString_Size(tempstring);
                                    title = QString::fromUtf16(utf16 + 1, length/2 - 1);
                                    Py_DECREF(tempstring);
                                }

                                if (PyString_Check(urlObj)) {
                                    url = PyString_AsString(urlObj);
                                } else if (PyUnicode_Check(urlObj)) {
                                    PyObject * tempstring = PyUnicode_AsUTF16String(urlObj);
                                    const uint16_t * utf16 = (const uint16_t *) PyString_AsString(tempstring);
                                    size_t length = PyString_Size(tempstring);
                                    url = QString::fromUtf16(utf16 + 1, length/2 - 1);
                                    Py_DECREF(tempstring);
                                }

                                if (!title.isEmpty() && !url.isEmpty()) {
                                    links.append(qMakePair(title, url));
                                }
                            }
                        }
                    } else {
                        // FIXME should have been a sequence
                    }

                    Py_DECREF(ret);
                }

                Py_DECREF(pyann);
            }

            PyGILState_Release(gstate);
        }

        return links;
    }

    QString title()
    {
        return QString::fromStdString(extensionDocString());
    }

protected:
    Spine::CapabilityHandle _capability;
};





