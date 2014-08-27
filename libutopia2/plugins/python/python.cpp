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

#include "conversion.h"
#include "interpreter.h"

#include "swigruntime.h"

#include "pyextension.h"
#include "pyconfigurator.h"
#ifdef UTOPIA_BUILD_DOCUMENTS
#include "pyannotator.h"
#include "pydecorator.h"
#include "pyoverlayrenderermapper.h"
#include "pyphraselookup.h"
#include "pyvisualiser.h"
#include "pylinkfinder.h"
#endif
#ifdef UTOPIA_BUILD_LIBRARY
#include "pyremotequery.h"
#include "pyresolver.h"
#endif

#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/mpl/vector.hpp>
#include <papyro/cslengine.h>
#include <papyro/cslengineadapter.h>
#include <utopia2/global.h>
#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>
#include <utopia2/pluginmanager.h>
#include <utopia2/plugin.h>

#include <QDir>



#define REGISTER_PYTHON_EXTENSION_FACTORIES(package, cls)                                                   \
    {                                                                                                       \
        PyRun_SimpleString("import " #package);                                                             \
        typedef Utopia::ExtensionFactory< Py ## cls, Py ## cls::API, std::string > Py ## cls ## Factory;    \
            std::set< std::string > typeNames = PythonInterpreter::getTypeNames(#package "." #cls);         \
            BOOST_FOREACH(std::string extensionClass, typeNames)                                            \
            {                                                                                               \
                UTOPIA_REGISTER_EXTENSION_FACTORY_NAMED(                                                    \
                    Py ## cls ## Factory,                                                                   \
                    extensionClass,                                                                         \
                    new Py ## cls ## Factory(extensionClass)                                                \
                    );                                                                                      \
            }                                                                                               \
    }

#define REGISTER_TYPED_PYTHON_EXTENSION_FACTORIES(package, api, cls)                                        \
    {                                                                                                       \
        PyRun_SimpleString("import " #package);                                                             \
        typedef Utopia::ExtensionFactory< Py ## cls, api, std::string > Py ## cls ## Factory;               \
            std::set< std::string > typeNames = PythonInterpreter::getTypeNames(#package "." #cls);         \
            BOOST_FOREACH(std::string extensionClass, typeNames)                                            \
            {                                                                                               \
                UTOPIA_REGISTER_EXTENSION_FACTORY_NAMED(                                                    \
                    Py ## cls ## Factory,                                                                   \
                    extensionClass,                                                                         \
                    new Py ## cls ## Factory(extensionClass)                                                \
                    );                                                                                      \
            }                                                                                               \
    }

namespace python = boost::python;
namespace mpl = boost::mpl;

python::object format_citation(python::object metadata, python::object style = python::object())
{
    boost::shared_ptr< Papyro::CSLEngine > cslengine(Papyro::CSLEngine::instance());

    python::object formatted;
    PyObject * formattedObj = convert(cslengine->format(Papyro::convert_to_cslengine(convert(metadata).toMap()), convert(style).toString()));
    if (formattedObj != Py_None) {
        formatted = python::object(python::handle<>(formattedObj));
    }
    return formatted;
}

python::object unicode(const QString & str)
{
    return python::object(python::handle<>(convert(QVariant(str))));
}

#define SAFE_EXEC(command) \
    try { python::exec(command, global, global); } catch (python::error_already_set e) { PyErr_PrintEx(0); }


// dlsym handles

extern "C" const char * utopia_apiVersion()
{
    return UTOPIA_EXTENSION_LIBRARY_VERSION;
}

extern "C" const char * utopia_description()
{
    return "Python extensions";
}

extern "C" void utopia_registerExtensions()
{
    static PythonInterpreter interpreter;

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // Get main scope
    python::object main = python::import("__main__");
    python::object global = python::extract< python::dict >(main.attr("__dict__"));

    // Prevent bytecode compilation
    python::object sys = python::import("sys");
    sys.attr("dont_write_bytecode") = true;

    // Redirect logging
    global["_log_file"] = unicode(Utopia::profile_path(Utopia::ProfileLogs) + "/python.log");
    SAFE_EXEC("logfile = open(_log_file, 'w', 1)");
    SAFE_EXEC("import sys");
    SAFE_EXEC("sys.stderr = logfile");
    SAFE_EXEC("sys.stdout = logfile");
    SAFE_EXEC("import logging");
    SAFE_EXEC("logging.basicConfig(format='%(asctime)s %(levelname)s %(name)s %(message)s')");

    // Set up MIME information
    global["_mime_dir"] = unicode(Utopia::resource_path() + "/mime");
    SAFE_EXEC(
      "import mimetypes, os\n"
      "mimetypes.init([os.path.join(_mime_dir, filename) for filename in os.listdir(_mime_dir)])"
      );

    // Timestamp log
    SAFE_EXEC(
      "import time\n"
      "print 'Documents started: %s' % time.strftime('%Y %b %d - %H:%M:%S')"
      );

    // These are the allowed places / names for Python plugins
    typedef std::pair< Utopia::Plugin::PluginBase, QString > path_pair;
    std::list< path_pair > paths;
    paths.push_back(path_pair(Utopia::Plugin::InstallBase, QString("python/core")));
    paths.push_back(path_pair(Utopia::Plugin::InstallBase, QString("python/3rdparty")));
    paths.push_back(path_pair(Utopia::Plugin::ProfileBase, QString("python")));
    QStringList names;
    names << "*.py" << "*.zip";

    SAFE_EXEC("import utopia");
    std::list< path_pair > old_paths(paths);
    old_paths.push_back(path_pair(Utopia::Plugin::InstallBase, QString("python")));
    foreach (path_pair path, old_paths) {
        global["_plugin_dir"] = unicode(Utopia::Plugin::constructAbsolutePath(path.first, path.second));
        SAFE_EXEC("utopia.extension.cleanPluginDir(_plugin_dir)");
    }

    {
        try {
        python::object module(python::import("utopia"));
        python::object citation(module.attr("citation"));
        python::scope outer(citation);
        python::def("format", python::make_function(bind(format_citation, _1, python::object()), python::default_call_policies(), mpl::vector< python::object, python::object >()));
        python::def("format", python::make_function(bind(format_citation, _1, _2), python::default_call_policies(), mpl::vector< python::object, python::object, python::object >()));
        } catch (python::error_already_set e) { PyErr_PrintEx(0); }
    }

    // Resolve all plugins found in the above specified plugins paths.
    // FIXME deleting unused ones?
    boost::shared_ptr< Utopia::PluginManager > pluginManager(Utopia::PluginManager::instance());
    foreach (path_pair path, paths) {
        //qDebug() << "path" << path.first << path.second;
        QDir dir(Utopia::Plugin::constructAbsolutePath(path.first, path.second));
        dir.setFilter(QDir::Files | QDir::Dirs);
        dir.setNameFilters(names);
        foreach (QString script, dir.entryList()) {
            //qDebug() << "resolve" << path.first << (path.second + "/" + script);
            pluginManager->resolve(path.first, path.second + "/" + script);
        }
    }

    // Load discovered plugins
    foreach (Utopia::Plugin * plugin, pluginManager->plugins()) {
        QString absolutePath = plugin->absolutePath();
        //qDebug() << "absolutePath" << absolutePath;
        if (QFile::exists(absolutePath)) {
            global["_plugin_path"] = unicode(absolutePath);
            SAFE_EXEC("utopia.extension.loadPlugin(_plugin_path)");
        }
    }

    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia, Configurator)
#ifdef UTOPIA_BUILD_DOCUMENTS
    REGISTER_TYPED_PYTHON_EXTENSION_FACTORIES(utopia.document, Papyro::SelectionProcessorFactory, PhraseLookup)
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.document, OverlayRendererMapper)
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.document, Decorator)
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.document, Annotator)
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.document, Visualiser)
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.document, LinkFinder)
#endif
#ifdef UTOPIA_BUILD_LIBRARY
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.library, RemoteQuery)
    REGISTER_PYTHON_EXTENSION_FACTORIES(utopia.library, Resolver)
#endif

    PyGILState_Release(gstate);
}
