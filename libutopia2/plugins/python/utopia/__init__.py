###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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


def get_plugin_data(path):
    try:
        # Find next __loader__ object by inspection
        import inspect, sys
        loader = None
        for frame in inspect.stack():
            loader = frame[0].f_globals.get('__loader__')
            if loader is None:
                plugin_name = frame[0].f_globals.get('__plugin__', None)
                if plugin_name is not None:
                    loader = getattr(sys.modules.get(plugin_name), '__loader__')
            if loader is not None:
                break

        # If a loader is found, use it
        if loader is not None:
            return loader.get_data(path)
    except:
        pass

def get_plugin_data_as_url(path, mime):
    try:
        # If a data is found, create a data URL
        data = get_plugin_data(path)
        if data is not None:
            import base64
            encoded = base64.standard_b64encode(data)
            return 'data:{0};base64,{1}'.format(mime, encoded)
    except:
        pass

from utopiacore import *
