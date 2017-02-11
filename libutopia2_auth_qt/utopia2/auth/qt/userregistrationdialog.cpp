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

#include <utopia2/auth/qt/userregistrationdialog_p.h>
#include <utopia2/auth/qt/userregistrationdialog.h>
#include <utopia2/auth/qt/resetpassworddialog.h>
#include <utopia2/auth/service.h>
#include <utopia2/qt/imagecroppingdialog.h>
#include <utopia2/qt/imageformatmanager.h>
#include <utopia2/qt/spinner.h>

#include <QBuffer>
#include <QComboBox>
#include <QEvent>
#include <QEventLoop>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPushButton>

#include <QDebug>

namespace Kend
{

    QLabel * UserRegistrationDialogPrivate::verticalLabel(const QString & msg, QWidget * parent)
    {
        QLabel * label = new QLabel(parent);
        label->setObjectName("verticalGroupLabel");
        label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        label->ensurePolished();

        QRect rect(QFontMetrics(label->font()).boundingRect(msg).adjusted(0, 0, 10, 10));
        rect.moveTop(0);
        QPixmap pixmap(rect.height(), rect.width());
        pixmap.fill(Qt::transparent);

        label->setMinimumSize(pixmap.size());

        {
            QPainter painter(&pixmap);
            painter.initFrom(label);
            painter.rotate(-90);
            painter.translate(-rect.width(), 0);
            painter.drawText(rect, Qt::TextDontClip | Qt::AlignCenter, msg);
        }

        label->setPixmap(pixmap);
        return label;
    }

    UserRegistrationDialogPrivate::UserRegistrationDialogPrivate(UserRegistrationDialog * dialog)
        : QObject(dialog), dialog(dialog), avatarChanged(false)
    {
        emailConfirmNoticeTemplate = QString(
            "You will shortly receive a confirmation email to the address you just "
            "registered (%1). Before you can log in to Utopia Documents for the first "
            "time, you must follow the instructions in that email to active your "
            "account."
        );

        dialog->setFixedWidth(500);

        QVBoxLayout * dialogLayout = new QVBoxLayout(dialog);
        dialogLayout->setSpacing(0);
        dialogLayout->setContentsMargins(0, 0, 0, 0);

        gridLayout = new QGridLayout;
        gridLayout->setSpacing(8);
        gridLayout->setContentsMargins(12, 12, 12, 0);
        dialogLayout->addLayout(gridLayout);

        QLabel * stepLabel = verticalLabel("Step 1", dialog);
        gridLayout->addWidget(stepLabel, 0, 0, 9, 1, Qt::AlignLeft);
        QLabel * msgLabel = new QLabel;
        msgLabel->setText(
            "Please provide some personal information about yourself here. You can also "
            "choose a profile picture for your account. This information will be visible "
            "to other users."
        );
        msgLabel->setWordWrap(true);
        gridLayout->addWidget(msgLabel, 1, 1, 1, 4);

        // Title
        QLabel * titleLabel = new QLabel("Title:");
        gridLayout->addWidget(titleLabel, 2, 1, Qt::AlignRight);
        title = new QComboBox;
        title->setEditable(true);
        gridLayout->addWidget(title, 2, 2);

        // Forename
        QLabel * forenameLabel = new QLabel("Forename:");
        gridLayout->addWidget(forenameLabel, 3, 1, Qt::AlignRight);
        forename = new QLineEdit;
        gridLayout->addWidget(forename, 3, 2);

        // Surname
        QLabel * surnameLabel = new QLabel("Surname:");
        gridLayout->addWidget(surnameLabel, 4, 1, Qt::AlignRight);
        surname = new QLineEdit;
        gridLayout->addWidget(surname, 4, 2);

        // Institution
        QLabel * institutionLabel = new QLabel("Institution:");
        gridLayout->addWidget(institutionLabel, 5, 1, Qt::AlignRight);
        institution = new QLineEdit;
        gridLayout->addWidget(institution, 5, 2);

        QFrame * avatarFrame = new QFrame;
        avatarFrame->setObjectName("avatarFrame");
        avatarFrame->setFixedSize(128, 128);
        QVBoxLayout * avatarFrameLayout = new QVBoxLayout(avatarFrame);
        avatarFrameLayout->setContentsMargins(1, 1, 1, 1);
        avatar = new QLabel;
        avatar->setScaledContents(true);
        avatar->installEventFilter(this);
        avatar->setCursor(Qt::PointingHandCursor);
        avatar->setToolTip("Choose picture...");
        avatarFrameLayout->addWidget(avatar);
        gridLayout->addWidget(avatarFrame, 2, 3, 5, 2, Qt::AlignTop | Qt::AlignHCenter);

        QPushButton * avatarButton = new QPushButton("Choose picture...");
        connect(avatarButton, SIGNAL(clicked()), this, SLOT(pickAvatar()));
        gridLayout->addWidget(avatarButton, 7, 1, 1, 4, Qt::AlignTop | Qt::AlignRight);

        /////////////////////////////////////////////////////////////////////////////////
        // Email address

        // Email
        stepLabel = verticalLabel("Step 2", dialog);
        gridLayout->addWidget(stepLabel, 9, 0, 4, 1, Qt::AlignLeft);
        emailDisclaimer = new QLabel(
            "You will use your email address to log in to Utopia. You will need to "
            "confirm your email address before you can use your account; instructions "
            "will be emailed to this address."
            );
        emailDisclaimer->setWordWrap(true);
        gridLayout->addWidget(emailDisclaimer, 10, 1, 1, 4);

        QLabel * emailLabel = new QLabel("Email address:");
        gridLayout->addWidget(emailLabel, 11, 1, Qt::AlignRight);
        email = new QLineEdit;
        gridLayout->addWidget(email, 11, 2, 1, 2);
        emailErrorIcon = new QLabel;
        emailErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(emailErrorIcon, 11, 4, 1, 1, Qt::AlignCenter);

        emailErrorText = new QLabel;
        emailErrorText->setObjectName("errorLabel");
        emailErrorText->setWordWrap(true);
        emailErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        connect(emailErrorText, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));
        emailErrorText->hide();
        gridLayout->addWidget(emailErrorText, 12, 2, 1, 2);

        stepLabel = verticalLabel("Step 3", dialog);
        gridLayout->addWidget(stepLabel, 13, 0, 5, 1, Qt::AlignLeft);
        passwordDisclaimer = new QLabel(
            "Please choose (and confirm) a password for your new account. A combination "
            "of letters, numbers and other characters that is not easy to guess will be "
            "more secure."
            );
        passwordDisclaimer->setWordWrap(true);
        gridLayout->addWidget(passwordDisclaimer, 14, 1, 1, 4);

        // Password 1
        QLabel * newPassword1Label = new QLabel("Password:");
        gridLayout->addWidget(newPassword1Label, 15, 1, Qt::AlignRight);
        newPassword1 = new QLineEdit;
        newPassword1->setEchoMode(QLineEdit::Password);
        gridLayout->addWidget(newPassword1, 15, 2, 1, 2);
        newPasswordErrorIcon = new QLabel;
        newPasswordErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(newPasswordErrorIcon, 15, 4, 2, 1, Qt::AlignCenter);

        // Password 2
        QLabel * newPassword2Label = new QLabel("Retype password:");
        gridLayout->addWidget(newPassword2Label, 16, 1, Qt::AlignRight);
        newPassword2 = new QLineEdit;
        newPassword2->setEchoMode(QLineEdit::Password);
        gridLayout->addWidget(newPassword2, 16, 2, 1, 2);

        newPasswordErrorText = new QLabel;
        newPasswordErrorText->setObjectName("errorLabel");
        newPasswordErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        newPasswordErrorText->hide();
        gridLayout->addWidget(newPasswordErrorText, 17, 2, 1, 2);

        /////////////////////////////////////////////////////////////////////////////////
        // Control buttons
        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(12, 12, 12, 12);
        buttonLayout->setSpacing(8);
        QPushButton * okButton = new QPushButton("Register");
        okButton->setDefault(true);
        connect(okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
        QPushButton * cancelButton = new QPushButton("Cancel");
        cancelButton->setDefault(false);
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelButtonClicked()));
        spinner = new Utopia::Spinner;
        spinner->setFixedSize(18, 18);
        buttonLayout->addWidget(cancelButton, 0);
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(spinner, 0);
        buttonLayout->addWidget(okButton, 0);
        dialogLayout->addStretch(1);
        dialogLayout->addLayout(buttonLayout, 0);

        // Layout
        gridLayout->setColumnStretch(0, 0);
        gridLayout->setColumnStretch(1, 0);
        gridLayout->setColumnStretch(2, 100);
        gridLayout->setColumnStretch(3, 1);
        gridLayout->setColumnStretch(4, 0);

        gridLayout->setColumnMinimumWidth(0, stepLabel->minimumWidth() + 10);

        gridLayout->setRowStretch(6, 1);

        dialog->setFixedHeight(dialog->heightForWidth(500));
        dialog->setSizeGripEnabled(false);
    }

    bool UserRegistrationDialogPrivate::commit(const QMap< QString, QString > & values)
    {
        static const QString conflictMsg(
            "This email address is already registered with Utopia. <a href='forgotten'>Forgotten your password?</a>"
        );
        static const QString networkErrorMsg(
            "A network error has occurred that has prevented your request from being "
            "processed. Please check your network connectivity and try again."
        );
        static const QString unknownErrorMsg(
            "Oops! Something went wrong when processing your request. Please try again."
        );

        static const QString user_tpl(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<user xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                 " xmlns=\"http://utopia.cs.manchester.ac.uk/authd\""
                 " version=\"0.3\""
                 " service=\"%1\""
                 " xsi:schemaLocation=\"http://utopia.cs.manchester.ac.uk/authd https://utopia.cs.manchester.ac.uk/authd/0.3/xsd/user\">"
            "%2"
            "</user>");
        static const QString info_tpl("<info name=\"%1\">%2</info>");

        bool error = false;
        if (service) {
            dialog->setEnabled(false);
            spinner->start();

            // Create a registration document
            QMapIterator< QString, QString > iter(values);
            QString infoStr;
            while (iter.hasNext()) {
                iter.next();
                QString key(iter.key());
                QString value(iter.value());
                infoStr += info_tpl.arg(key.replace("\"", "&quot;"), value.replace("<", "&lt;"));
            }
            QString userStr = user_tpl.arg(service->authenticationMethod(), infoStr);

            // Find registration URL
            QUrl authUrl(service->resourceUrl(Service::AuthenticationResource));
            authUrl.setPath(authUrl.path() + "/users");

            QEventLoop commitLoop;
            QNetworkReply * reply = service->post(QNetworkRequest(authUrl), userStr.toUtf8(), "application/x-authd+xml;version=0.3;type=user");
            connect(reply, SIGNAL(finished()), &commitLoop, SLOT(quit()));
            error = commitLoop.exec() != 0;
            reply->deleteLater();

            // Success?
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 409) { // Already exists
                setEmailError(conflictMsg);
                error = true;
            } else {
                switch (reply->error()) {
                case QNetworkReply::ConnectionRefusedError: // Network errors
                case QNetworkReply::RemoteHostClosedError:
                case QNetworkReply::HostNotFoundError:
                case QNetworkReply::TimeoutError:
                case QNetworkReply::TemporaryNetworkFailureError:
                case QNetworkReply::UnknownNetworkError:
                    QMessageBox::warning(dialog, "Network error", networkErrorMsg);
                    error = true;
                    break;
                case QNetworkReply::NoError:
                    // SUCCESS
                    break;
                default: // some other error!?
                    QMessageBox::warning(dialog, "Unexpected error", unknownErrorMsg);
                    error = true;
                    break;
                }
            }

            spinner->stop();
            dialog->setEnabled(true);
        }
        return !error;
    }

    bool UserRegistrationDialogPrivate::eventFilter(QObject * obj, QEvent * e)
    {
        if (obj == avatar && e->type() == QEvent::MouseButtonRelease) {
            pickAvatar();
        }

        return QObject::eventFilter(obj, e);
    }

    void UserRegistrationDialogPrivate::informEmailConfirmation()
    {
        static QString title("New account requested");
        static QString msg(
            "You will shortly receive an email with instructions on how "
            "to confirm the registration of your email address."
        );
        QMessageBox::information(dialog, title, msg);
    }

    void UserRegistrationDialogPrivate::onCancelButtonClicked()
    {
        dialog->reject();
    }

    void UserRegistrationDialogPrivate::onCommitCompleted(bool success)
    {
        commitLoop.exit(success ? 0 : 1);
    }

    void UserRegistrationDialogPrivate::onLinkActivated(const QString & link)
    {
        if (service && link == "forgotten") {
            ResetPasswordDialog dialog;
            dialog.setService(service);
            dialog.setEmail(email->text());
            dialog.exec();
        }
    }

    void UserRegistrationDialogPrivate::onOkButtonClicked()
    {
        static const QRegExp emailRegExp("[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?");
        static const QString invalidEmailErrorText("You must enter a valid email address");
        static const QString passwordErrorText("Passwords must match and cannot be empty");

        // Reset error labels
        setEmailError(QString());
        setPasswordError(QString());

        resize();

        // Apply changes
        bool error = false;
        QMap< QString, QString > values;
        if (service) {
            // Gather personal details (all optional)
            QString titleText = title->currentIndex() > 0 ? title->itemText(title->currentIndex()) : QString();
            if (!titleText.isEmpty()) {
                values["title"] = titleText;
            }
            QString forenameText(forename->text());
            if (!forenameText.isEmpty()) {
                values["forename"] = forenameText;
            }
            QString surnameText(surname->text());
            if (!surnameText.isEmpty()) {
                values["surname"] = surnameText;
            }
            QString institutionText(institution->text());
            if (!institutionText.isEmpty()) {
                values["institution"] = institutionText;
            }

            // Email address - MANDATORY
            QString emailText(email->text());
            if (!emailText.isEmpty() && emailRegExp.exactMatch(emailText)) {
                values["email"] = emailText;
            } else {
                setEmailError(invalidEmailErrorText);
                error = true;
            }

            // Check for entered and matching passwords
            if (newPassword1->text().isEmpty() && newPassword2->text().isEmpty()) { // EMPTY
                setPasswordError(passwordErrorText);
                error = true;
            } else if (newPassword1->text() != newPassword2->text()) { // MISMATCH
                setPasswordError(passwordErrorText);
                error = true;
            } else { // FIXME sensible password?
                values["password"] = newPassword1->text();
            }

            // Avatar
            if (avatarChanged) {
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::WriteOnly);
                avatar->pixmap()->save(&buffer, "PNG");
                values["avatar"] = QString::fromUtf8(bytes.toBase64());
            }
        }

        resize();

        if (!error && commit(values)) {
            informEmailConfirmation();
            dialog->accept();
        }

        resize();
    }

    void UserRegistrationDialogPrivate::pickAvatar()
    {
        QPixmap pixmap(Utopia::ImageFormatManager::loadImageFile(dialog, "Choose a profile image..."));
        if (!pixmap.isNull()) {
            boost::scoped_ptr< Utopia::ImageCroppingDialog > imageCroppingDialog(new Utopia::ImageCroppingDialog(pixmap, dialog));
            if(imageCroppingDialog->exec() == QDialog::Accepted) {
                QPixmap newPixmap = imageCroppingDialog->pixmap();
                if (!newPixmap.isNull()) {
                    avatar->setPixmap(newPixmap);
                    avatarChanged = true;
                }
            }
        }
    }

    void UserRegistrationDialogPrivate::reset(Service * service)
    {
        this->service = service;

        forename->clear();
        surname->clear();
        institution->clear();
        avatar->setPixmap(QPixmap(":/icons/default-avatar.png"));
        avatarChanged = false;

        QStringList titles;
        titles << "[Please Choose...]" << "Dr" << "Miss" << "Mr" << "Mrs" << "Ms" << "Professor";
        title->clear();
        title->addItems(titles);

        if (service) {

        }
    }

    void UserRegistrationDialogPrivate::resize()
    {
        gridLayout->layout()->activate();
        dialog->layout()->activate();
        dialog->setFixedHeight(dialog->heightForWidth(500));
    }

    void UserRegistrationDialogPrivate::setEmailError(const QString & msg)
    {
        if (msg.isEmpty()) {
            emailErrorText->hide();
            emailErrorIcon->setPixmap(QPixmap());
        } else {
            emailErrorText->setText(msg);
            emailErrorText->show();
            emailErrorIcon->setPixmap(QPixmap(":/icons/error-cross.png"));
        }
    }

    void UserRegistrationDialogPrivate::setPasswordError(const QString & msg)
    {
        if (msg.isEmpty()) {
            newPasswordErrorText->hide();
            newPasswordErrorIcon->setPixmap(QPixmap());
        } else {
            newPasswordErrorText->setText(msg);
            newPasswordErrorText->show();
            newPasswordErrorIcon->setPixmap(QPixmap(":/icons/error-cross.png"));
        }
    }




    UserRegistrationDialog::UserRegistrationDialog(QWidget * parent)
        : QDialog(parent), d(new UserRegistrationDialogPrivate(this))
    {
        setWindowTitle("Register an account");
    }

    void UserRegistrationDialog::clear()
    {
        d->reset();
    }

    void UserRegistrationDialog::setService(Service * service)
    {
        d->reset(service);
    }

    void UserRegistrationDialog::showEvent(QShowEvent * e)
    {
        d->resize();
        QDialog::showEvent(e);
    }

} // namespace Kend
