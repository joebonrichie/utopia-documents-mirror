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

#include "exploring.h"

#include <papyro/documentview.h>
#include <papyro/papyrotab.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>

#include <QtDebug>




namespace Papyro
{

    /////////////////////////////////////////////////////////////////////////////////////////
    /// ExploringProcessor

    int ExploringProcessor::category() const
    {
        return -10; // Try to be the first and most default section
    }

    void ExploringProcessor::processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos)
    {
        if (PapyroWindow * window = PapyroWindow::currentWindow()) {
            if (PapyroTab * tab = window->currentTab()) {
                tab->exploreSelection();
            }
        }
    }

    QString ExploringProcessor::title() const
    {
        return "Explore";
    }

    int ExploringProcessor::weight() const
    {
        return 0;
    }




    /////////////////////////////////////////////////////////////////////////////////////////
    /// ExploringProcessorFactory

    QList< boost::shared_ptr< SelectionProcessor > > ExploringProcessorFactory::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
    {
        QList< boost::shared_ptr< SelectionProcessor > > list;
        if (hasTextSelection(document, cursor)) {
            list << boost::shared_ptr< SelectionProcessor >(new ExploringProcessor);
        }
        return list;
    }

}  // namespace Papyro

