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

#include <papyro/printer.h>
#include <papyro/printer_p.h>
#include <papyro/utils.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <QDebug>
#include <QMutexLocker>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QProgressDialog>

namespace Papyro
{

    PrinterThread::PrinterThread(QObject * parent, Spine::DocumentHandle document, QPrinter * printer)
        : QThread(parent), document(document), printer(printer), cancelled(false), mutex(QMutex::Recursive)
    {}

    void PrinterThread::cancel()
    {
        QMutexLocker guard(&mutex);
        cancelled = true;
    }

    void PrinterThread::run()
    {
        mutex.lock();

        if (!cancelled) {

            // Which pages to print
            int step = 1;
            int fromPage = printer->printRange() == QPrinter::PageRange ?
              printer->fromPage() : 1;
            int toPage = printer->printRange() == QPrinter::PageRange ?
              printer->toPage() : document->numberOfPages();
            int resolution = qMin(printer->resolution(),
                                  Printer::maxResolution);

            // The order to print them in
            if (printer->pageOrder() == QPrinter::LastPageFirst) {
                step = -1;
                qSwap(fromPage, toPage);
            }

            int count = 0;
            for (int page = fromPage; page <= toPage && !cancelled; page += step) {
                mutex.unlock();
                Spine::Image image(document->newCursor(page)->page()
                                   ->render(resolution, Printer::antialias));
                QImage pageImage(qImageFromSpineImage(&image));
                mutex.lock();

                emit imageGenerated(pageImage, (page == fromPage));
                emit progressChanged(++count);
            }

            if (cancelled) {
                printer->abort();
            }
        }

        mutex.unlock();
    }






    PrinterPrivate::PrinterPrivate(Printer * p)
        : QObject(p), p(p), mutex(QMutex::Recursive), painter(0), printer(0)
    {
        // Check environment variable for flags
#ifdef _WIN32
        char env_c_str[1024] = { 0 };
        int status = GetEnvironmentVariable("UTOPIA_PRINTER_FLAGS", env_c_str, sizeof(env_c_str));
        if (status == 0) { env_c_str[0] = 0; }
#else
        char * env_c_str = ::getenv("UTOPIA_PRINTER_FLAGS");
#endif

        // Defaults
        Printer::monochrome = false;
        Printer::maxResolution = 300;
        Printer::antialias = false;

        QStringList flags(QString::fromUtf8(env_c_str).split(' ', QString::SkipEmptyParts));

        foreach (QString flag, flags) {
            // Antialiasing
            if (flag.startsWith("antialias")) {
                Printer::antialias = (flag.size() == 9) ||
                                     (flag.mid(9) == "=yes");
            }
            // Greyscale
            if (flag.startsWith("monochrome")) {
                Printer::monochrome = (flag.size() == 10) ||
                                      (flag.mid(10) == "=yes");
            }
        }

        // Resolution
        foreach (QString flag, flags) {
            if (flag.startsWith("resolution=")) {
                Printer::maxResolution = flag.mid(11).toInt();
            }
        }

        //qDebug() << "PRINTING AA:" << Printer::antialias << "  MC:" << Printer::monochrome << "  RES:" << Printer::maxResolution;
    }

    PrinterPrivate::~PrinterPrivate()
    {}

    void PrinterPrivate::onFinished()
    {
        delete painter;
        painter = 0;
        delete printer;
        printer = 0;
    }

    void PrinterPrivate::onImageGenerated(QImage image, bool first)
    {
        if (!first) {
            printer->newPage();
        }

        QRect viewport(painter->viewport());
        QSize size(image.size());
        size.scale(viewport.size(), Qt::KeepAspectRatio);
        QPoint centring(qAbs(viewport.width() - size.width()) / 2.0,
                        qAbs(viewport.height() - size.height()) / 2.0);

        painter->setViewport(QRect(viewport.topLeft() + centring, size));
        painter->setWindow(image.rect());
        painter->drawImage(0, 0, image);
        painter->setViewport(viewport);
    }

    int Printer::maxResolution;
    bool Printer::monochrome;
    bool Printer::antialias;

    Printer::Printer(QObject * parent)
        : QObject(parent), d(new PrinterPrivate(this))
    {}

    Printer::~Printer()
    {}

    boost::shared_ptr< Printer > Printer::instance()
    {
        static boost::weak_ptr< Printer > singleton;
        boost::shared_ptr< Printer > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< Printer >(new Printer());
            singleton = shared;
        }
        return shared;
    }

    bool Printer::print(Spine::DocumentHandle document, QWidget * parent)
    {
        if (document) {
            d->mutex.lock();

            // Create sensible default printer
            d->printer = new QPrinter(QPrinter::HighResolution);
            d->printer->setFullPage(true);
            d->printer->setResolution(maxResolution);
            d->printer->setCreator("Utopia");
            if (parent && parent->isWindow()) {
                d->printer->setDocName(parent->windowTitle());
            }

            // Provide the print dialog to the user for customisation
            QPrintDialog printDialog(d->printer, parent);
            printDialog.setWindowTitle(tr("Print Document"));
            printDialog.setOptions(QAbstractPrintDialog::PrintPageRange);
            if (printDialog.exec() == QDialog::Accepted) {
                if (d->printer->isValid()) {
                    if (monochrome) {
                        d->printer->setColorMode(QPrinter::GrayScale);
                    }

                    // Open progress dialog
                    int from = 0;
                    int to = d->printer->printRange() == QPrinter::PageRange ? qAbs(1 + d->printer->toPage() - d->printer->fromPage()) : document->numberOfPages();
                    QProgressDialog progressDialog("Printing...", "Cancel", from, to, parent);

                    PrinterThread * thread = new PrinterThread(this, document, d->printer);

                    connect(thread, SIGNAL(imageGenerated(QImage,bool)), d, SLOT(onImageGenerated(QImage,bool)));
                    connect(thread, SIGNAL(finished()), d, SLOT(onFinished()));
                    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

                    connect(&progressDialog, SIGNAL(canceled()), thread, SLOT(cancel()));
                    connect(thread, SIGNAL(progressChanged(int)), &progressDialog, SLOT(setValue(int)));
                    connect(thread, SIGNAL(finished()), &progressDialog, SLOT(accept()));

                    d->painter = new QPainter(d->printer);

                    // Print document
                    thread->start();

                    d->mutex.unlock();

                    return progressDialog.exec();
                } else {

                }
            }

            d->mutex.unlock();
        }

        return false;
    }

}
