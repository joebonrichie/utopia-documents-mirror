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

#include <utopia2/auth/qt/userinfoeditor_p.h>
#include <utopia2/auth/qt/userinfoeditor.h>
#include <utopia2/auth/qt/userlabel.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/user.h>
#include <utopia2/qt/imagecroppingdialog.h>
#include <utopia2/qt/imageformatmanager.h>
#include <utopia2/qt/spinner.h>

#include <QComboBox>
#include <QEventLoop>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include <QDebug>

namespace Kend
{

    UserInfoEditorPrivate::UserInfoEditorPrivate(UserInfoEditor * editor)
        : QObject(editor), editor(editor)
    {
        emailChangeNoticeTemplate = QString(
            "<strong>Note:</strong> You have recently requested to change your email "
            "address to %1 but you have not yet confirmed the change.<br>"
            "<a href=\"confirm\">Resend confirmation email</a> or "
            "<a href=\"cancel\">cancel the change</a>."
        );

        editor->setFixedWidth(500);

        QVBoxLayout * dialogLayout = new QVBoxLayout(editor);
        dialogLayout->setSpacing(0);
        dialogLayout->setContentsMargins(0, 0, 0, 0);

        QVBoxLayout * groupsLayout = new QVBoxLayout;
        groupsLayout->setSpacing(10);
        groupsLayout->setContentsMargins(12, 12, 12, 4);
        dialogLayout->addLayout(groupsLayout);

        /////////////////////////////////////////////////////////////////////////////////
        // Personal info
        QGroupBox * userGroup = new QGroupBox("Personal information");
        groupsLayout->addWidget(userGroup);

        QGridLayout * gridLayout = new QGridLayout(userGroup);
        gridLayout->setContentsMargins(12, 12, 12, 12);
        gridLayout->setSpacing(8);

        // Title
        QLabel * titleLabel = new QLabel("Title:");
        gridLayout->addWidget(titleLabel, 0, 0, Qt::AlignRight);
        title = new QComboBox;
        title->setEditable(true);
        gridLayout->addWidget(title, 0, 1);

        // Forename
        QLabel * forenameLabel = new QLabel("Forename:");
        gridLayout->addWidget(forenameLabel, 1, 0, Qt::AlignRight);
        forename = new QLineEdit;
        gridLayout->addWidget(forename, 1, 1);

        // Surname
        QLabel * surnameLabel = new QLabel("Surname:");
        gridLayout->addWidget(surnameLabel, 2, 0, Qt::AlignRight);
        surname = new QLineEdit;
        gridLayout->addWidget(surname, 2, 1);

        // Institution
        QLabel * institutionLabel = new QLabel("Institution:");
        gridLayout->addWidget(institutionLabel, 3, 0, Qt::AlignRight);
        institution = new QLineEdit;
        gridLayout->addWidget(institution, 3, 1);

        QFrame * avatarFrame = new QFrame;
        avatarFrame->setObjectName("avatarFrame");
        avatarFrame->setFixedSize(128, 128);
        QVBoxLayout * avatarFrameLayout = new QVBoxLayout(avatarFrame);
        avatarFrameLayout->setContentsMargins(1, 1, 1, 1);
        avatar = new UserLabel(UserLabel::Avatar);
        connect(avatar, SIGNAL(clicked()), this, SLOT(pickAvatar()));
        avatarFrameLayout->addWidget(avatar);
        gridLayout->addWidget(avatarFrame, 0, 2, 4, 1, Qt::AlignTop | Qt::AlignHCenter);

        /////////////////////////////////////////////////////////////////////////////////
        // Email address
        emailGroup = new QGroupBox("Registered email address");
        groupsLayout->addWidget(emailGroup);

        gridLayout = new QGridLayout(emailGroup);
        gridLayout->setContentsMargins(12, 12, 12, 12);
        gridLayout->setSpacing(8);

        // Title
        emailDisclaimer = new QLabel(
            "You will need to confirm a new email address before this change is saved; "
            "instructions will be emailed to the new address."
            );
        emailDisclaimer->setWordWrap(true);
        emailDisclaimer->setObjectName("disclaimer");
        gridLayout->addWidget(emailDisclaimer, 0, 0, 1, 3);

        QLabel * emailLabel = new QLabel("Email address:");
        gridLayout->addWidget(emailLabel, 1, 0, Qt::AlignRight);
        email = new QLineEdit;
        gridLayout->addWidget(email, 1, 1);
        emailErrorIcon = new QLabel;
        emailErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(emailErrorIcon, 1, 2, Qt::AlignCenter);

        emailErrorText = new QLabel;
        emailErrorText->setObjectName("errorLabel");
        emailErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        emailErrorText->setWordWrap(true);
        emailErrorText->hide();
        gridLayout->addWidget(emailErrorText, 2, 1);

        // Password 0
        QLabel * confirmPasswordLabel = new QLabel("Confirm password:");
        gridLayout->addWidget(confirmPasswordLabel, 3, 0, Qt::AlignRight);
        confirmPassword = new QLineEdit;
        confirmPassword->setEchoMode(QLineEdit::Password);
        gridLayout->addWidget(confirmPassword, 3, 1);
        confirmPasswordErrorIcon = new QLabel;
        confirmPasswordErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(confirmPasswordErrorIcon, 3, 2, 1, 1, Qt::AlignCenter);

        confirmPasswordErrorText = new QLabel("Incorrect password");
        confirmPasswordErrorText->setObjectName("errorLabel");
        confirmPasswordErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        confirmPasswordErrorText->setWordWrap(true);
        confirmPasswordErrorText->hide();
        gridLayout->addWidget(confirmPasswordErrorText, 4, 1);

        emailChangeNotice = new QLabel;
        emailChangeNotice->setWordWrap(true);
        emailChangeNotice->setObjectName("disclaimer");
        connect(emailChangeNotice, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));
        gridLayout->addWidget(emailChangeNotice, 5, 1);

        /////////////////////////////////////////////////////////////////////////////////
        // Email address
        passwordGroup = new QGroupBox("Account password");
        groupsLayout->addWidget(passwordGroup);

        gridLayout = new QGridLayout(passwordGroup);
        gridLayout->setContentsMargins(12, 12, 12, 12);
        gridLayout->setSpacing(8);

        passwordDisclaimer = new QLabel(
            "To optionally change your password, enter the current and new passwords below."
            );
        passwordDisclaimer->setWordWrap(true);
        passwordDisclaimer->setObjectName("disclaimer");
        gridLayout->addWidget(passwordDisclaimer, 0, 0, 1, 3);

        // Password 0
        QLabel * oldPasswordLabel = new QLabel("Current password:");
        gridLayout->addWidget(oldPasswordLabel, 1, 0, Qt::AlignRight);
        oldPassword = new QLineEdit;
        oldPassword->setEchoMode(QLineEdit::Password);
        gridLayout->addWidget(oldPassword, 1, 1);
        oldPasswordErrorIcon = new QLabel;
        oldPasswordErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(oldPasswordErrorIcon, 1, 2, 1, 1, Qt::AlignCenter);

        oldPasswordErrorText = new QLabel("Incorrect password");
        oldPasswordErrorText->setObjectName("errorLabel");
        oldPasswordErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        oldPasswordErrorText->hide();
        gridLayout->addWidget(oldPasswordErrorText, 2, 1);

        // Password 1
        QLabel * newPassword1Label = new QLabel("New password:");
        gridLayout->addWidget(newPassword1Label, 3, 0, Qt::AlignRight);
        newPassword1 = new QLineEdit;
        newPassword1->setEchoMode(QLineEdit::Password);
        gridLayout->addWidget(newPassword1, 3, 1);
        newPasswordErrorIcon = new QLabel;
        newPasswordErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(newPasswordErrorIcon, 3, 2, 2, 1, Qt::AlignCenter);

        // Password 2
        QLabel * newPassword2Label = new QLabel("Retype new password:");
        gridLayout->addWidget(newPassword2Label, 4, 0, Qt::AlignRight);
        newPassword2 = new QLineEdit;
        newPassword2->setEchoMode(QLineEdit::Password);
        gridLayout->addWidget(newPassword2, 4, 1);

        newPasswordErrorText = new QLabel("Passwords must match and cannot be empty");
        newPasswordErrorText->setObjectName("errorLabel");
        newPasswordErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        newPasswordErrorText->hide();
        gridLayout->addWidget(newPasswordErrorText, 5, 1);

        /////////////////////////////////////////////////////////////////////////////////
        // Control buttons
        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(12, 12, 12, 12);
        buttonLayout->setSpacing(8);
        QPushButton * okButton = new QPushButton("Save");
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
        gridLayout->setColumnStretch(1, 1);
        gridLayout->setColumnStretch(2, 0);

        editor->setFixedHeight(editor->heightForWidth(500));
        editor->setSizeGripEnabled(false);
    }

    bool UserInfoEditorPrivate::commit(bool includeAvatar)
    {
        bool error = false;
        if (user && !user->isNull()) {
            editor->setEnabled(false);
            spinner->start();
            user->commit(includeAvatar);
            error = commitLoop.exec() != 0;
            spinner->stop();
            editor->setEnabled(true);
        }
        return error;
    }

    void UserInfoEditorPrivate::informEmailChange()
    {
        static QString title("Email change requested");
        static QString msg(
            "You will shortly receive an email with instructions on how "
            "to confirm your requested change of email address."
        );
        QMessageBox::information(editor, title, msg);
    }

    void UserInfoEditorPrivate::informEmailChangeCancel()
    {
        static QString title("Email change cancelled");
        static QString msg(
            "Your previous request to change your email address has "
            "been cancelled."
        );
        QMessageBox::information(editor, title, msg);
    }

    void UserInfoEditorPrivate::onCancelButtonClicked()
    {
        // Reset error labels
        setOldPasswordError(false);
        setNewPasswordError(false);

        // Revert changes
        if (user && !user->isNull()) {
            user->revert();
        }

        // Refresh information
        onUserInfoChanged();

        resize();

        editor->reject();
    }

    void UserInfoEditorPrivate::onCommitCompleted(bool success)
    {
        commitLoop.exit(success ? 0 : 1);
    }

    void UserInfoEditorPrivate::onLinkActivated(const QString & link)
    {
        if (user && !user->isNull()) {
            if (link == "cancel") {
                user->set("revert_email", "yes");
                if (!commit(false)) {
                    informEmailChangeCancel();
                } else {
                    // FIXME problem?
                }
            } else { // "confirm"
                user->set("email", user->get("new_email"));
                if (!commit(false)) {
                    informEmailChange();
                } else {
                    // FIXME problem?
                }
            }
        }
    }

    void UserInfoEditorPrivate::onOkButtonClicked()
    {
        static const QString emptyPasswordMsg("You must type your password to make this change");
        static const QString incorrectPasswordMsg("Incorrect password");

        // Reset error labels
        setEmailError(QString());
        setConfirmPasswordError(QString());
        setOldPasswordError(false);
        setNewPasswordError(false);

        resize();

        // Apply changes
        bool error = false;
        QMap< QString, QString > newValues;
        if (user && !user->isNull()) {
            QString expectedCurrentPassword = service->credentials().value("password").toString();
            bool emailChangeRequested = false;

            // Gather changes to personal information
            if (title->currentIndex() > 0) {
                QString titleText = title->itemText(title->currentIndex());
                if (user->get("title") != titleText) {
                    newValues["title"] = titleText;
                }
            }
            if (user->get("forename") != forename->text()) {
                newValues["forename"] = forename->text();
            }
            if (user->get("surname") != surname->text()) {
                newValues["surname"] = surname->text();
            }
            if (user->get("institution") != institution->text()) {
                newValues["institution"] = institution->text();
            }

            // Check for new email management
            if (user->get("email") != email->text() || !confirmPassword->text().isEmpty()) {
                if (confirmPassword->text().isEmpty()) {
                    setConfirmPasswordError(emptyPasswordMsg);
                    error = true;
                } else if (confirmPassword->text() != expectedCurrentPassword) {
                    setConfirmPasswordError(incorrectPasswordMsg);
                    error = true;
                } else if (user->get("email") != email->text()) {
                    newValues["email"] = email->text();
                    emailChangeRequested = true;
                }
            }

            // Check passwords
            if (!oldPassword->text().isEmpty() ||
                !newPassword1->text().isEmpty() ||
                !newPassword2->text().isEmpty()) {

                if (expectedCurrentPassword != oldPassword->text()) {
                    setOldPasswordError(true);
                    error = true;
                }

                if (newPassword1->text().isEmpty() || newPassword1->text() != newPassword2->text()) {
                    setNewPasswordError(true);
                    error = true;
                }

                if (!error) {
                    newValues["password"] = newPassword1->text();
                }
            }

            if (!error && (user->isAvatarModified() || !newValues.isEmpty() || user->isInfoModified())) {
                QMapIterator< QString, QString > iter(newValues);
                while (iter.hasNext()) {
                    iter.next();
                    user->set(iter.key(), iter.value());
                }
                error = commit();
                if (!error) {
                    if (emailChangeRequested) {
                        informEmailChange();
                    }
                } else {
                    // FIXME error?
                    if (user->errorCode() == "ACCOUNT_EXISTS") {
                        setEmailError(user->errorString());
                    }
                }
            }
        }

        resize();

        if (!error) {
            editor->accept();
        }
    }

    void UserInfoEditorPrivate::onUserInfoChanged()
    {
        // Get new data
        QMap< QString, QString > info = user->info();
        bool editable = !user->isReadOnly();
        bool avatarEditable = !user->isAvatarReadOnly();

        // Title
        QString newTitle = info.value("title");
        if (!newTitle.isEmpty()) {
            bool found = false;
            for (int i = 0; !found && i < title->count(); ++i) {
                if (title->itemText(i) == newTitle) {
                    found = true;
                    title->setCurrentIndex(i);
                }
            }
            if (!found) {
                title->addItem(newTitle);
                title->setCurrentIndex(title->count() - 1);
            }
        }
        title->setEnabled(editable);

        // Forename
        QString newForename = info.value("forename");
        if (forename->text() != newForename) {
            forename->setText(newForename);
        }
        forename->setEnabled(editable);

        // Surname
        QString newSurname = info.value("surname");
        if (surname->text() != newSurname) {
            surname->setText(newSurname);
        }
        surname->setEnabled(editable);

        // Institution
        QString newInstitution = info.value("institution");
        if (institution->text() != newInstitution) {
            institution->setText(newInstitution);
        }
        institution->setEnabled(editable);

        // Avatar widget
        avatar->setToolTip(avatarEditable ? QString("Click to change avatar...") : QString());
        avatar->setClickable(avatarEditable);

        // Institution
        QString currentEmail = info.value("email");
        QString newEmail = info.value("new_email");
        emailDisclaimer->setVisible(editable);
        emailGroup->setVisible(!currentEmail.isEmpty());
        emailGroup->setEnabled(!currentEmail.isEmpty());
        email->setText(currentEmail);
        email->setEnabled(editable);
        emailErrorText->hide();
        emailErrorIcon->setPixmap(QPixmap());
        confirmPassword->setText(QString());
        confirmPasswordErrorText->hide();
        confirmPasswordErrorIcon->setPixmap(QPixmap());
        emailChangeNotice->setText(newEmail.isEmpty() ? QString() : emailChangeNoticeTemplate.arg(newEmail));
        emailChangeNotice->setWordWrap(true);
        emailChangeNotice->setVisible(!newEmail.isEmpty());

        passwordGroup->setVisible(editable);
        passwordGroup->setEnabled(editable);
        oldPassword->setText(QString());
        oldPasswordErrorText->hide();
        oldPasswordErrorIcon->setPixmap(QPixmap());
        newPassword1->setText(QString());
        newPassword2->setText(QString());
        newPasswordErrorText->hide();
        newPasswordErrorIcon->setPixmap(QPixmap());

        resize();
    }

    void UserInfoEditorPrivate::pickAvatar()
    {
        QPixmap pixmap(Utopia::ImageFormatManager::loadImageFile(editor, "Choose a profile image..."));
        if (user && !pixmap.isNull()) {
            boost::scoped_ptr< Utopia::ImageCroppingDialog > dialog(new Utopia::ImageCroppingDialog(pixmap, editor));
            if(dialog->exec() == QDialog::Accepted) {
                QPixmap newPixmap = dialog->pixmap();
                if (!newPixmap.isNull()) {
                    user->setAvatar(newPixmap);
                }
            }
        }
    }

    void UserInfoEditorPrivate::reset(Service * service, const QString & id)
    {
        this->service = service;
        User * newUser = 0;
        if (service) {
            newUser = service->user(id);
            user.reset(newUser);
        }

        forename->clear();
        surname->clear();
        institution->clear();
        avatar->clear();

        QStringList titles;
        titles << "[Please Choose...]" << "Dr" << "Miss" << "Mr" << "Mrs" << "Ms" << "Professor";
        title->clear();
        title->addItems(titles);

        if (newUser) {
            newUser->update();

            connect(newUser, SIGNAL(expired()), this, SLOT(reset()));
            connect(newUser, SIGNAL(setup()), this, SLOT(onUserInfoChanged()));
            connect(newUser, SIGNAL(infoChanged()), this, SLOT(onUserInfoChanged()));
            connect(newUser, SIGNAL(commitCompleted(bool)), this, SLOT(onCommitCompleted(bool)));

            avatar->setUser(newUser->clone());

            // Refresh data
            onUserInfoChanged();
        }
    }

    void UserInfoEditorPrivate::resize()
    {
        emailGroup->layout()->activate();
        editor->layout()->activate();
        editor->setFixedHeight(editor->heightForWidth(500));
    }

    void UserInfoEditorPrivate::setEmailError(const QString & msg)
    {
        emailErrorIcon->setPixmap(msg.isEmpty() ? QPixmap() : QPixmap(":/icons/error-cross.png"));
        emailErrorText->setText(msg);
        emailErrorText->setVisible(!msg.isEmpty());
    }

    void UserInfoEditorPrivate::setConfirmPasswordError(const QString & msg)
    {
        confirmPasswordErrorIcon->setPixmap(msg.isEmpty() ? QPixmap() : QPixmap(":/icons/error-cross.png"));
        confirmPasswordErrorText->setText(msg);
        confirmPasswordErrorText->setVisible(!msg.isEmpty());
    }

    void UserInfoEditorPrivate::setNewPasswordError(bool set)
    {
        newPasswordErrorIcon->setPixmap(set ? QPixmap(":/icons/error-cross.png") : QPixmap());
        newPasswordErrorText->setVisible(set);
    }

    void UserInfoEditorPrivate::setOldPasswordError(bool set)
    {
        oldPasswordErrorIcon->setPixmap(set ? QPixmap(":/icons/error-cross.png") : QPixmap());
        oldPasswordErrorText->setVisible(set);
    }




    UserInfoEditor::UserInfoEditor(QWidget * parent)
        : QDialog(parent), d(new UserInfoEditorPrivate(this))
    {
        setWindowTitle("Account information");
    }

    void UserInfoEditor::clear()
    {
        d->reset();
    }

    void UserInfoEditor::setUser(Service * service, const QString & id)
    {
        d->reset(service, id);
    }

    void UserInfoEditor::showEvent(QShowEvent * e)
    {
        d->resize();
        QDialog::showEvent(e);
    }

} // namespace Kend
