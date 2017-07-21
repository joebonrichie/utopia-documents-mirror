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

import exceptions

class ConversionError(exceptions.ValueError):
    pass

class NotImplementedError(ConversionError):
    """Raised when attempting to call an unimplemented converter method"""
    pass

class ParseError(ConversionError):
    """Raised when attempting to parse a format"""
    pass

class FormatError(ConversionError):
    """Raised when attempting to parse a format of the wrong schema"""
    pass

class VersionError(ConversionError):
    """Raised when attempting to parse a format of the wrong version"""
    pass

class SerialisationError(ConversionError):
    """Raised when attempting to serialise an incomplete object"""
    pass

class BaseConverter:

    @staticmethod
    def parse(*args, **kwargs):
        raise NotImplementedError()

    @staticmethod
    def serialise(*args, **kwargs):
        raise NotImplementedError()

class InvalidConverter:

    @staticmethod
    def parse(*args, **kwargs):
        raise ParseError('Unable to find parser for requested class (%s).' % str(args[0]))

    @staticmethod
    def serialise(*args, **kwargs):
        raise SerialisationError('Unable to find serialiser for requested class (%s).' % str(args[0]))


MIME_TYPE_BASE = 'application/x-authd'
