###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2017 Lost Island Labs
#            <info@utopiadocs.com>
#    
#    Utopia Documents is free software: you can redistribute it and/or modify
#    it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
#    published by the Free Software Foundation.
#    
#    Utopia Documents is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#    Public License for more details.
#    
#    In addition, as a special exception, the copyright holders give
#    permission to link the code of portions of this program with the OpenSSL
#    library under certain conditions as described in each individual source
#    file, and distribute linked combinations including the two.
#    
#    You must obey the GNU General Public License in all respects for all of
#    the code used other than OpenSSL. If you modify file(s) with this
#    exception, you may extend this exception to your version of the file(s),
#    but you are not obligated to do so. If you do not wish to do so, delete
#    this exception statement from your version.
#    
#    You should have received a copy of the GNU General Public License
#    along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
#   
###############################################################################

import fnmatch
import imp
import inspect
import os
import sys
import uuid
import zipimport
from utopia.log import logger



# Define metaclass for managing extensions
class MetaExtension(type):
    __extensions = dict()

    def _typeName(cls):
        return '{}.{}'.format(cls.__module__, cls.__name__)

    def __init__(cls, name, bases, attrs):
        # Keep track of subclasses of Extension
        if not attrs.get('__module__', '').startswith('utopia.') and not cls.__name__.startswith('_'):
            cls.__extensions[cls._typeName()] = cls
            cls.__uuid__ = uuid.uuid4().urn
            logger.debug('    Found {}'.format(cls))
            # Give this class's module the name of its loaded plugin
            if len(inspect.stack()) >= 5:
                inspect.getmodule(cls).__dict__['__plugin__'] = inspect.stack()[-5][0].f_globals['__name__']

    def __del__(cls):
        # Keep track of subclasses of Extension
        del cls.__extensions[cls._typeName()]

    def types(cls):
        return tuple([c for c in cls.__extensions.values() if issubclass(c, cls) and c != cls])

    def typeNames(cls):
        return [n for (n, c) in cls.__extensions.iteritems() if issubclass(c, cls) and c != cls]

    def typeOf(cls, name):
        return cls.__extensions[name]

    def describe(cls, name):
        return str(cls.__doc__).strip()

# Abstract base class for extensions
class Extension(object):
    __metaclass__ = MetaExtension
    def uuid(self):
        return getattr(self, '__uuid__', None)

# Clear up directory of cached Python plugins
def cleanPluginDir(directory):
    logger.debug('Cleaning path: {}'.format(directory))
    if os.path.isdir(directory):
        # Clear up directory
        for doomed in os.listdir(directory):
            if fnmatch.fnmatch(doomed, '[!_]*.py[co]'):
                os.unlink(os.path.join(directory, doomed))

def _makeLoader(module_path):
    class Loader:
        def get_data(self, data_path):
            try:
                path = os.path.join(module_path, data_path)
                return open(path, 'rb').read()
            except:
                logger.error('Error opening module data file', exc_info=True)
    return Loader()

# Load all extensions from a given plugin object
def loadPlugin(path):
    # Split the path up
    directory, filename = os.path.split(path)
    basename, ext = os.path.splitext(filename)
    mod_name = 'plugin_{}_{}'.format(str(uuid.uuid4()).replace('-', ''), basename)

    # Only python files and zips are acceptable
    is_py_file = ext == '.py' and not os.path.isdir(path)
    is_zip_file = ext == '.zip' and not os.path.isdir(path)
    is_zip_dir = ext == '.zip' and os.path.isdir(path)
    #logger.info((path, ext, is_py_file, is_zip_file, is_zip_dir))
    if is_py_file or is_zip_file or is_zip_dir:
        # Any filename beginning with an underscore is ignored
        if filename.startswith('_'):
            logger.info('Ignoring underscored file: {}'.format(path))
            return

        # Attempt to load the plugin
        logger.debug('Loading extensions from: {}'.format(path))
        if is_zip_dir:
            entry = os.path.join(path, 'python', basename + '.py')
            if os.path.exists(entry):
                try:
                    sys.path.append(os.path.join(directory, filename, 'python'))
                    mod = imp.load_source(mod_name, entry)
                    mod.__file__ = path
                    mod.__loader__ = _makeLoader(path)
                except Exception as e:
                    logger.error('Failed to load %s', filename, exc_info=True)
                finally:
                    sys.path.pop()
            else:
                logger.error('Cannot find entry point %s in zip directory: %s', os.path.join('python', basename + '.py'), path)
        elif is_py_file:
            try:
                mod = imp.load_source(mod_name, path)
                mod.__file__ = path
            except Exception as e:
                logger.error('Failed to load %s', filename, exc_info=True)
        elif is_zip_file:
            try:
                importer = zipimport.zipimporter(os.path.join(path, 'python'))
                mod = importer.load_module(mod_name)
            except Exception as e:
                logger.error('Failed to load %s', filename, exc_info=True)

# Load all plugins from a given directory
def loadPlugins(path):
    if os.path.isdir(path) and not path.endswith('.zip'):
        for plugin in os.listdir(path):
            if plugin.endswith('.py') or plugin.endswith('.zip'):
                loadPlugin(os.path.join(path, plugin))
    elif path.endswith('.py') or path.endswith('.zip'):
        loadPlugin(path)

__all__ = ['Extension', 'loadPlugin', 'loadPlugins']
