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

#include <utopia2/nucleotide.h>
#include <utopia2/node.h>
#include <utopia2/ontology.h>

#include <QMap>
#include <QtDebug>

#include <cctype>

namespace Utopia
{

    namespace
    {

        class Registry
        {
        public:
            // Get map
            static QMap< QString, Node* >& nucleotides()
                {
                    return get()._nucleotides;
                }
            // Get generic Nucleotide
            static Node* Nucleotide()
                {
                    return get()._nucleotide;
                }


        private:
            // Constructor
            Registry()
                {
                    // Grab generic Nucleotide Node
                    _nucleotide = UtopiaDomain.term("Nucleoside");

                    // Convenience
                    Node* p_code = UtopiaDomain.term("code");

                    // Find all nucleotides beneath it...
                    Node::relation::iterator subclass_iter = _nucleotide->relations(~rdfs.subClassOf).begin();
                    Node::relation::iterator subclass_end = _nucleotide->relations(~rdfs.subClassOf).end();
                    for (; subclass_iter != subclass_end; ++subclass_iter)
                    {
                        if ((*subclass_iter)->attributes.exists(p_code))
                        {
                            _nucleotides[(*subclass_iter)->attributes.get(p_code).toString().toUpper()] = *subclass_iter;
                        }
                    }

#ifdef DEBUG
                    qDebug() << " * Resolved" << _nucleotides.size() << "nucleotides.";
#endif
                }

            // Maps...
            QMap< QString, Node* > _nucleotides;
            // Generic Nucleotide
            Node* _nucleotide;

            // Get static Registry
            static Registry& get()
                {
                    static Registry reg;
                    return reg;
                }

        }; // class Registry

    } // anonymous namespace

    /** Get Nucleotide term by id. */
    Node* Nucleotide::get(QString code_, bool generic_)
    {
        // Find in Registry
        QMap< QString, Node* >::iterator found = Registry::nucleotides().find(code_.toUpper());
        if (found != Registry::nucleotides().end())
        {
            return found.value();
        }
        else
        {
            return generic_ ? Registry::Nucleotide() : 0;
        }
    }

} // namespace Utopia
