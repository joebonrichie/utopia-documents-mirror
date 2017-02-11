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

#ifndef PAPYRO_CAPABILITY_H
#define PAPYRO_CAPABILITY_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <boost/scoped_ptr.hpp>
#  include <boost/shared_ptr.hpp>
#  include <boost/weak_ptr.hpp>
#  include <spine/Capability.h>
#endif

#include <QPixmap>
#include <QString>
#include <QUrl>

namespace Spine
{
    class Annotation;
}

namespace Papyro
{

    /*
     * Summary capability
     *
     * This capability allows the UI to provide a DIV element for inclusion in a web view.
     */
    class SummaryCapability : public Spine::Capability
    {
    public:
        virtual void generate(boost::shared_ptr< Spine::Annotation > annotation, QString key, QObject * listener, const char * method) = 0;
    };


    /*
     * Summary capability
     *
     * This capability allows the UI to provide a link to the user.
     */
    class LinkFinderCapability : public Spine::Capability
    {
    public:
        virtual void generate(boost::shared_ptr< Spine::Annotation > annotation, QObject * listener, const char * method) = 0;
    };


    /*
     * URL capability
     *
     * This capability allows the UI to provide a way of browsing to a given URL for further
     * information.
     */
    class UrlCapability : public Spine::Capability
    {
    public:
        UrlCapability(const QUrl & url,
                      const QString & title,
                      const QPixmap & pixmap);
        QUrl url;
        QString title;
        QPixmap pixmap;
    };


    /*
     * Web page capability
     *
     * This capability allows the UI to provide a way of browsing to a given web page URL.
     */
    class WebPageCapability : public UrlCapability
    {
    public:
        WebPageCapability(const QUrl & url,
                          const QString & title,
                          const QPixmap & pixmap,
                          const QString & target);
        QString target;
    };


    /*
     * Download capability
     *
     * This capability allows the UI to provide a way of downloading a file from a given URL.
     */
    class DownloadCapability : public UrlCapability
    {
    public:
        DownloadCapability(const QUrl & url,
                           const QString & title,
                           const QPixmap & pixmap,
                           const QString & mime,
                           const QString & name);
        QString mime;
        QString name;
    };


    /*
     * Attachment capability
     *
     * This capability allows the UI to provide a way of downloading a file from a given URL.
     */
    class AttachmentCapability : public DownloadCapability
    {
    public:
        AttachmentCapability(const QUrl & url,
                             const QString & title,
                             const QPixmap & pixmap,
                             const QString & mime,
                             const QString & name,
                             int revision,
                             const QUrl & edit);
        int revision;
        QUrl edit;
    };

}

#endif // PAPYRO_CAPABILITY_H
