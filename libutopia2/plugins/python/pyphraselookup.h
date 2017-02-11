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

#include <papyro/documentview.h>
#include <papyro/papyrowindow.h>
#include <papyro/selectionprocessor.h>
#include <papyro/utils.h>
#include <string>
#include <iostream>

class PyPhraseLookupInstance : public PyExtension, public Papyro::SelectionProcessor
{
public:
    PyPhraseLookupInstance(std::string extensionClassName)
        : PyExtension("utopia.document.PhraseLookup", extensionClassName)
    {
        static QRegExp stripper("^(\\d*)([^\\d]*)$");
        QString title = Papyro::qStringFromUnicode(extensionDocString());
        stripper.exactMatch(title);
        _category = stripper.cap(1).toInt();
        _title = stripper.cap(2);
    }

    int category() const
    {
        return _category;
    }

    void processSelection(Spine::DocumentHandle document, Spine::CursorHandle, const QPoint & globalPos)
    {
        std::string link;

        if (extensionObject())
        {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            std::string phrase = document->selectionText();

            PyObject * input = PyUnicode_DecodeUTF8(phrase.data(), phrase.length(), 0);
            if (input)
            {
                PyObject * output = PyObject_CallMethod(extensionObject(), (char *) "lookup", (char *) "(O)", input);
                Py_DECREF(input);
                if (output)
                {
                    link = PyString_AsString(output);
                    Py_DECREF(output);
                }
                else
                {
                    PyErr_PrintEx(0);
                }
            }
            else
            {
                PyErr_PrintEx(0);
            }

            PyGILState_Release(gstate);
        }

        if (!link.empty()) {
            QUrl url(Papyro::qStringFromUnicode(link));
            Papyro::PapyroWindow::currentWindow()->requestUrl(url);
        }
    }

    QString title() const
    {
        return "Lookup Selected Text | " + _title;
    }

    int weight() const
    {
        return 0;
    }

private:
    QString _title;
    int _category;
};




class PyPhraseLookup : public Papyro::SelectionProcessorFactory
{
public:
    PyPhraseLookup(std::string extensionClassName)
        : extensionClassName(extensionClassName)
    {}

    QList< boost::shared_ptr< Papyro::SelectionProcessor > > selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
    {
        QList< boost::shared_ptr< Papyro::SelectionProcessor > > list;
        if (hasTextSelection(document, cursor)) {
            PyPhraseLookupInstance * a = new PyPhraseLookupInstance(extensionClassName);
            list << boost::shared_ptr< Papyro::SelectionProcessor >(a);
        }
        return list;
    }

private:
    std::string extensionClassName;

}; // class PyPhraseLookup
