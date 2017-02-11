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

#ifndef Utopia_SERVICE_INTERFACE_H
#define Utopia_SERVICE_INTERFACE_H

#include <utopia2/config.h>
#include <utopia2/extension.h>

#include <string>
#include <vector>

namespace Utopia
{

    class Node;
    class Invocation;

    /**
     *  \class Plugin
     *  \brief The abstract Plugin class.
     */
    class LIBUTOPIA_API Plugin
    {
    public:
        typedef Plugin API;

        // Virtual Destructor
        virtual ~Plugin() {};

        // Invocation methods
        virtual bool invoke(Invocation * invocation, std::vector< Node * > input) = 0;

        // Get description information
        const std::vector< Node * > & input();
        Node * action();
        const std::vector< Node * > & output();
        std::string name() const;

    protected:
        // Triple of input, action and output
        std::vector< Node * > _input;
        Node * _action;
        std::vector< Node * > _output;

        // Name
        std::string _name;

    }; /* class Plugin */

} /* namespace Utopia */

UTOPIA_DECLARE_EXTENSION_CLASS(LIBUTOPIA, Utopia::Plugin)

#endif /* Utopia_SERVICE_INTERFACE_H */
