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

#include <utopia2/certificateerrordialog.h>
#include <utopia2/certificateerrordialog_p.h>

#include <QCheckBox>
#include <QDateTime>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWebElement>
#include <QWebFrame>
#include <QWebView>

#include <QDebug>

bool operator < (const QSslCertificate & c1, const QSslCertificate & c2) { return c1.digest() < c2.digest(); }

namespace Utopia
{

// --- Not sure what this next block was in here for, so it's commented out for now
//
//     namespace
//     {
//         QLabel * newLabel(const QString & text, const QString & name)
//         {
//             QLabel * l = new QLabel(text);
//             l->setObjectName(name);
//             return l;
//         }
//
//         QLabel * newFieldLabel(const QString & text) { return newLabel(text, "field"); }
//         QLabel * newValueLabel(const QString & text) { return newLabel(text, "value"); }
//         QLabel * newErrorLabel(const QString & text) { return newLabel(text, "error"); }
//     }

    CertificateErrorDialogPrivate::CertificateErrorDialogPrivate(const QString & host, const QList< QSslError > & sslErrors, CertificateErrorDialog * dialog)
        : QObject(dialog), dialog(dialog), host(host), policy(CertificateErrorDialog::Deny)
    {
        dialog->setWindowTitle("There is a problem with security");
        QVBoxLayout * mainLayout = new QVBoxLayout(dialog);
        mainLayout->setContentsMargins(8, 8, 8, 8);
        dialog->setMinimumWidth(560);
        dialog->resize(560, 400);

        foreach (QSslError sslError, sslErrors) {
            QSslCertificate certificate = sslError.certificate();
            sslErrorsByCertificate[certificate].append(sslError.error());
        }

        QLabel * icon = new QLabel;
        icon->setPixmap(QPixmap(":/assets/ssl-warning.png"));
        QLabel * label = new QLabel(
            "Utopia has attempted to securely access a remote resource, but has found a problem in "
            "how that resource is configured. Details of the problem can be found below. If you are "
            "not sure why you are getting this error, you should 'Deny' access, as it could "
            "represent an attempt to compromise your security."
        );
        label->setWordWrap(true);
        QHBoxLayout * iconLayout = new QHBoxLayout;
        iconLayout->setContentsMargins(0, 0, 0, 0);
        iconLayout->addWidget(icon, 0);
        iconLayout->addWidget(label, 1);
        iconLayout->addSpacing(10);
        mainLayout->addLayout(iconLayout);

        QListWidget * listWidget = new QListWidget;
        connect(listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onCertificateListCurrentRowChanged(int)));
        mainLayout->addWidget(listWidget);
        listWidget->setVisible(sslErrorsByCertificate.size() > 1);
        foreach (QSslCertificate certificate, sslErrorsByCertificate.keys()) {
            listWidget->addItem(certificate.subjectInfo(QSslCertificate::CommonName).join(" "));
        }

        infoFrame = new QWebView;
        infoFrame->setObjectName("info");
        mainLayout->addWidget(infoFrame);

        alwaysCheckBox = new QCheckBox("Always allow this certificate");
        mainLayout->addWidget(alwaysCheckBox);

        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(0);
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        QPushButton * allowButton = new QPushButton("Allow");
        connect(allowButton, SIGNAL(clicked()), this, SLOT(onAllowButtonClicked()));
        QPushButton * denyButton = new QPushButton("Deny");
        denyButton->setDefault(true);
        connect(denyButton, SIGNAL(clicked()), dialog, SLOT(reject()));
        buttonLayout->addWidget(allowButton);
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(denyButton);
        mainLayout->addLayout(buttonLayout);

        QFile f(":/assets/certificate.html");
        f.open(QIODevice::ReadOnly);
        htmlTemplate = QString::fromUtf8(f.readAll());

        // Initialise
        listWidget->setCurrentRow(0);
        dialog->resize(dialog->width(), dialog->layout()->minimumHeightForWidth(560));

        // Set image (work around a sizing bug with images)
        infoFrame->page()->mainFrame()->findFirstElement("#certificate").setAttribute("src", "qrc:/assets/ssl-certificate.png");
    }

    void CertificateErrorDialogPrivate::onAllowButtonClicked()
    {
        policy = alwaysCheckBox->isChecked() ? CertificateErrorDialog::AllowAlways : CertificateErrorDialog::AllowSession;
        dialog->accept();
    }



    void CertificateErrorDialogPrivate::onCertificateListCurrentRowChanged(int row)
    {
        // Generate appropriate HTML and pass it to the webview
        QString html(htmlTemplate);
        QStringList errorClasses;
        errorClasses << "#dummy";

        QMap< QSslCertificate::SubjectInfo, QString > tagNames;
        tagNames[QSslCertificate::Organization] = "Organization";
        tagNames[QSslCertificate::CommonName] = "Common Name";
        tagNames[QSslCertificate::LocalityName] = "Locality";
        tagNames[QSslCertificate::OrganizationalUnitName] = "Organizational Unit";
        tagNames[QSslCertificate::CountryName] = "Country";
        tagNames[QSslCertificate::StateOrProvinceName] = "State/Province";
        QMap< QSslCertificate::SubjectInfo, QString > tagClasses;
        tagClasses[QSslCertificate::Organization] = "o";
        tagClasses[QSslCertificate::CommonName] = "cn";
        tagClasses[QSslCertificate::LocalityName] = "l";
        tagClasses[QSslCertificate::OrganizationalUnitName] = "ou";
        tagClasses[QSslCertificate::CountryName] = "c";
        tagClasses[QSslCertificate::StateOrProvinceName] = "st";
        QList< QSslCertificate::SubjectInfo > tags;
        tags << QSslCertificate::CountryName;
        tags << QSslCertificate::LocalityName;
        tags << QSslCertificate::StateOrProvinceName;
        tags << QSslCertificate::Organization;
        tags << QSslCertificate::OrganizationalUnitName;
        tags << QSslCertificate::CommonName;

        if (row < sslErrorsByCertificate.size()) {
            // Get certificate in question
            QSslCertificate certificate(sslErrorsByCertificate.keys().at(row));
            QList< QSslError::SslError > errors(sslErrorsByCertificate[certificate]);

            // Find a printable name for this certificate's subject
            QString subjectName;
            for (int i = tags.size() - 1; i > 0; --i) {
                QStringList info(certificate.subjectInfo(tags.at(i)));
                if (!info.isEmpty()) {
                    subjectName = info.join(" ");
                    break;
                }
            }

            // And the common name of the subject
            QString subjectCommonName(certificate.subjectInfo(QSslCertificate::CommonName).join(" "));

            // Find a printable name for this certificate's issuer
            QString issuerName;
            for (int i = tags.size() - 1; i > 0; --i) {
                QStringList info(certificate.issuerInfo(tags.at(i)));
                if (!info.isEmpty()) {
                    issuerName = info.join(" ");
                    break;
                }
            }

            // Get the valid dates
            int day = certificate.effectiveDate().date().day();
            QString suffix = (day == 1 || day == 21 || day == 31) ? "st" : (day == 2 || day == 22) ? "nd" : (day == 3 || day == 23) ? "rd" : "th";
            QString validFrom = certificate.effectiveDate().toString("dddd, d%1 MMMM yyyy, hh:mm:ss").arg(suffix);
            QString validTo = certificate.expiryDate().toString("dddd, d%1 MMMM yyyy, hh:mm:ss").arg(suffix);

            // Subject information
            QString subjectInfo;
            foreach (QSslCertificate::SubjectInfo tag, tags) {
                QStringList value(certificate.subjectInfo(tag));
                if (!value.isEmpty()) {
                    subjectInfo += QString("<div class=\"field subject-%1\">%2</div><div id=\"subject-%1\" class=\"value\">%3</div>\n").arg(tagClasses[tag], tagNames[tag], value.join(" "));
                }
            }

            // Issuer information
            QString issuerInfo;
            foreach (QSslCertificate::SubjectInfo tag, tags) {
                QStringList value(certificate.issuerInfo(tag));
                if (!value.isEmpty()) {
                    issuerInfo += QString("<div class=\"field issuer-%1\">%2</div><div id=\"issuer-%1\" class=\"value\">%3</div>\n").arg(tagClasses[tag], tagNames[tag], value.join(" "));
                }
            }

            QString notice;

            if (errors.contains(QSslError::CertificateNotYetValid)) {
                errorClasses << "#effective";
                notice = "This %1 certificate is not yet valid";
            } else if (errors.contains(QSslError::CertificateExpired)) {
                errorClasses << "#expiry";
                notice = "This %1 certificate has expired";
            }
            if (errors.contains(QSslError::SelfSignedCertificate) ||
                errors.contains(QSslError::SelfSignedCertificateInChain)) {
                errorClasses << "#issuer-name";
                if (notice.isEmpty()) {
                    notice = "This certificate is self-signed";
                } else {
                    notice = notice.arg("self-signed");
                }
            } else if (!notice.isEmpty()) {
                notice = notice.arg("");
            }
            if (errors.contains(QSslError::HostNameMismatch)) {
                errorClasses << "#subject-cn" << "#host" << "#subject-name";
                if (!notice.isEmpty()) {
                    notice = notice + QString(", and is not valid for the requested resource");
                } else {
                    notice = QString("This certificate is not valid for the requested resource");
                }
            }

            if (!notice.isEmpty()) {
                errorClasses << "#notice";
            }

            // Compile summary information
            html = html.arg(host, subjectName, issuerName, validFrom, validTo, notice, subjectInfo, issuerInfo, errorClasses.join(", "));
            //qDebug() << html;
        }

        infoFrame->setContent(html.toUtf8(), "text/html");

        QRect summaryRect = infoFrame->page()->mainFrame()->findFirstElement("#summary").geometry();
        if (!summaryRect.isNull()) {
            infoFrame->setMinimumHeight(summaryRect.height());
        }
    }




    CertificateErrorDialog::CertificateErrorDialog(const QString & host, const QList< QSslError > & sslErrors, QWidget * parent, Qt::WindowFlags f)
        : QDialog(parent, f), d(new CertificateErrorDialogPrivate(host, sslErrors, this))
    {}

    CertificateErrorDialog::AllowPolicy CertificateErrorDialog::allowPolicy() const
    {
        return d->policy;
    }

}
