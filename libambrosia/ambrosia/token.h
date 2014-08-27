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

#ifndef AMBROSIA_TOKEN_H
#define AMBROSIA_TOKEN_H

#include <ambrosia/config.h>
#include <utopia2/utopia2.h>

#include <string>
#include <map>

namespace AMBROSIA
{

    class LIBAMBROSIA_API token
    {
    public:
        // Static constuction method
        static token get(const std::string& name_);

        // Destructor
        ~token();

        // Getters
        std::string name();
        unsigned int value();

        // Implicit casts
        operator std::string ();
        operator unsigned int ();

    private:
        // Constructor
        token(const std::string& name_);

        // Token name
        std::string _name;
        // Token value
        unsigned int _value;

        // Static list of tokens
        static unsigned int _next_value;
        static std::map< std::string, unsigned int > _tokens;

    }; /* class token */

} /* namespace AMBROSIA */

#endif /* AMBROSIA_TOKEN_H */
