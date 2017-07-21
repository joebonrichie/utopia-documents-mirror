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

#include "interpreter.h"

#include <utopia2/global.h>
#include <boost/python.hpp>
#include <iostream>
#include <stdlib.h>
#include <datetime.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <QDir>
#include <QDebug>


namespace python = boost::python;


namespace
{

    QString joinPath(const QString& s1_, const QString& s2_, const QString& s3_ = "", const QString& s4_ = "", const QString& s5_ = "", const QString& s6_ = "")
    {
        QString sep = "/";
        QString path = s1_ + sep + s2_;
        if (!s3_.isEmpty()) {
            path += sep + s3_;
            if (!s4_.isEmpty()) {
                path += sep + s4_;
                if (!s5_.isEmpty()) {
                    path += sep + s5_;
                    if (!s6_.isEmpty()) {
                        path += sep + s6_;
                    }
                }
            }
        }

        return QDir::toNativeSeparators(path);
    }

}

#define PYTHON_STEP ("python" PYTHON_VERSION)

PythonInterpreter::PythonInterpreter()
{
    // Initialise python interpreter
#ifdef _WIN32
    //QString home(joinPath(Utopia::private_library_path(), "python").replace("\\", "/"));
    //Py_SetPythonHome((char*) home.toUtf8().constData());
    putenv(("PYTHONHOME=" + joinPath(Utopia::private_library_path(), "python")).toUtf8().constData());
    putenv("PYTHONHTTPSVERIFY=0");
    //SetEnvironmentVariable("PYTHONHOME", joinPath(Utopia::private_library_path(), "python").toUtf8().constData());
#elif __linux__
    Py_SetProgramName((char *) "/usr/bin/python2.7");
    ::setenv("PYTHONPATH", joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP, "site-packages").toUtf8().constData(), 1);
    ::setenv("PYTHONHTTPSVERIFY", "0", 1);
#else
    ::setenv("PYTHONHOME", joinPath(Utopia::private_library_path(), "python").toUtf8().constData(), 1);
    ::setenv("PYTHONPATH", joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP).toUtf8().constData(), 1);
    ::setenv("PYTHONHTTPSVERIFY", "0", 1);
#endif

    Py_Initialize();
    PyEval_InitThreads();

    // Import sys module
    PyObject* sysName = PyString_FromString("sys");
    PyObject* sys = PyImport_Import(sysName);
    Py_DECREF(sysName);
    PyObject* path = PyObject_GetAttrString(sys, "path");

#ifdef _WIN32
    // When we're using a bundled Python, empty out sys.path, and add the root
    // of the bundled Python
    {
        //PySequence_DelSlice(path, 0, PySequence_Size(path));
        PyObject* root = Py_BuildValue("[s]",
                                       joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP).toUtf8().constData());
        PySequence_SetSlice(path, 0, 0, root);
        Py_DECREF(root);
    }

    // Append bundled paths to sys.path
    {
        PyObject* paths = Py_BuildValue("[sssss]",
                                        joinPath(Utopia::private_library_path(), "python", "dlls").toUtf8().constData(),
                                        joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP, "lib-dynload").toUtf8().constData(),
                                        joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP, "plat-win").toUtf8().constData(),
                                        joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP, "lib-dynload").toUtf8().constData(),
                                        joinPath(Utopia::private_library_path(), "python", "lib", PYTHON_STEP, "lib-tk").toUtf8().constData());
        Py_ssize_t end = PySequence_Size(path);
        PySequence_SetSlice(path, end, end, paths);
        Py_DECREF(paths);
    }

    // Prepend to sys.path any standard Utopia paths
    {
        PyObject* paths = Py_BuildValue("[s]",
                                        joinPath(Utopia::plugin_path(), "python").toUtf8().constData());
        PySequence_SetSlice(path, 0, 0, paths);
        Py_DECREF(paths);
    }
#endif

    // Reset search path
    Py_DECREF(path);
    Py_DECREF(sys);

    // Release GIL
    pyThreadState = PyThreadState_Swap(NULL);
    PyEval_ReleaseLock();
}

PythonInterpreter::~PythonInterpreter()
{
    // Synchronise with GIL
    PyEval_AcquireLock();
    PyThreadState_Swap(pyThreadState);
    PyEval_ReleaseLock();


#ifndef _WIN32 // FIXME horrid hack to stop this hanging on Windows
    Py_Finalize();
#endif
}

PythonInterpreter & PythonInterpreter::instance()
{
    static PythonInterpreter interpreter;
    return interpreter;
}

std::set< std::string > PythonInterpreter::getTypeNames(const std::string & api)
{
    std::set< std::string > extensionClasses;


    if (PyObject * main = PyImport_AddModule("__main__"))
    {
        PyObject * dict = PyModule_GetDict(main);
        std::string cmd(api + ".typeNames()");

        if (PyObject * extensionClassTuple = PyRun_String(cmd.c_str(), Py_eval_input, dict, dict))
        {

            if (PySequence_Check(extensionClassTuple))
            {
                int rows = PySequence_Size(extensionClassTuple);
                for (int j = 0; j < rows; ++j)
                {
                    PyObject * extensionClass = PySequence_GetItem(extensionClassTuple, j);
                    extensionClasses.insert(PyString_AsString(extensionClass));
                }
            }

            Py_DECREF(extensionClassTuple);
        }
        else
        {
            PyErr_Print();
        }
    }

    return extensionClasses;
}
