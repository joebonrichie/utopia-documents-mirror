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

#include <papyro/annotator.h>
#include <papyro/utils.h>
#include <spine/spineapi.h>
#include <spine/spineapi_internal.h>
#include <utopia2/busagent.h>

#include <boost/python.hpp>
#include <boost/mpl/vector.hpp>

#include "conversion.h"
#include "spine/pyspineapi.h"

#include <string>
#include <iostream>

#include <QDebug>
#include <QVariant>

namespace python = boost::python;
namespace mpl = boost::mpl;




static QMap< QString, QString > event_name_to_legacy_method_name;

QString event_name_to_method_name(const QString & event)
{
    QRegExp parse("(?:(\\w+):)?(\\w+)");
    QString name;
    if (parse.exactMatch(event)) {
        QString timing(parse.cap(1));
        QString type(parse.cap(2));
        if (timing.isEmpty()) { timing = "on"; }
        name = QString("%1_%2_event").arg(timing).arg(type);
    }
    return name;
}


class PyAnnotator : public Papyro::Annotator, public PyExtension
{
public:
    PyAnnotator(std::string extensionClassName)
        : PyExtension("utopia.document.Annotator", extensionClassName)
    {
        if (event_name_to_legacy_method_name.isEmpty()) {
            event_name_to_legacy_method_name["on:load"] = "prepare";
            event_name_to_legacy_method_name["after:load"] = "reducePrepare";
            event_name_to_legacy_method_name["on:ready"] = "populate";
            event_name_to_legacy_method_name["after:ready"] = "reducePopulate";
            event_name_to_legacy_method_name["on:filter"] = "filter";
            event_name_to_legacy_method_name["after:filter"] = "reduceFilter";
            event_name_to_legacy_method_name["on:activate"] = "annotate";
            event_name_to_legacy_method_name["after:activate"] = "reduceAnnotate";
            event_name_to_legacy_method_name["on:marshal"] = "marshal";
            event_name_to_legacy_method_name["after:marshal"] = "reduceMarshal";
            event_name_to_legacy_method_name["on:persist"] = "persist";
            event_name_to_legacy_method_name["on:explore"] = "lookup";
        }




        // Acquire Python's global interpreter lock
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        // Ensure the extension object instantiated correctly, then tailor this object
        if (extensionObject()) {
            // Get BusId
            if (PyObject * busidret = PyObject_CallMethod(extensionObject(), (char *) "busId", NULL)) {
                _busId = PyString_AsString(busidret);
                Py_DECREF(busidret);

                // Use boost::python to attach a method to the extension instance
                python::scope outer(python::object(python::handle<>(python::borrowed(extensionObject()))));
                python::def("postToBus", python::make_function(bind(&PyAnnotator::postToBusFromPython, this, _1, python::object()), python::default_call_policies(), mpl::vector< void, python::object >()));
                python::def("postToBus", python::make_function(bind(&PyAnnotator::postToBusFromPython, this, _1, _2), python::default_call_policies(), mpl::vector< void, python::object, python::object >()));
            }

            // Get handleable events timing:name/weight
            if (PyObject * eventsret = PyObject_CallMethod(extensionObject(), (char *) "handleableEvents", NULL)) {
                _handleableEvents = convert(eventsret).toStringList();
                foreach (const QString & event, _handleableEvents) {
                    _handleableEventNames << event.mid(0, event.indexOf('/'));
                }
                Py_DECREF(eventsret);
            } else {
                PyErr_Clear();
            }

            // Work out handleable events timing:name
            if (PyObject * dir = PyObject_Dir(extensionObject())) {
                foreach (const QString & attr, convert(dir).toStringList()) {
                    std::string std_attr = Papyro::unicodeFromQString(attr);
                    const char * c_attr = std_attr.c_str();
                    if (PyObject_HasAttrString(extensionObject(), (char *) c_attr)) {
                        if (PyObject * py_attr = PyObject_GetAttrString(extensionObject(), (char *) c_attr)) {
                            QRegExp parse("(before|on|after)_(\\w+)_event");
                            if (PyCallable_Check(py_attr) && parse.exactMatch(attr)) {
                                int weight = 0;
                                if (PyObject * doc = PyObject_GetAttrString(py_attr, (char *) "__doc__")) {
                                    QRegExp parseWeight(".*\\[(?:.+;)?\\s*weight=(-?\\d+)\\s*(?:;.+)?\\].*");
                                    if (parseWeight.exactMatch(convert(doc).toString())) {
                                        weight = parseWeight.cap(1).toInt();
                                    }
                                    Py_DECREF(doc);
                                }

                                QString event(QString("%1:%2").arg(parse.cap(1)).arg(parse.cap(2)));
                                _handleableEventNames << event;
                                event += QString("/%1").arg(weight);
                                _handleableEvents << event;
                            }
                            Py_DECREF(py_attr);
                        }
                    }
                }
                Py_DECREF(dir);
            } else {
                PyErr_PrintEx(0);
            }

            // Register legacy method names to event names
            QMapIterator< QString, QString > liter(event_name_to_legacy_method_name);
            while (liter.hasNext()) {
                liter.next();
                std::string legacy_method_name(Papyro::unicodeFromQString(liter.value()));
                if (PyObject_HasAttrString(extensionObject(), legacy_method_name.c_str()) &&
                    PyCallable_Check(PyObject_GetAttrString(extensionObject(), legacy_method_name.c_str()))) {
                    _handleableLegacyEvents << liter.key();
                }
            }
        }

        // Release Python's global interpreter lock
        PyGILState_Release(gstate);
    }

    bool _annotate(std::string name, Spine::DocumentHandle document, const QVariantMap & kwargs = QVariantMap())
    {
        bool success = true;

        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        PyObject * method = PyString_FromString(name.c_str());

        /* Get python wrapper of document */
        PyObject * pydoc = 0;
        if (document) {
            Document * doc = static_cast<Document *>(malloc(sizeof(Document)));
            doc->_doc = Spine::share_SpineDocument(document, 0);
            doc->_err = SpineError_NoError;
            pydoc = SWIG_NewPointerObj(static_cast<void *>(doc),
                                       SWIG_TypeQuery("_p_Document"),
                                       SWIG_POINTER_OWN);
        }

        if (extensionObject()) {
            PyObject * ret = 0;

            // Build the *args and *kwargs objects for this invocation
            PyObject * pyargs = PyTuple_New(0);
            PyObject * pykwargs = convert(kwargs);

            if (pydoc) {
                // This will overwrite whatever kwarg has been passed in
                PyDict_SetItemString(pykwargs, (char *) "document", pydoc);
            }

            /* Invoke method on extension */
            if (PyObject * callable = PyObject_GetAttrString(extensionObject(), name.c_str())) {
                ret = PyObject_Call(callable, pyargs, pykwargs);
                Py_DECREF(callable);
            }

            Py_DECREF(pyargs);
            Py_DECREF(pykwargs);

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

                success = false;
            } else {
                // Don't care about the return value
                Py_DECREF(ret);
            }
        }

        /*  Clean up */
        Py_XDECREF(pydoc);
        Py_DECREF(method);

        PyGILState_Release(gstate);

        return success;
    }

    // Ensure the extension is cancelled
    void cancel()
    {
        PyExtension::cancel();
    }

    bool canHandleEvent(const QString & event)
    {
        foreach (const QString & candidate, handleableEvents()) {
            if (candidate == event ||
                candidate.startsWith(event + "/")) {
                return true;
            }
        }
        return false;
    }

    QStringList handleableEvents()
    {
        QStringList unique(_handleableEvents + _handleableLegacyEvents);
        unique.removeDuplicates();
        return unique;
    }

    bool handleEvent(const QString & event, Spine::DocumentHandle document, const QVariantMap & kwargs)
    {
        makeCancellable();

        // Only attempt events we've registered
        if (_handleableEventNames.contains(event)) {
            QString name(event_name_to_method_name(event));
            return _annotate(Papyro::unicodeFromQString(name), document, kwargs);
        }
        if (_handleableLegacyEvents.contains(event)) {
            // Map event name to legacy method name
            QString legacy(event_name_to_legacy_method_name.value(event));
            return _annotate(Papyro::unicodeFromQString(legacy), document, kwargs);
        }
        return false;
    }

    std::set< Spine::AnnotationHandle > lookup(Spine::DocumentHandle document, const std::string & phrase, const QVariantMap & kwargs = QVariantMap())
    {
        std::set< Spine::AnnotationHandle > derived;

        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        /* Get python wrapper of document */
        PyObject * pydoc = 0;
        if (document) {
            Document * doc = static_cast<Document *>(malloc(sizeof(Document)));
            doc->_doc = Spine::share_SpineDocument(document, 0);
            doc->_err = SpineError_NoError;
            pydoc = SWIG_NewPointerObj(static_cast<void *>(doc),
                                       SWIG_TypeQuery("_p_Document"),
                                       SWIG_POINTER_OWN);
        }

        /* Get python wrapper of annotation */
        PyObject * input = PyUnicode_DecodeUTF8(phrase.data(), phrase.length(), 0);

        if (input) {
            PyObject * pyargs = PyTuple_New(0);
            PyObject * pykwargs = convert(kwargs);
            PyDict_SetItemString(pykwargs, "phrase", input);

            if (pydoc) {
                // This will overwrite whatever kwarg has been passed in
                PyDict_SetItemString(pykwargs, "document", pydoc);
            }

            /* Invoke method on extension */
            PyObject * ret = 0;

            /* Invoke method on extension */
            PyObject * callable = PyObject_GetAttrString(extensionObject(), "on_explore_event");
            if (callable == 0) {
                callable = PyObject_GetAttrString(extensionObject(), "lookup");
            }
            if (callable) {
                ret = PyObject_Call(callable, pyargs, pykwargs);
                Py_DECREF(callable);
            }

            Py_DECREF(pyargs);
            Py_DECREF(pykwargs);

            if (ret == 0) { /* Exception */
                PyErr_PrintEx(0);
            } else {
                // ret is now a sequence of annotation objects
                if (PySequence_Check(ret)) {
                    for (Py_ssize_t i = 0; i < PySequence_Size(ret); ++i) {
                        PyObject * pyAnnotation = PySequence_GetItem(ret, i);
                        Annotation * ann = 0;
                        if (SWIG_ConvertPtr(pyAnnotation,
                                            (void**) &ann,
                                            SWIG_TypeQuery("_p_Annotation"),
                                            0) == 0) {
                            derived.insert(ann->_ann->_handle);
                        }
                    }
                } else { /* Not a sequence! */
                    PyErr_PrintEx(0);
                }

                Py_DECREF(ret);
            }
        }

        Py_XDECREF(pydoc);

        PyGILState_Release(gstate);

        return derived;
    }




    //**** Message bus methods ****//

    void resubscribeToBus()
    {
        // Register on message bus
        if (!_busId.isEmpty()) {
            subscribeToBus();
        }
    }

    void receiveFromBus(const QString & sender, const QVariant & data)
    {
        BusAgent::receiveFromBus(sender, data); // DEBUG

        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        PyObject * recipient = convert(sender);
        PyObject * kwargs = convert(data);
        PyObject * method = PyString_FromString("event");
        PyObject * ret = PyObject_CallMethodObjArgs(extensionObject(), method, recipient, kwargs, NULL);
        Py_XDECREF(ret);
        Py_XDECREF(method);
        Py_XDECREF(kwargs);
        Py_XDECREF(recipient);

        PyGILState_Release(gstate);
    }

    QString busId() const
    {
        if (_busId.isEmpty()) {
            QString uuidstr = QString::fromStdString(uuid());
            return uuidstr.mid(1, uuidstr.size()-2);
        } else {
            return _busId;
        }
    }

    void postToBusFromPython(python::object first, python::object second = python::object())
    {
        QString recipient;
        QVariant data;
        if (second.ptr()) {
            recipient = convert(first).toString();
            data = convert(second);
            postToBus(recipient, data);
        } else {
            data = convert(first);
            postToBus(data);
        }
    }




    std::string title()
    {
        return extensionDocString();
    }

protected:
    QString _busId;

    QStringList _handleableEvents;
    QStringList _handleableLegacyEvents;
    QStringList _handleableEventNames;
};


#undef ANNOTATOR_METHOD
