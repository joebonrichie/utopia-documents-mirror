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

#include <utopia2/element.h>
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
            static QMap< QString, Node* >& elements()
                {
                    return get()._elements;
                }
            // Get generic Element
            static Node* Element()
                {
                    return get()._element;
                }


        private:
            // Constructor
            Registry()
                {
                    // Grab generic Element Node
                    _element = Node::getNode("http://utopia.cs.manchester.ac.uk/2007/03/utopia-domain#Element");

                    // Find all elements beneath it...
                    Node::relation::iterator subclass_iter = _element->relations(~rdfs.subClassOf).begin();
                    Node::relation::iterator subclass_end = _element->relations(~rdfs.subClassOf).end();
                    for (; subclass_iter != subclass_end; ++subclass_iter)
                    {
                        if ((*subclass_iter)->attributes.exists("http://utopia.cs.manchester.ac.uk/2007/03/utopia-domain#formula"))
                        {
                            // Symbol
                            QString symbol = (*subclass_iter)->attributes.get("http://utopia.cs.manchester.ac.uk/2007/03/utopia-domain#formula").toString();

                            // Uppercase!
                            symbol = symbol.toUpper();

                            _elements[symbol] = *subclass_iter;
                        }
                    }

#ifdef DEBUG
                    qDebug() << " * Resolved" << _elements.size() << "elements.";
#endif
                }

            // Maps...
            QMap< QString, Node* > _elements;
            // Generic Element
            Node* _element;

            // Get static Registry
            static Registry& get()
                {
                    static Registry reg;
                    return reg;
                }

        }; // class Registry

    } // anonymous namespace

    /** Get Element term by id. */
    Node* Element::get(QString symbol_, bool generic_)
    {
        // Uppercase!
        symbol_ = symbol_.toUpper();

        // Find in Registry
        QMap< QString, Node* >::const_iterator found = Registry::elements().find(symbol_);
        if (found != Registry::elements().end())
        {
            return found.value();
        }
        else
        {
            return generic_ ? Registry::Element() : 0;
        }
    }

} // namespace Utopia
