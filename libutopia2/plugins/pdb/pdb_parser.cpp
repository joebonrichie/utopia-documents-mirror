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

#include "pdb_parser.h"
#include <gtl/matrix.h>

namespace Utopia {

    // Constructor
    PDBParser::PDBParser()
        : Parser()
    {}

    // strip whitespace from a std::string
    std::string strip(std::string input) {
        if (input == "") return "";

        size_t start = 0;
        size_t end = input.size() - 1;

        // strip whitespace
        for (start = 0; start < input.size() && std::string(" \n\r\t;").find(input[start]) != std::string::npos; ++start);
        for (end = input.size() - 1; std::string(" \n\r\t;").find(input[end]) != std::string::npos; --end);

        return input.substr(start, 1 + end - start);
    }

/*
  MODEL::MolecularContainer * PDBParser::load(std::istream & stream, MODEL::MolecularContainer * existing) {
  std::string log = "Decoding stream as ";
  log += getDescription();
  Logger::log(log, 1);

  if (!stream) {
  Logger::log(Logger::U_WARNING, "Empty stream, no model loaded");
  return 0;
  }

  MODEL::MolecularContainer * source = new MODEL::MolecularContainer(MODEL::Name::source);
  MODEL::MolecularContainer * model = source->add(MODEL::Name::complex);
  MODEL::MolecularContainer * secondaryStructure = source->add("annotation");
  secondaryStructure->setAttribute<std::string>("name", "Secondary Structure");
  secondaryStructure->setAttribute<std::string>("class", "group");
  secondaryStructure = secondaryStructure->add("annotation");
  secondaryStructure->setAttribute<std::string>("name", "PDB");
  secondaryStructure->setAttribute<std::string>("class", "group");

  try {
  // Local variables required for importing the structure and data
  MODEL::MolecularContainer * molecule = 0;
  std::map< size_t, MODEL::MolecularContainer * > moleculeIdToMolecule;
  MODEL::MolecularContainer * chain = 0;
  MODEL::MolecularContainer * chain_het = 0;
  MODEL::MolecularContainer * residue = 0;
  MODEL::MolecularContainer * backbone = 0;
  MODEL::MolecularContainer * sidechain = 0;
  MODEL::Atom * atom = 0;
  std::map< int, MODEL::Atom * > atoms;
  std::map< std::pair< char, std::string >, MODEL::MolecularContainer * > residues;
  std::set< MODEL::Bond * > bonds;
  std::set< DummySSBOND* > _dummySSBONDs;
  std::set< DummyHYDBND* > _dummyHYDBNDs;
  std::set< DummyLINK* > _dummyLINKs;
  std::set< DummyCISPEP* > _dummyCISPEPs;
  std::set< DummySLTBRG* > _dummySLTBRGs;
  std::vector< std::map< std::string, std::string > > compndInfo;
  std::string lastKey = "MOLECULE";
  std::list< Heterogen > hetInfo;
  std::string utopia_name = "Unknown Model";
  std::string utopia_description = "No Header Information Found";
  std::string classification = "";
  std::string title = "";
  std::string date = "";
  std::string pdbcode = "";
  std::list< Turn > turnInfo;
  std::list< Helix > helixInfo;
  std::list< Sheet > sheetInfo;
  bool firstModel = true;
  std::vector< gtl::matrix_4d > matrices;

  while (!stream.eof() && firstModel) {
  std::string line = "";
  getline(stream, line);
  if (line == "")
  {
  continue;
  }
  if (line.length() < 80)
  {
  line += std::string(80 - line.length(), ' ');
  }
//                  cout << line << endl;

std::string recordtype = line.substr(0, 6);

if (recordtype == "COMPND") {
std::string compnd = strip(line.substr(10, 60));
std::string key = "";
std::string value = "";

// Parse compound record
size_t colonIndex = compnd.find(':');
if (colonIndex == std::string::npos) {
key = lastKey;
value = compnd;
} else {
key = strip(compnd.substr(0, colonIndex));
value = strip(compnd.substr(colonIndex + 1));
}
if (compndInfo.size() == 0 || key == "MOL_ID") {
compndInfo.push_back(std::map< std::string, std::string >());
if (key != "MOL_ID")
compndInfo.back()["MOL_ID"] = "1";
}
compndInfo.back()[key] += (compndInfo.back()[key] == "" ? "" : " ") + value;
lastKey = key;
} else if (recordtype == "HET   ") {
Heterogen * het = 0;
hetInfo.push_back(Heterogen(strip(line.substr(7, 3))));
het = &hetInfo.back();
het->chainId = line[12];
het->seqId = strip(line.substr(13, 4));
het->iCode = line[17];
het->text = strip(line.substr(30, 40));
} else if (recordtype == "TER   ") {
//                      chain = 0;
} else if (recordtype == "ENDMDL") {
firstModel = false;
} else if (recordtype == "HETNAM") {
std::string hetID = strip(line.substr(11, 3));
std::string name = strip(line.substr(15, 55));
std::list< Heterogen >::iterator het = hetInfo.begin();
std::list< Heterogen >::iterator end = hetInfo.end();
for (; het != end; ++het)
if ((*het).hetID == hetID)
(*het).name = name;
} else if (recordtype == "TURN  ") {
char chainId = line[19];
std::string initSeqId = strip(line.substr(20, 4));
std::string endSeqId = strip(line.substr(31, 4));
turnInfo.push_back(Turn(chainId, initSeqId, endSeqId));
} else if (recordtype == "HELIX ") {
char chainId = line[19];
std::string initSeqId = strip(line.substr(21, 4));
std::string endSeqId = strip(line.substr(33, 4));
helixInfo.push_back(Helix(chainId, initSeqId, endSeqId));
} else if (recordtype == "SHEET ") {
char chainId = line[21];
std::string initSeqId = strip(line.substr(22, 4));
std::string endSeqId = strip(line.substr(33, 4));
sheetInfo.push_back(Sheet(chainId, initSeqId, endSeqId));
} else if (recordtype == "HEADER") {
classification = strip(line.substr(9, 41));
date = strip(line.substr(50, 9));
pdbcode = strip(line.substr(62, 4));
if (pdbcode.size() != 4)
{
pdbcode = "????";
}
if (classification == "")
{
classification = "*Unclassified entry*";
}
model->setAttribute<std::string>("classification", classification);
model->setAttribute<std::string>("date", date);
model->setAttribute<std::string>("pdbcode", pdbcode);
utopia_name = pdbcode;
utopia_description = classification;
} else if (recordtype == "TITLE ") {
    if (title != "")
        title += " ";
    title += strip(line.substr(10, 60));
    if (strip(title) == "")
    {
        title = "*Unnamed entry*";
    }
    model->setAttribute<std::string>("title", title);
    utopia_description = title;
} else if (recordtype == "REMARK") {
    int number = atoi(line.substr(7, 3).c_str());
    switch (number)
    {
    case 350:
        if (line.substr(13, 5) == "BIOMT")
        {
            size_t row = atoi(line.substr(18, 1).c_str());
            size_t id = atoi(line.substr(20, 3).c_str());
            double r1 = atof(line.substr(24, 9).c_str());
            double r2 = atof(line.substr(34, 9).c_str());
            double r3 = atof(line.substr(44, 9).c_str());
            double t = atof(line.substr(54, 14).c_str());
            if (id > matrices.size())
            {
                matrices.push_back(gtl::matrix_4d::identity());
            }
            matrices[id - 1].row(row - 1, r1, r2, r3, t);
        }
        break;
    }
} else if (recordtype == "ATOM  ") {
    long int serial = atoi(line.substr(6, 5).c_str());
    std::string name = line.substr(12, 4);
    char remoteness = line[14];
    char branch = line[15];
    char altLoc = line[16];
    std::string resSymbol = strip(line.substr(17, 3));
    char chainId = line[21];
    std::string seqId = strip(line.substr(22, 4));
//                         char iCode = line[26];
    float x = atof(line.substr(30, 8).c_str());
    float y = atof(line.substr(38, 8).c_str());
    float z = atof(line.substr(46, 8).c_str());
//                         float occupancy = atof(line.substr(54, 6).c_str());
    float tempFactor = atof(line.substr(60, 6).c_str());
    std::string segID = strip(line.substr(72, 4));
    std::string element = "";
    if (line[77] >= 'A' && line[77] <= 'Z')
        element = strip(line.substr(76, 2));
    int charge = 0;
    char chargeSign = line[79];
    if (chargeSign == '+' || chargeSign == '-') {
        charge = line[78] - '0';
        if (chargeSign == '-') charge *= -1;
    }

// Is this part of a protein, nucleic acid or heterogen?
    std::string moleculeClass = "";
    if (MODEL::Nucleotide::getNucleotide(resSymbol) != 0) moleculeClass = MODEL::Name::nucleicacid;
    else if (MODEL::AminoAcid::getAminoAcid(resSymbol) != 0) moleculeClass = MODEL::Name::protein;
    else moleculeClass = MODEL::Name::heterogen;

// Initialise new chains
    if (chain == 0 || !chain->hasAttribute("chainId") || chainId != chain->getAttribute<char>("chainId")) {
        size_t moleculeId = 1;
        size_t compndIndex = 0;
        if (compndInfo.size() > 1) {
            molecule = 0;

// If this chain is on an existing molecule, then use that molecule
            std::vector< std::map< std::string, std::string > >::iterator iter = compndInfo.begin();
            std::vector< std::map< std::string, std::string > >::iterator end = compndInfo.end();
            for (; iter != end; ++iter, ++compndIndex) {
                if ((*iter)["CHAIN"].find(chainId) != std::string::npos) {
                    moleculeId = atoi((*iter)["MOL_ID"].c_str());
                    if (moleculeIdToMolecule.find(moleculeId) != moleculeIdToMolecule.end())
                        molecule = moleculeIdToMolecule[moleculeId];
                    break;
                }
            }
        }

// else create a new molecule
        if (molecule == 0) {
            molecule = model->add(moleculeClass);
            moleculeIdToMolecule[moleculeId] = molecule;
            std::string name = "";
            if (compndIndex < compndInfo.size()) {
                name = compndInfo[compndIndex]["MOLECULE"];
            }
            molecule->setAttribute<std::string>("name", name != "" ? name : "unknown");
        }

        chain = molecule->add(MODEL::Name::chain);
        chain->setAttribute<char>("chainId", chainId);
    }

// Initialise new residues
    if (residue == 0 || seqId != residue->getAttribute<std::string>("seqId")) {
        if (moleculeClass == MODEL::Name::protein) {
            residue = chain->add(MODEL::Name::aminoacid);
            residues[std::pair< char, std::string >(chainId, seqId)] = residue;
            if (MODEL::AminoAcid::getAminoAcid(resSymbol) == 0) {
                std::string log = "Can't find amino acid '";
                log += resSymbol + "', inserting 'X' instead";
                Logger::log(Logger::U_WARNING, log);
                resSymbol = "X";
            }
            residue->setNodeData<MODEL::AminoAcid*>(MODEL::AminoAcid::getAminoAcid(resSymbol));
            residue->setAttribute<std::string>("seqId", seqId);
        } else if (moleculeClass == MODEL::Name::nucleicacid) {
            residue = chain->add(MODEL::Name::nucleotide);
            if (MODEL::Nucleotide::getNucleotide(resSymbol) == 0) {
                std::string log = "Can't find nucleotide ,";
                log += resSymbol + "', inserting 'X' instead";
                Logger::log(Logger::U_WARNING, log);
                resSymbol = "X";
            }
            residue->setNodeData<MODEL::Nucleotide*>(MODEL::Nucleotide::getNucleotide(resSymbol));
            residue->setAttribute<std::string>("seqId", seqId);
        }
        if (moleculeClass != MODEL::Name::heterogen) {
            backbone = residue->add(MODEL::Name::backbone);
            sidechain = residue->add(MODEL::Name::sidechain);
        }
    }

// Is this the primary record or an alternative location?
    if (altLoc == ' ' || altLoc == 'A') {
// Create atom
        std::string symbol = "";
        if (name[0] == ' ' || (name[0] >= '1' && name[0] <='9')) symbol = name.substr(1,1);
        else symbol = name.substr(0,2);
        atom = new MODEL::Atom(symbol);
        if (moleculeClass == MODEL::Name::protein) {
            if (remoteness == ' ' || remoteness == 'A')
                backbone->add(atom);
            else
                sidechain->add(atom);
        } else if (moleculeClass == MODEL::Name::nucleicacid) {
            if (branch == '*' || branch == 'P' || remoteness == ' ')
                backbone->add(atom);
            else
                sidechain->add(atom);
        } else {
            chain->add(atom);
        }
        atom->setSerial(serial);
        atom->setPosition(x, y, z);
        atom->setTemperature(tempFactor);
        atom->setCharge(charge);
        atom->setRemoteness(remoteness);
        atom->setBranch(branch);
        atoms[serial] = atom;
    }
} else if (recordtype == "HETATM") {
    long int serial = atoi(line.substr(6, 5).c_str());
    std::string name = line.substr(12, 4);
    char remoteness = line[14];
    char branch = line[15];
    char altLoc = line[16];
    std::string resSymbol = strip(line.substr(17, 3));
    char chainId = line[21];
    std::string seqId = strip(line.substr(22, 4));
//                         char iCode = line[26];
    float x = atof(line.substr(30, 8).c_str());
    float y = atof(line.substr(38, 8).c_str());
    float z = atof(line.substr(46, 8).c_str());
//                         float occupancy = atof(line.substr(54, 6).c_str());
    float tempFactor = atof(line.substr(60, 6).c_str());
    std::string segID = strip(line.substr(72, 4));
    std::string element = "";
    if (line[77] >= 'A' && line[77] <= 'Z')
        element = strip(line.substr(76, 2));
    int charge = 0;
    char chargeSign = line[79];
    if (chargeSign == '+' || chargeSign == '-') {
        charge = line[78] - '0';
        if (chargeSign == '-') charge *= -1;
    }

    molecule = model->add(MODEL::Name::heterogen);
    std::string molname = "unknown";
    std::list< Heterogen >::iterator het = hetInfo.begin();
    std::list< Heterogen >::iterator end = hetInfo.end();
    for (; het != end; ++het) {
        if ((*het).hetID == resSymbol) {
            molname = (*het).name;
            break;
        }
    }
    molecule->setAttribute<std::string>("name", molname);
    molecule->setAttribute<std::string>("hetID", resSymbol);
    chain_het = molecule;

// Initialise new residues
    if (residue == 0 || seqId != residue->getAttribute<std::string>("seqId")) {
        if (MODEL::AminoAcid::getAminoAcid(resSymbol) != 0) {
            residue = chain_het->add(MODEL::Name::aminoacid);
            residue->setNodeData<MODEL::AminoAcid*>(MODEL::AminoAcid::getAminoAcid(resSymbol));
            residue->setAttribute<std::string>("seqId", seqId);
            backbone = residue->add(MODEL::Name::backbone);
            sidechain = residue->add(MODEL::Name::sidechain);
        } else if (MODEL::Nucleotide::getNucleotide(resSymbol) != 0) {
            residue = chain_het->add(MODEL::Name::nucleotide);
            residue->setNodeData<MODEL::Nucleotide*>(MODEL::Nucleotide::getNucleotide(resSymbol));
            residue->setAttribute<std::string>("seqId", seqId);
            backbone = residue->add(MODEL::Name::backbone);
            sidechain = residue->add(MODEL::Name::sidechain);
        } else {
            residue = chain_het->add(MODEL::Name::heterogen);
            residue->setAttribute<std::string>("seqId", seqId);
            residue->setAttribute<std::string>("hetID", resSymbol);
            std::list< Heterogen >::iterator het_iter = hetInfo.begin();
            std::list< Heterogen >::iterator het_end = hetInfo.end();
            for (; het_iter != het_end; ++het_iter)
                if ((*het_iter).hetID == resSymbol && (*het_iter).chainId == chainId && (*het_iter).seqId == seqId) {
                    (*het_iter).name = name;
                    residue->setAttribute<std::string>("name", (*het_iter).name);
                }
            backbone = 0;
            sidechain = 0;
        }
    }

// Is this the primary record or an alternative location?
    if (altLoc == ' ' || altLoc == 'A') {
// Create atom
        std::string symbol = "";
        if (name[0] == ' ' || (name[0] >= '1' && name[0] <='9')) symbol = name.substr(1,1);
        else symbol = name.substr(0,2);
        atom = new MODEL::Atom(symbol);
        atom->setSerial(serial);
        atom->setPosition(x, y, z);
        atom->setTemperature(tempFactor);
        atom->setCharge(charge);
        atom->setRemoteness(remoteness);
        atom->setBranch(branch);
        atoms[serial] = atom;
        residue->add(atom);
    }
}
}

// Apply secondary Structure
std::list< Turn >::iterator turn_iter = turnInfo.begin();
std::list< Turn >::iterator turn_end = turnInfo.end();
for (; turn_iter != turn_end; ++turn_iter) {
    char chainId = turn_iter->chainId;
    std::string initSeqId = turn_iter->initSeqId;
    std::string endSeqId = turn_iter->endSeqId;
    MODEL::MolecularContainer * initResidue = 0;
    MODEL::MolecularContainer * endResidue = 0;
    if (residues.find(std::pair< char, std::string >(chainId, initSeqId)) != residues.end() && residues.find(std::pair< char, std::string >(chainId, endSeqId)) != residues.end()) {
        initResidue = residues[std::pair< char, std::string >(chainId, initSeqId)];
        endResidue = residues[std::pair< char, std::string >(chainId, endSeqId)];
// Calculate Length
        int length = 0;
        MODEL::MolecularContainer::parent_iterator parent_iter = initResidue->parentsBegin(&MODEL::isChain);
        MODEL::MolecularContainer::parent_iterator parent_end = initResidue->parentsEnd();
        if (parent_iter != parent_end) {
            MODEL::MolecularContainer::child_iterator sibling_iter = (*parent_iter)->childrenBegin(&MODEL::isAminoAcid);
            MODEL::MolecularContainer::child_iterator sibling_end = (*parent_iter)->childrenEnd();
            bool first = false;
            for (; sibling_iter != sibling_end; ++sibling_iter) {
                if (*sibling_iter == initResidue) {
                    first = true;
                }
                if (first) {
                    if (*sibling_iter == endResidue) {
                        break;
                    } else {
                        ++length;
                    }
                }
            }
            if (length) {
                MODEL::MolecularContainer * annotation = secondaryStructure->add("annotation");
                annotation->setAttribute<std::string>("name", "Turns");
                annotation->setAttribute<std::string>("class", "extent");
                annotation->setAttribute<int>("width", length);
                annotation->add(initResidue);
            }
        }
    }
}
std::list< Sheet >::iterator sheet_iter = sheetInfo.begin();
std::list< Sheet >::iterator sheet_end = sheetInfo.end();
for (; sheet_iter != sheet_end; ++sheet_iter) {
    char chainId = sheet_iter->chainId;
    std::string initSeqId = sheet_iter->initSeqId;
    std::string endSeqId = sheet_iter->endSeqId;
    MODEL::MolecularContainer * initResidue = 0;
    MODEL::MolecularContainer * endResidue = 0;
    if (residues.find(std::pair< char, std::string >(chainId, initSeqId)) != residues.end() && residues.find(std::pair< char, std::string >(chainId, endSeqId)) != residues.end()) {
        initResidue = residues[std::pair< char, std::string >(chainId, initSeqId)];
        endResidue = residues[std::pair< char, std::string >(chainId, endSeqId)];
// Calculate Length
        int length = 0;
        MODEL::MolecularContainer::parent_iterator parent_iter = initResidue->parentsBegin(&MODEL::isChain);
        MODEL::MolecularContainer::parent_iterator parent_end = initResidue->parentsEnd();
        if (parent_iter != parent_end) {
            MODEL::MolecularContainer::child_iterator sibling_iter = (*parent_iter)->childrenBegin(&MODEL::isAminoAcid);
            MODEL::MolecularContainer::child_iterator sibling_end = (*parent_iter)->childrenEnd();
            bool first = false;
            for (; sibling_iter != sibling_end; ++sibling_iter) {
                if (*sibling_iter == initResidue) {
                    first = true;
                }
                if (first) {
                    if (*sibling_iter == endResidue) {
                        break;
                    } else {
                        ++length;
                    }
                }
            }
            if (length) {
                MODEL::MolecularContainer * annotation = secondaryStructure->add("annotation");
                annotation->setAttribute<std::string>("name", "Sheets");
                annotation->setAttribute<std::string>("class", "extent");
                annotation->setAttribute<int>("width", length);
                annotation->add(initResidue);
            }
        }
    }
}
std::list< Helix >::iterator helix_iter = helixInfo.begin();
std::list< Helix >::iterator helix_end = helixInfo.end();
for (; helix_iter != helix_end; ++helix_iter) {
    char chainId = helix_iter->chainId;
    std::string initSeqId = helix_iter->initSeqId;
    std::string endSeqId = helix_iter->endSeqId;
    MODEL::MolecularContainer * initResidue = 0;
    MODEL::MolecularContainer * endResidue = 0;
    if (residues.find(std::pair< char, std::string >(chainId, initSeqId)) != residues.end() && residues.find(std::pair< char, std::string >(chainId, endSeqId)) != residues.end()) {
        initResidue = residues[std::pair< char, std::string >(chainId, initSeqId)];
        endResidue = residues[std::pair< char, std::string >(chainId, endSeqId)];
// Calculate Length
        int length = 0;
        MODEL::MolecularContainer::parent_iterator parent_iter = initResidue->parentsBegin(&MODEL::isChain);
        MODEL::MolecularContainer::parent_iterator parent_end = initResidue->parentsEnd();
        if (parent_iter != parent_end) {
            MODEL::MolecularContainer::child_iterator sibling_iter = (*parent_iter)->childrenBegin(&MODEL::isAminoAcid);
            MODEL::MolecularContainer::child_iterator sibling_end = (*parent_iter)->childrenEnd();
            bool first = false;
            for (; sibling_iter != sibling_end; ++sibling_iter) {
                if (*sibling_iter == initResidue) {
                    first = true;
                }
                if (first) {
                    if (*sibling_iter == endResidue) {
                        break;
                    } else {
                        ++length;
                    }
                }
            }
            if (length) {
                MODEL::MolecularContainer * annotation = secondaryStructure->add("annotation");
                annotation->setAttribute<std::string>("name", "Helices");
                annotation->setAttribute<std::string>("class", "extent");
                annotation->setAttribute<int>("width", length);
                annotation->add(initResidue);
            }
        }
    }
}

// Ensure matrices include identity
if (matrices.empty())
{
    matrices.push_back(gtl::matrix_4d::identity());
}
else
{
    bool present = false;
    std::vector< gtl::matrix_4d >::iterator iter = matrices.begin();
    std::vector< gtl::matrix_4d >::iterator end = matrices.end();
    for (; !present && iter != end; ++iter)
    {
        present = (*iter == gtl::matrix_4d::identity());
    }
    if (!present)
    {
        matrices.push_back(gtl::matrix_4d::identity());
    }
}

// Set model's Biological transformation matrices
model->setAttribute< std::vector< gtl::matrix_4d > >("BIOMT", matrices);

if (utopia_name.find_first_not_of(" \t\n\r") == std::string::npos)
{
    utopia_name = "Unknown Model";
}
if (utopia_description.find_first_not_of(" \t\n\r") == std::string::npos)
{
    utopia_description = "no description";
}
model->setAttribute<std::string>("utopia name", utopia_name);
model->setAttribute<std::string>("utopia description", utopia_description);
} catch (int) {
      Logger::log(Logger::U_ERROR, "Error parsing stream");
      delete source;
      return 0;
  }

if (model->childCount() == 0) {
    delete source;
    source = 0;
}

return source;
}
*/

 // Parse!
Node * PDBParser::parse(Parser::Context& ctx, QIODevice& stream_) const
{
    // Ensure valid stream
    if (!stream_.isOpen() || !stream_.isReadable())
    {
        ctx.setErrorCode(StreamError);
        ctx.setMessage("Invalid Stream");
    }

    // Ensure not empty stream
    if (stream_.atEnd())
    {
        ctx.setErrorCode(StreamEmpty);
        ctx.setMessage("Empty Stream");
    }

    QTextStream stream(&stream_);

    // Convenience...
    Node * c_ExtentAnnotation = UtopiaSystem.term("ExtentAnnotation");
    //Node * p_annotates = UtopiaSystem.term("annotates");

    //Node * c_Alignment = UtopiaDomain.term("Alignment");
    //Node * c_Sequence = UtopiaDomain.term("Sequence");

    Node * authority = createAuthority();
    Node * model = authority->create("complex");
    authority->relations(Utopia::UtopiaSystem.hasPart).append(model);
    Node * secondaryStructure = authority->create(c_ExtentAnnotation);
    secondaryStructure->attributes.set("name", "Secondary Structure");
    secondaryStructure->attributes.set("class", "group");

    try {
        // Local variables required for importing the structure and data
        Node * molecule = 0;
        QMap< size_t, Node * > moleculeIdToMolecule;
        Node * chain = 0;
        Node * chain_het = 0;
        Node * residue = 0;
        Node * backbone = 0;
        Node * sidechain = 0;
        Node * atom = 0;
        QMap< int, Node * > atoms;
        QMap< QString, Node * > residues;
        QSet< Node * > bonds;
        QSet< DummySSBOND* > _dummySSBONDs;
        QSet< DummyHYDBND* > _dummyHYDBNDs;
        QSet< DummyLINK* > _dummyLINKs;
        QSet< DummyCISPEP* > _dummyCISPEPs;
        QSet< DummySLTBRG* > _dummySLTBRGs;
        QVector< QMap< QString, QString > > compndInfo;
        QString lastKey = "MOLECULE";
        QList< Heterogen > hetInfo;
        QString utopia_name = "Unknown Model";
        QString utopia_description = "No Header Information Found";
        QString classification = "";
        QString title = "";
        QString date = "";
        QString pdbcode = "";
        QList< Turn > turnInfo;
        QList< Helix > helixInfo;
        QList< Sheet > sheetInfo;
        bool firstModel = true;
        QVector< gtl::matrix_4d > matrices;
        size_t line_no = 0;

        while (!stream.atEnd() && firstModel) {
            QString line = stream.readLine();
            ++line_no;
            // Ignore empty lines
            if (line.isEmpty())
            {
                continue;
            }

            if (line.length() < 80)
            {
                line += QString(80 - line.length(), ' ');
            }
//qDebug() << line;


            QString recordtype = line.mid(0, 6);

            if (recordtype == "COMPND") {
                QString compnd = line.mid(10, 60).trimmed();
                QString key = "";
                QString value = "";

                // Parse compound record
                int colonIndex = compnd.indexOf(':');
                if (colonIndex == -1) {
                    key = lastKey;
                    value = compnd;
                } else {
                    key = compnd.mid(0, colonIndex).trimmed();
                    value = compnd.mid(colonIndex + 1).trimmed();
                }
                if (compndInfo.size() == 0 || key == "MOL_ID") {
                    compndInfo.push_back(QMap< QString, QString >());
                    if (key != "MOL_ID")
                        compndInfo.back()["MOL_ID"] = "1";
                }
                compndInfo.back()[key] += (compndInfo.back()[key] == "" ? "" : " ") + value;
                lastKey = key;
            } else if (recordtype == "HET   ") {
                Heterogen * het = 0;
                hetInfo.push_back(Heterogen(line.mid(7, 3).trimmed()));
                het = &hetInfo.back();
                het->chainId = line[12];
                het->seqId = line.mid(13, 4).trimmed();
                het->iCode = line[17];
                het->text = line.mid(30, 40).trimmed();
            } else if (recordtype == "TER   ") {
//                      chain = 0;
            } else if (recordtype == "ENDMDL") {
                firstModel = false;
            } else if (recordtype == "HETNAM") {
                QString hetID = line.mid(11, 3).trimmed();
                QString name = line.mid(15, 55).trimmed();
                QList< Heterogen >::iterator het = hetInfo.begin();
                QList< Heterogen >::iterator end = hetInfo.end();
                for (; het != end; ++het)
                    if ((*het).hetID == hetID)
                        (*het).name = name;
            } else if (recordtype == "TURN  ") {
                QChar chainId = line[19];
                QString initSeqId = line.mid(20, 4).trimmed();
                QString endSeqId = line.mid(31, 4).trimmed();
                turnInfo.push_back(Turn(chainId, initSeqId, endSeqId));
            } else if (recordtype == "HELIX ") {
                QChar chainId = line[19];
                QString initSeqId = line.mid(21, 4).trimmed();
                QString endSeqId = line.mid(33, 4).trimmed();
                helixInfo.push_back(Helix(chainId, initSeqId, endSeqId));
            } else if (recordtype == "SHEET ") {
                QChar chainId = line[21];
                QString initSeqId = line.mid(22, 4).trimmed();
                QString endSeqId = line.mid(33, 4).trimmed();
                sheetInfo.push_back(Sheet(chainId, initSeqId, endSeqId));
            } else if (recordtype == "HEADER") {
                classification = line.mid(9, 41).trimmed();
                date = line.mid(50, 9).trimmed();
                pdbcode = line.mid(62, 4).trimmed();
                if (pdbcode.size() != 4)
                {
                    pdbcode = "????";
                }
                if (classification == "")
                {
                    classification = "*Unclassified entry*";
                }
                model->attributes.set("classification", classification);
                model->attributes.set("date", date);
                model->attributes.set("pdbcode", pdbcode);
                utopia_name = pdbcode;
                utopia_description = classification;
            } else if (recordtype == "TITLE ") {
                if (title != "")
                    title += " ";
                title += line.mid(10, 60).trimmed();
                if (title.trimmed() == "")
                {
                    title = "*Unnamed entry*";
                }
                model->attributes.set("title", title);
                utopia_description = title;
            } else if (recordtype == "REMARK") {
                int number = atoi(line.mid(7, 3).toStdString().c_str());
                switch (number)
                {
                case 350:
                    if (line.mid(13, 5) == "BIOMT")
                    {
                        size_t row = atoi(line.mid(18, 1).toStdString().c_str());
                        int id = atoi(line.mid(20, 3).toStdString().c_str());
                        double r1 = atof(line.mid(24, 9).toStdString().c_str());
                        double r2 = atof(line.mid(34, 9).toStdString().c_str());
                        double r3 = atof(line.mid(44, 9).toStdString().c_str());
                        double t = atof(line.mid(54, 14).toStdString().c_str());
                        if (id > matrices.size())
                        {
                            matrices.push_back(gtl::matrix_4d::identity());
                        }
                        matrices[id - 1].row(row - 1, r1, r2, r3, t);
                    }
                    break;
                }
            } else if (recordtype == "ATOM  ") {
                long int serial = atoi(line.mid(6, 5).toStdString().c_str());
                QString name = line.mid(12, 4);
                QChar remoteness = line[14];
                QChar branch = line[15];
                QChar altLoc = line[16];
                QString resSymbol = line.mid(17, 3).trimmed();
                QChar chainId = line[21];
                QString seqId = line.mid(22, 4).trimmed();
//                         QChar iCode = line[26];
                float x = atof(line.mid(30, 8).toStdString().c_str());
                float y = atof(line.mid(38, 8).toStdString().c_str());
                float z = atof(line.mid(46, 8).toStdString().c_str());
//                         float occupancy = atof(line.mid(54, 6).toStdString().c_str());
                //float tempFactor = atof(line.mid(60, 6).toStdString().c_str());
                QString segID = line.mid(72, 4).trimmed();
                QString element = "";
                if (line[77] >= 'A' && line[77] <= 'Z')
                    element = line.mid(76, 2).trimmed();
                int charge = 0;
                QChar chargeSign = line[79];
                if (chargeSign == '+' || chargeSign == '-') {
                    charge = line[78].toLatin1() - '0';
                    if (chargeSign == '-') charge *= -1;
                }

                // Is this part of a protein, nucleic acid or heterogen?
                QString moleculeClass = "";
                if (Nucleotide::get(resSymbol) != 0) moleculeClass = "nucleicacid";
                else if (AminoAcid::get(resSymbol) != 0) moleculeClass = "protein";
                else moleculeClass = "heterogen";

                // Initialise new chains
                if (chain == 0 || !chain->attributes.exists("chainId") || chainId != chain->attributes.get("chainId").toChar()) {
                    size_t moleculeId = 1;
                    int compndIndex = 0;
                    if (compndInfo.size() > 1) {
                        molecule = 0;

                        // If this chain is on an existing molecule, then use that molecule
                        QVector< QMap< QString, QString > >::iterator iter = compndInfo.begin();
                        QVector< QMap< QString, QString > >::iterator end = compndInfo.end();
                        for (; iter != end; ++iter, ++compndIndex) {
                            if ((*iter)["CHAIN"].indexOf(chainId) != -1) {
                                moleculeId = atoi((*iter)["MOL_ID"].toStdString().c_str());
                                if (moleculeIdToMolecule.find(moleculeId) != moleculeIdToMolecule.end())
                                    molecule = moleculeIdToMolecule[moleculeId];
                                break;
                            }
                        }
                    }

                    // else create a new molecule
                    if (molecule == 0) {
                        molecule = model->create(moleculeClass);
                        model->relations(Utopia::UtopiaSystem.hasPart).append(molecule);
                        moleculeIdToMolecule[moleculeId] = molecule;
                        QString name = "";
                        if (compndIndex < compndInfo.size()) {
                            name = compndInfo[compndIndex]["MOLECULE"];
                        }
                        molecule->attributes.set("name", name != "" ? name : "unknown");
                    }

                    chain = molecule->create("chain");
                    molecule->relations(Utopia::UtopiaSystem.hasPart).append(chain);
                    chain->attributes.set("chainId", chainId);
                }

                // Initialise new residues
                if (residue == 0 || seqId != residue->attributes.get("seqId").toString()) {
                    if (moleculeClass == "protein") {
                        residue = chain->create();
                        chain->relations(Utopia::UtopiaSystem.hasPart).append(residue);
                        residues[QString(chainId) + "_" + seqId] = residue;
                        if (AminoAcid::get(resSymbol) == 0) {
                            QString log = "Can't find amino acid '";
                            log += resSymbol + "', inserting 'X' instead";
                            qWarning() << log;
                            resSymbol = "X";
                        }
                        residue->setType(AminoAcid::get(resSymbol, true));
                        residue->attributes.set("seqId", seqId);
                    } else if (moleculeClass == "nucleicacid") {
                        residue = chain->create();
                        chain->relations(Utopia::UtopiaSystem.hasPart).append(residue);
                        if (Nucleotide::get(resSymbol) == 0) {
                            QString log = "Can't find nucleotide ,";
                            log += resSymbol + "', inserting 'X' instead";
                            qWarning() << log;
                            resSymbol = "X";
                        }
                        residue->setType(Nucleotide::get(resSymbol, true));
                        residue->attributes.set("seqId", seqId);
                    }
                    if (moleculeClass != "heterogen") {
                        backbone = residue->create("backbone");
                        residue->relations(Utopia::UtopiaSystem.hasPart).append(backbone);
                        sidechain = residue->create("sidechain");
                        residue->relations(Utopia::UtopiaSystem.hasPart).append(sidechain);
                    }
                }

                // Is this the primary record or an alternative location?
                if (altLoc == ' ' || altLoc == 'A') {
                    // Create atom
                    QString symbol = "";
                    if (name[0] == ' ' || (name[0] >= '1' && name[0] <='9')) symbol = name.mid(1,1);
                    else symbol = name.mid(0,2);
                    if (moleculeClass == "protein") {
                        if (remoteness == ' ' || remoteness == 'A')
                        {
                            atom = backbone->create(Element::get(symbol, true));
                            backbone->relations(Utopia::UtopiaSystem.hasPart).append(atom);
                        }
                        else
                        {
                            atom = sidechain->create(Element::get(symbol, true));
                            sidechain->relations(Utopia::UtopiaSystem.hasPart).append(atom);
                        }
                    } else if (moleculeClass == "nucleicacid") {
                        if (branch == '*' || branch == 'P' || remoteness == ' ')
                        {
                            atom = backbone->create(Element::get(symbol, true));
                            backbone->relations(Utopia::UtopiaSystem.hasPart).append(atom);
                        }
                        else
                        {
                            atom = sidechain->create(Element::get(symbol, true));
                            sidechain->relations(Utopia::UtopiaSystem.hasPart).append(atom);
                        }
                    } else {
                        atom = chain->create(Element::get(symbol, true));
                        chain->relations(Utopia::UtopiaSystem.hasPart).append(atom);
                    }
                    atom->attributes.set("x", x);
                    atom->attributes.set("y", y);
                    atom->attributes.set("z", z);
                    atom->attributes.set("remoteness", remoteness);
/*
  atom->setSerial(serial);
  atom->setPosition(x, y, z);
  atom->setTemperature(tempFactor);
  atom->setCharge(charge);
  atom->setBranch(branch);
*/
                    atoms[serial] = atom;
                }
            } else if (recordtype == "HETATM") {
                long int serial = atoi(line.mid(6, 5).toStdString().c_str());
                QString name = line.mid(12, 4);
                QChar remoteness = line[14];
                //QChar branch = line[15];
                QChar altLoc = line[16];
                QString resSymbol = line.mid(17, 3).trimmed();
                QChar chainId = line[21];
                QString seqId = line.mid(22, 4).trimmed();
//                         QChar iCode = line[26];
                float x = atof(line.mid(30, 8).toStdString().c_str());
                float y = atof(line.mid(38, 8).toStdString().c_str());
                float z = atof(line.mid(46, 8).toStdString().c_str());
//                         float occupancy = atof(line.mid(54, 6).toStdString().c_str());
                //float tempFactor = atof(line.mid(60, 6).toStdString().c_str());
                QString segID = line.mid(72, 4).trimmed();
                QString element = "";
                if (line[77] >= 'A' && line[77] <= 'Z')
                    element = line.mid(76, 2).trimmed();
                int charge = 0;
                QChar chargeSign = line[79];
                if (chargeSign == '+' || chargeSign == '-') {
                    charge = line[78].toLatin1() - '0';
                    if (chargeSign == '-') charge *= -1;
                }

                molecule = model->create("heterogen");
                model->relations(Utopia::UtopiaSystem.hasPart).append(molecule);
                QString molname = "unknown";
                QList< Heterogen >::iterator het = hetInfo.begin();
                QList< Heterogen >::iterator end = hetInfo.end();
                for (; het != end; ++het) {
                    if ((*het).hetID == resSymbol) {
                        molname = (*het).name;
                        break;
                    }
                }
                molecule->attributes.set("name", molname);
                molecule->attributes.set("hetID", resSymbol);
                chain_het = molecule;

                // Initialise new residues
                if (residue == 0 || seqId != residue->attributes.get("seqId").toString()) {
                    if (AminoAcid::get(resSymbol) != 0) {
                        residue = chain_het->create(AminoAcid::get(resSymbol));
                        chain_het->relations(Utopia::UtopiaSystem.hasPart).append(residue);
                        residue->attributes.set("seqId", seqId);
                        backbone = residue->create("backbone");
                        sidechain = residue->create("sidechain");
                    } else if (Nucleotide::get(resSymbol) != 0) {
                        residue = chain_het->create(Nucleotide::get(resSymbol));
                        chain_het->relations(Utopia::UtopiaSystem.hasPart).append(residue);
                        residue->attributes.set("seqId", seqId);
                        backbone = residue->create("backbone");
                        sidechain = residue->create("sidechain");
                    } else {
                        residue = chain_het->create("heterogen");
                        chain_het->relations(Utopia::UtopiaSystem.hasPart).append(residue);
                        residue->attributes.set("seqId", seqId);
                        residue->attributes.set("hetID", resSymbol);
                        QList< Heterogen >::iterator het_iter = hetInfo.begin();
                        QList< Heterogen >::iterator het_end = hetInfo.end();
                        for (; het_iter != het_end; ++het_iter)
                            if ((*het_iter).hetID == resSymbol && (*het_iter).chainId == chainId && (*het_iter).seqId == seqId) {
                                (*het_iter).name = name;
                                residue->attributes.set("name", (*het_iter).name);
                            }
                        backbone = 0;
                        sidechain = 0;
                    }
                }

                // Is this the primary record or an alternative location?
                if (altLoc == ' ' || altLoc == 'A') {
                    // Create atom
                    QString symbol = "";
                    if (name[0] == ' ' || (name[0] >= '1' && name[0] <='9')) symbol = name.mid(1,1);
                    else symbol = name.mid(0,2);
                    atom = residue->create(Element::get(symbol, true));
                    residue->relations(Utopia::UtopiaSystem.hasPart).append(atom);
//                    qDebug() << "+++++" << symbol << Element::get(symbol, true) << atom;
                    atom->attributes.set("x", x);
                    atom->attributes.set("y", y);
                    atom->attributes.set("z", z);
                    atom->attributes.set("remoteness", remoteness);
/*
  atom->setSerial(serial);
  atom->setPosition(x, y, z);
  atom->setTemperature(tempFactor);
  atom->setCharge(charge);
  atom->setBranch(branch);
*/
                    atoms[serial] = atom;
//                                              residue->add(atom);
                }
            }
        }
/*
// Apply secondary Structure
QList< Turn >::iterator turn_iter = turnInfo.begin();
QList< Turn >::iterator turn_end = turnInfo.end();
for (; turn_iter != turn_end; ++turn_iter) {
QChar chainId = turn_iter->chainId;
QString initSeqId = turn_iter->initSeqId;
QString endSeqId = turn_iter->endSeqId;
Node * initResidue = 0;
Node * endResidue = 0;
if (residues.find(QString(chainId) + "_" + initSeqId) != residues.end() && residues.find(QString(chainId) + "_" + endSeqId) != residues.end()) {
initResidue = residues[QString(chainId) + "_" + initSeqId];
endResidue = residues[QString(chainId) + "_" + endSeqId];
// Calculate Length
int length = 0;
Node::parent_iterator parent_iter = initResidue->parentsBegin(&MODEL::isChain);
Node::parent_iterator parent_end = initResidue->parentsEnd();
if (parent_iter != parent_end) {
Node::child_iterator sibling_iter = (*parent_iter)->childrenBegin(&MODEL::isAminoAcid);
Node::child_iterator sibling_end = (*parent_iter)->childrenEnd();
bool first = false;
for (; sibling_iter != sibling_end; ++sibling_iter) {
if (*sibling_iter == initResidue) {
first = true;
}
if (first) {
if (*sibling_iter == endResidue) {
break;
} else {
++length;
}
}
}
if (length) {
Node * annotation = secondaryStructure->add("annotation");
annotation->attributes.set("name", "Turns");
annotation->attributes.set("class", "extent");
annotation->attributes.set("width", length);
annotation->add(initResidue);
}
}
}
}
QList< Sheet >::iterator sheet_iter = sheetInfo.begin();
QList< Sheet >::iterator sheet_end = sheetInfo.end();
for (; sheet_iter != sheet_end; ++sheet_iter) {
QChar chainId = sheet_iter->chainId;
QString initSeqId = sheet_iter->initSeqId;
QString endSeqId = sheet_iter->endSeqId;
Node * initResidue = 0;
Node * endResidue = 0;
if (residues.find(QString(chainId) + "_" + initSeqId) != residues.end() && residues.find(QString(chainId) + "_" + endSeqId) != residues.end()) {
initResidue = residues[QString(chainId) + "_" + initSeqId];
endResidue = residues[QString(chainId) + "_" + endSeqId];
// Calculate Length
int length = 0;
Node::parent_iterator parent_iter = initResidue->parentsBegin(&MODEL::isChain);
Node::parent_iterator parent_end = initResidue->parentsEnd();
if (parent_iter != parent_end) {
Node::child_iterator sibling_iter = (*parent_iter)->childrenBegin(&MODEL::isAminoAcid);
Node::child_iterator sibling_end = (*parent_iter)->childrenEnd();
bool first = false;
for (; sibling_iter != sibling_end; ++sibling_iter) {
if (*sibling_iter == initResidue) {
first = true;
}
if (first) {
if (*sibling_iter == endResidue) {
break;
} else {
++length;
}
}
}
if (length) {
Node * annotation = secondaryStructure->add("annotation");
annotation->attributes.set("name", "Sheets");
annotation->attributes.set("class", "extent");
annotation->attributes.set("width", length);
annotation->add(initResidue);
}
}
}
}
QList< Helix >::iterator helix_iter = helixInfo.begin();
QList< Helix >::iterator helix_end = helixInfo.end();
for (; helix_iter != helix_end; ++helix_iter) {
QChar chainId = helix_iter->chainId;
QString initSeqId = helix_iter->initSeqId;
QString endSeqId = helix_iter->endSeqId;
Node * initResidue = 0;
Node * endResidue = 0;
if (residues.find(QString(chainId) + "_" + initSeqId) != residues.end() && residues.find(QString(chainId) + "_" + endSeqId) != residues.end()) {
initResidue = residues[QString(chainId) + "_" + initSeqId];
endResidue = residues[QString(chainId) + "_" + endSeqId];
// Calculate Length
int length = 0;
Node::parent_iterator parent_iter = initResidue->parentsBegin(&MODEL::isChain);
Node::parent_iterator parent_end = initResidue->parentsEnd();
if (parent_iter != parent_end) {
Node::child_iterator sibling_iter = (*parent_iter)->childrenBegin(&MODEL::isAminoAcid);
Node::child_iterator sibling_end = (*parent_iter)->childrenEnd();
bool first = false;
for (; sibling_iter != sibling_end; ++sibling_iter) {
if (*sibling_iter == initResidue) {
first = true;
}
if (first) {
if (*sibling_iter == endResidue) {
break;
} else {
++length;
}
}
}
if (length) {
Node * annotation = secondaryStructure->add("annotation");
annotation->attributes.set("name", "Helices");
annotation->attributes.set("class", "extent");
annotation->attributes.set("width", length);
annotation->add(initResidue);
}
}
}
}
*/
        // Ensure matrices include identity
        if (matrices.empty())
        {
            matrices.push_back(gtl::matrix_4d::identity());
        }
        else
        {
            bool present = false;
            QVector< gtl::matrix_4d >::iterator iter = matrices.begin();
            QVector< gtl::matrix_4d >::iterator end = matrices.end();
            for (; !present && iter != end; ++iter)
            {
                present = (*iter == gtl::matrix_4d::identity());
            }
            if (!present)
            {
                matrices.push_back(gtl::matrix_4d::identity());
            }
        }

        // Set model's Biological transformation matrices
        model->attributes.set("BIOMT", qVariantFromValue((void *) new QVector< gtl::matrix_4d >(matrices)));

        if (utopia_name.trimmed() == "")
        {
            utopia_name = "Unknown Model";
        }
        if (utopia_description.trimmed() == "")
        {
            utopia_description = "no description";
        }
        model->attributes.set("utopia name", utopia_name);
        model->attributes.set("utopia description", utopia_description);

        if (model->attributes.get("pdbcode").toString().isEmpty() &&
            model->attributes.get("classification").toString().isEmpty() &&
            model->attributes.get("title").toString().isEmpty())
        {
            delete authority;
            authority = 0;
        }
    } catch (...) {
        delete authority;
        authority = 0;
    }

    if (authority)
    {
        if (authority->relations.empty()) {
            delete authority;
            authority = 0;
        }
    }

    if (authority == 0)
    {
        ctx.setErrorCode(StreamError);
        ctx.setMessage("Error parsing stream");
    }

    return authority;
}

QString PDBParser::description() const
{
    return "PDB";
}

QSet< FileFormat * > PDBParser::formats() const
{
    QSet< FileFormat * > formats;
    FileFormat * pdb = FileFormat::create("PDB", StructureFormat);
    *pdb << "pdb";
    formats << pdb;
    return formats;
}

} // namespace Utopia
