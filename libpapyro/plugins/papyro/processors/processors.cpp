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

#include "citations.h"
#include "commenting.h"
#include "copying.h"
#include "demo_logo.h"
#include "exploring.h"
#include "highlighting.h"
#include "hyperlinking.h"
#include "saveimage.h"
#include "wiley.h"

#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>


// dlsym handles

extern "C" const char * utopia_apiVersion()
{
    return UTOPIA_EXTENSION_LIBRARY_VERSION;
}

extern "C" const char * utopia_description()
{
    return "Standard processors";
}

extern "C" void utopia_registerExtensions()
{
    UTOPIA_REGISTER_EXTENSION(Papyro::CopyingProcessorFactory);

    UTOPIA_REGISTER_EXTENSION(Papyro::ExploringProcessorFactory);

    UTOPIA_REGISTER_EXTENSION(Papyro::ImagingProcessorFactory);

    UTOPIA_REGISTER_EXTENSION(WileyActivator);

    UTOPIA_REGISTER_EXTENSION(CitationActivator);

    UTOPIA_REGISTER_EXTENSION(CommentProcessor);
    UTOPIA_REGISTER_EXTENSION(CommentProcessorFactory);
    UTOPIA_REGISTER_EXTENSION(CommentRenderer);

    UTOPIA_REGISTER_EXTENSION(DemoLogoRenderer);

    UTOPIA_REGISTER_EXTENSION(HyperlinkRenderer);
    UTOPIA_REGISTER_EXTENSION(MailToFactory);
    UTOPIA_REGISTER_TYPED_EXTENSION(Papyro::AnnotationProcessor, HyperlinkFactory);
#if defined(BUILD_PERSISTENCE) || defined(BUILD_DEBUG)
    UTOPIA_REGISTER_TYPED_EXTENSION(Papyro::SelectionProcessorFactory, HyperlinkFactory);
    UTOPIA_REGISTER_EXTENSION(HighlightFactory);
#endif
    UTOPIA_REGISTER_EXTENSION(HighlightRenderer);
}
