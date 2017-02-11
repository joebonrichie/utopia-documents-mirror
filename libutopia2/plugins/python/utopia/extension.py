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

import os, sys, fnmatch, uuid, inspect, traceback

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
            print('    Found {}'.format(cls))
            # Give this class' module the name of its loaded plugin
            inspect.getmodule(cls).__dict__['__plugin__'] = inspect.stack()[-3][0].f_globals['__name__']

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
def cleanPluginDir(dir):
    print('Cleaning path: {}'.format(dir))
    if os.path.isdir(dir):
        # Clear up dir
        for doomed in os.listdir(dir):
            if fnmatch.fnmatch(doomed, '[!_]*.py[co]'):
                os.unlink(os.path.join(dir, doomed))

def _makeLoader(module_path):
    class Loader:
        def get_data(self, data_path):
            try:
                path = os.path.join(module_path, data_path)
                return open(path, 'r').read()
            except:
                traceback.print_exc()
    return Loader()

# Load all extensions from a given plugin object
def loadPlugin(path):
    dir, p = os.path.split(path)
    if fnmatch.fnmatch(p, '_*'): # bail if underscored
        return
    print('Loading extensions from: {}'.format(path))
    if os.path.isdir(path):
        if fnmatch.fnmatch(p, '*.zip'):
            try:
                sys.path.append(os.path.join(dir, p, 'python'))
                mod = __import__(os.path.splitext(p)[0])
                mod.__file__ = path
                mod.__loader__ = _makeLoader(path)
                sys.path.pop()
            except Exception as e:
                traceback.print_exc()
                print('Failed to load {}\n{}'.format(p, e))

    else:
        # Attempt to load the plugin
        if fnmatch.fnmatch(p, '*.py'):
            try:
                sys.path.append(dir)
                __import__(os.path.splitext(p)[0]).__file__ = path
                sys.path.pop()
            except Exception as e:
                traceback.print_exc()
                print('Failed to load {}\n{}'.format(p, e))
        elif fnmatch.fnmatch(p, '*.zip'):
            try:
                sys.path.append(os.path.join(dir, p, 'python'))
                __import__(os.path.splitext(p)[0]).__file__ = path
                sys.path.pop()
            except Exception as e:
                traceback.print_exc()
                print('Failed to load {}\n{}'.format(p, e))

# Load all plugins that can be found in the UTOPIA_PLUGIN_PATH env var
if 'UTOPIA_PLUGIN_PATH' in os.environ:
    for path in os.environ.get('UTOPIA_PLUGIN_PATH', '').split(os.pathsep):
        path = path.strip()
        print('--- {}'.format(path))
        if os.path.isdir(path) and path[-4:] != '.zip':
            for plugin in os.listdir():
                loadPlugin(plugin)
        else:
            loadPlugin(path)

__all__ = ['Extension', 'loadPlugin']
