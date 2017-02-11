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

#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/nucleotide.h>
#include <utopia2/aminoacid.h>
#include <utopia2/element.h>

#include <QString>
#include <QMap>

namespace Utopia {

    //
    // PDBParser Parser class
    //

    class PDBParser : public Parser {

    public:
        // typedefs
        typedef QMap< QString, bool > StructureMap;

        // Dummy bond placeholders
        class DummySSBOND {
            QChar initChainId;
            QString initSeqId;
            QChar initICode;
            QChar endChainId;
            QString endSeqId;
            QChar endICode;
        };
        class DummySLTBRG {
            QChar initChainId;
            QString initSeqId;
            QChar initICode;
            QChar endChainId;
            QString endSeqId;
            QChar endICode;
        };
        class DummyCISPEP {
            QChar initChainId;
            QString initSeqId;
            QChar initICode;
            QChar endChainId;
            QString endSeqId;
            QChar endICode;
        };
        class DummyLINK {
            QChar initChainId;
            QString initSeqId;
            QChar initICode;
            QChar endChainId;
            QString endSeqId;
            QChar endICode;
        };
        class DummyHYDBND {
            QChar initChainId;
            QString initSeqId;
            QChar initICode;
            QChar endChainId;
            QString endSeqId;
            QChar endICode;
        };
        class Heterogen {
        public:
            // Constructor
            Heterogen(QString hetID)
                : hetID(hetID), name(""), chainId(0), seqId(), iCode(0), numHetAtoms(0), text("")
                {}

            // Members
            QString hetID;
            QString name;
            QChar chainId;
            QString seqId;
            QChar iCode;
            size_t numHetAtoms;
            QString text;
        };
        class Turn {
        public:
            // Constructor
            Turn(QChar chainId, QString initSeqId, QString endSeqId)
                : chainId(chainId), initSeqId(initSeqId), endSeqId(endSeqId), length(0)
                {}

            // Members
            QChar chainId;
            QString initSeqId;
            QString endSeqId;
            size_t length;
        };
        class Sheet {
        public:
            // Constructor
            Sheet(QChar chainId, QString initSeqId, QString endSeqId)
                : chainId(chainId), initSeqId(initSeqId), endSeqId(endSeqId), length(0)
                {}

            // Members
            QChar chainId;
            QString initSeqId;
            QString endSeqId;
            size_t length;
        };
        class Helix {
        public:
            // Constructor
            Helix(QChar chainId, QString initSeqId, QString endSeqId)
                : chainId(chainId), initSeqId(initSeqId), endSeqId(endSeqId), length(0)
                {}

            // Members
            QChar chainId;
            QString initSeqId;
            QString endSeqId;
            size_t length;
        };

        // Constructor
        PDBParser();
        // Destructor
        ~PDBParser() {};

        // Parse!
        Node* parse(Parser::Context& ctx, QIODevice& stream_) const;
        QSet< FileFormat* > formats() const;
        QString description() const;

    }; // class PDBParser

} // namespace Utopia
