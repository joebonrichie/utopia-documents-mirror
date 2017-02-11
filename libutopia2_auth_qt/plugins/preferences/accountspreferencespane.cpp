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

#include "accountspreferencespane.h"

#include <utopia2/auth/servicemanagermodel.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/qt/userregistrationdialog.h>
#include <utopia2/auth/qt/userinfoeditor.h>
#include <utopia2/auth/qt/resetpassworddialog.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <QSpinBox>
#include <QStackedLayout>
#include <QTableView>
#include <QToolButton>
#include <QVariant>
#include <QVBoxLayout>

#include <QDebug>

namespace Kend
{

    NewAccountDialog::NewAccountDialog(QWidget * parent, Qt::WindowFlags f)
        : QDialog(parent, f), serviceManager(ServiceManager::instance())
    {
        setWindowTitle("Add Account...");

        QVBoxLayout * dialogLayout = new QVBoxLayout(this);
        dialogLayout->setContentsMargins(0, 0, 0, 0);
        dialogLayout->setSpacing(0);

        slideLayout = new Utopia::SlideLayout;
        dialogLayout->addLayout(slideLayout);
        dialogLayout->addStretch(1);

        page_one.widget = new QWidget;
        QGridLayout * stepOneLayout = new QGridLayout(page_one.widget);
        stepOneLayout->addWidget(new QLabel("Service URL:"), 0, 0);
        stepOneLayout->addWidget(page_one.serviceUrlLineEdit = new QLineEdit, 0, 1);
        stepOneLayout->addWidget(page_one.errorLabel = new QLabel, 1, 1);
        stepOneLayout->setColumnStretch(0, 0);
        stepOneLayout->setColumnStretch(1, 1);
        page_one.errorLabel->setObjectName("errorLabel");
        slideLayout->addWidget(page_one.widget, "1");

        page_two.widget = new QWidget;
        QGridLayout * stepTwoLayout = new QGridLayout(page_two.widget);
        stepTwoLayout->addWidget(new QLabel("Service:"), 0, 0);
        stepTwoLayout->addWidget(page_two.serviceDescription = new QLabel, 0, 1);
        stepTwoLayout->addWidget(new QLabel("Account Type:"), 1, 0);
        stepTwoLayout->addWidget(page_two.authenticationMethodComboBox = new QComboBox, 1, 1);
        stepTwoLayout->setColumnStretch(0, 0);
        stepTwoLayout->setColumnStretch(1, 1);
        slideLayout->addWidget(page_two.widget, "2");

        page_three.widget = new QWidget;
        QGridLayout * stepThreeLayout = new QGridLayout(page_three.widget);
        stepThreeLayout->addWidget(new QLabel("Service:"), 0, 0);
        stepThreeLayout->addWidget(page_three.serviceDescription = new QLabel, 0, 1);
        stepThreeLayout->addWidget(new QLabel("Account Type:"), 1, 0);
        stepThreeLayout->addWidget(page_three.authenticationMethod = new QLabel, 1, 1);
        stepThreeLayout->setColumnStretch(0, 0);
        stepThreeLayout->setColumnStretch(1, 1);
        slideLayout->addWidget(page_three.widget, "3");

        slideLayout->push("1", false);

        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(12, 4, 12, 12);
        buttonLayout->setSpacing(6);
        nextButton = new QPushButton("Next", this);
        previousButton = new QPushButton("Back", this);
        previousButton->hide();
        cancelButton = new QPushButton("Cancel", this);
        doneButton = new QPushButton("Done", this);
        doneButton->hide();
        buttonLayout->addWidget(previousButton);
        buttonLayout->addSpacing(10);
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(doneButton);
        buttonLayout->addWidget(nextButton);
        connect(previousButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));
        connect(nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
        connect(doneButton, SIGNAL(clicked()), this, SLOT(onDoneButtonClicked()));
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
        dialogLayout->addLayout(buttonLayout);
    }

    void NewAccountDialog::onDoneButtonClicked()
    {
        if (newService) {
            bool anon = newService->resourceCapabilities(Service::AuthenticationResource).contains("anonymous");
            if (anon) {
                newService->setAnonymous(true);
            }
            serviceManager->addService(newService);
            serviceManager->start(newService);
        }
        accept();
    }

    void NewAccountDialog::onNextButtonClicked()
    {
        page_one.errorLabel->setText(QString());

        if (slideLayout->top() == page_one.widget) {
            // Remove current service, if one exists
            if (newService) {
                newService->deleteLater();
            }

            // Create new service
            newService = new Service(this);
            newService->setUrl(page_one.serviceUrlLineEdit->text());

            // If successful, fill out fields elsewhere
            if (newService->serviceState() == Service::StoppedState) {
                page_two.serviceDescription->setText(newService->description());
                page_three.serviceDescription->setText(newService->description());
            } else {
                page_one.errorLabel->setText("Unable to connect to service.");
                return;
            }

            // Check if we need to choose an authentication method
            QVariantMap methods(newService->supportedAuthenticationMethods());
            if (methods.size() > 0) {
                if (methods.size() == 1 || true) {
                    page_three.authenticationMethod->setText((--methods.end()).key());
                    newService->setAuthenticationMethod((--methods.end()).key());
                    slideLayout->push("3");
                } else {
                    // FIXME should make list
                    slideLayout->push("2");
                }
            } else {
                page_one.errorLabel->setText("Unable to authenticate with service.");
                return;
            }
        } else if (slideLayout->top() == page_two.widget) {
        }

        if (slideLayout->top() == page_three.widget) {
            nextButton->hide();
            doneButton->show();
            doneButton->setDefault(true);
        }
        previousButton->show();
    }

    void NewAccountDialog::onPreviousButtonClicked()
    {
        nextButton->show();
        nextButton->setDefault(true);
        doneButton->hide();
        slideLayout->pop();
        if (slideLayout->top() == page_one.widget) {
            previousButton->hide();
        }
    }




    AccountInfoPanel::AccountInfoPanel(Service * service, QWidget * parent)
        : QWidget(parent), service(service), serviceNameAlternative(false)
    {
        // Get defaults
        QVariantMap defaults(Utopia::defaults());

        serviceInfoLayout = new QGridLayout(this);

        // Account type (hidden until there are more possibilities)
        QLabel * serviceTypeLabel = new QLabel("Account type:");
        serviceInfoLayout->addWidget(serviceTypeLabel, 0, 0, Qt::AlignRight);
        serviceTypeLabel->hide();
        serviceType = new QLabel;
        serviceInfoLayout->addWidget(serviceType, 0, 1, 1, 2);
        serviceType->hide();

        // Service name
        QLabel * serviceNameLabel = new QLabel("Service name:");
        serviceNameLabel->installEventFilter(this);
        serviceInfoLayout->addWidget(serviceNameLabel, 1, 0, Qt::AlignRight);
        serviceName = new QLabel;
        serviceName->installEventFilter(this);
        serviceInfoLayout->addWidget(serviceName, 1, 1, 1, 2);

        // Account description
        serviceInfoLayout->addWidget(new QLabel("Description:"), 2, 0, Qt::AlignRight);
        descriptionLineEdit = new QLineEdit;
        connect(descriptionLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onDescriptionChanged(const QString &)));
        serviceInfoLayout->addWidget(descriptionLineEdit, 2, 1, 1, 2);

        // Credentials
        // The following renaming is a temporary fix for commercial users whose usernames
        // are not email addresses
        userNameLabel = new QLabel(defaults.value("rename_username", "Email address").toString() + ":");
        serviceInfoLayout->addWidget(userNameLabel, 3, 0, Qt::AlignRight);
        userName = new QLineEdit;
        connect(userName, SIGNAL(textEdited(const QString &)), this, SLOT(onUserNameChanged(const QString &)));
        serviceInfoLayout->addWidget(userName, 3, 1);
        registerLabel = new QLabel("<a href='register' style='text-decoration: none'>Register</a>");
        registerLabel->setProperty("class", "link");
        connect(registerLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));
        serviceInfoLayout->addWidget(registerLabel, 3, 2);
        passwordLabel = new QLabel("Password:");
        serviceInfoLayout->addWidget(passwordLabel, 4, 0, Qt::AlignRight);
        password = new QLineEdit;
        password->setEchoMode(QLineEdit::Password);
        serviceInfoLayout->addWidget(password, 4, 1);
        connect(password, SIGNAL(textEdited(const QString &)), this, SLOT(onPasswordChanged(const QString &)));
        resetPasswordLabel = new QLabel("<a href='resetPassword' style='text-decoration: none'>Reset Password</a>");
        resetPasswordLabel->setProperty("class", "link");
        connect(resetPasswordLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));
        serviceInfoLayout->addWidget(resetPasswordLabel, 4, 2);
        anonymousCheckBox = new QCheckBox("Use anonymously");
        connect(anonymousCheckBox, SIGNAL(clicked(bool)), this, SLOT(onAnonymousChanged(bool)));
        serviceInfoLayout->addWidget(anonymousCheckBox, 5, 1, 1, 2, Qt::AlignTop);

        profileButton = new QPushButton("User profile...");
        connect(profileButton, SIGNAL(clicked()), this, SLOT(onProfileButtonClicked()));
        serviceInfoLayout->addWidget(profileButton, 6, 0, 1, 3, Qt::AlignRight);

        // Disclaimer text
        disclaimer = new QLabel;
        disclaimer->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
        disclaimer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        disclaimer->setObjectName("disclaimer");
        serviceInfoLayout->addWidget(disclaimer, 6, 1, 1, 2);

        // Layout
        serviceInfoLayout->setColumnStretch(0, 0);
        serviceInfoLayout->setColumnStretch(1, 1);
        serviceInfoLayout->setRowStretch(0, 0);
        serviceInfoLayout->setRowStretch(1, 0);
        serviceInfoLayout->setRowStretch(2, 0);
        serviceInfoLayout->setRowStretch(3, 0);
        serviceInfoLayout->setRowStretch(4, 0);
        serviceInfoLayout->setRowStretch(5, 1);
        serviceInfoLayout->setRowStretch(6, 1);

        refreshInformation();

        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        connect(service, SIGNAL(serviceStateChanged(Kend::Service::ServiceState)), this, SLOT(onServiceStateChanged(Kend::Service::ServiceState)));

        onServiceStateChanged(service->serviceState());
    }

    bool AccountInfoPanel::apply()
    {
        // Apply service information changes
        if (service) {
            // Apply anonymity
            if (service->isAnonymous() != anonymousCheckBox->isChecked() && anonymousCheckBox->isChecked()) {
                userName->setText(QString());
                password->setText(QString());
            }

            if (service->description() != descriptionLineEdit->text()) {
                service->setDescription(descriptionLineEdit->text());
            }

            QVariantMap credentials(service->credentials());
            bool credentialsModified = false;
            if (credentials.value("password") != password->text()) {
                credentials["password"] = password->text();
                credentialsModified = true;
            }
            if (credentials.value("user") != userName->text()) {
                credentials["user"] = userName->text();
                credentialsModified = true;
            }
            if (credentialsModified) {
                service->setCredentials(credentials);
            }

            if (service->isAnonymous() != anonymousCheckBox->isChecked()) {
                service->setAnonymous(anonymousCheckBox->isChecked());
            } else if (credentialsModified && service->isEnabled() && !service->isAnonymous()) {
                // Reset and start a service in error
                if (service->serviceState() == Service::ErrorState) {
                    service->reset();
                    service->start();
                // Log off a started logged-in service
                } else if (service->serviceState() == Service::StartedState && service->isLoggedIn() && service->logOut()) {
                    loop.exec();
                }
                service->logIn();
            }

            refreshInformation();

            return true;
        }
        return false;
    }

    void AccountInfoPanel::discard()
    {
        // Discard service information changes
        refreshInformation();
    }

    bool AccountInfoPanel::eventFilter(QObject * obj, QEvent * event)
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (obj == serviceName) {
                if (service) {
                    serviceNameAlternative = !serviceNameAlternative;
                    setServiceName();
                }
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }

    void AccountInfoPanel::onAnonymousChanged(bool anonymous)
    {
        userName->setEnabled(!anonymous);
        password->setEnabled(!anonymous);
        userNameLabel->setEnabled(!anonymous);
        passwordLabel->setEnabled(!anonymous);
        emit modified();
    }

    void AccountInfoPanel::onDescriptionChanged(const QString & text)
    {
        emit modified();
    }

    void AccountInfoPanel::onLinkActivated(const QString & href)
    {
        if (href == "register") {
            UserRegistrationDialog dialog;
            dialog.setService(service);
            dialog.exec();
        } else if (href == "resetPassword") {
            ResetPasswordDialog dialog;
            dialog.setService(service);
            dialog.setEmail(userName->text());
            dialog.exec();
        }
    }

    void AccountInfoPanel::onPasswordChanged(const QString & text)
    {
        emit modified();
    }

    void AccountInfoPanel::onProfileButtonClicked()
    {
        emit openProfileDialog(service, service->userURI());
    }

    void AccountInfoPanel::onServiceStateChanged(Service::ServiceState state)
    {
        if (state == Service::StartedState && service->isLoggedIn()) {
            profileButton->setEnabled(true);
            user.reset(service->user());
        } else {
            profileButton->setEnabled(false);
            user.reset();
            emit closeProfileDialog();
        }
        if (state == Service::StartedState || state == Service::ErrorState) {
            loop.quit();
        }
    }

    void AccountInfoPanel::onUserNameChanged(const QString & text)
    {
        emit modified();
    }

    void AccountInfoPanel::refreshInformation()
    {
        if (service) {
            // Service name / URL
            setServiceName();
            // Description (only if changed)
            if (service->description() != descriptionLineEdit->text()) {
                descriptionLineEdit->setText(service->description());
            }

            bool anonymousService = !service->resourceUrl(Service::AuthenticationResource).isValid();
            bool anonymousCapableAccount = service->resourceCapabilities(Service::AuthenticationResource).contains("anonymous");
            bool registerCapableAccount = service->resourceCapabilities(Service::AuthenticationResource).contains("register");

            userName->setVisible(!anonymousService);
            userName->setEnabled(!service->isAnonymous());
            QString newUserNameText = service->isAnonymous() ? QString() : service->credentials().value("user").toString();
            if (userName->text() != newUserNameText) {
                userName->setText(newUserNameText);
            }
            userNameLabel->setVisible(!anonymousService);
            userNameLabel->setEnabled(!service->isAnonymous());

            password->setVisible(!anonymousService);
            password->setEnabled(!service->isAnonymous());
            QString newPasswordText = service->isAnonymous() ? QString() : service->credentials().value("password").toString();
            if (password->text() != newPasswordText) {
                password->setText(newPasswordText);
            }
            passwordLabel->setVisible(!anonymousService);
            passwordLabel->setEnabled(!service->isAnonymous());

            int row, column, rowSpan, columnSpan;
            serviceInfoLayout->getItemPosition(serviceInfoLayout->indexOf(userName), &row, &column, &rowSpan, &columnSpan);
            if (registerCapableAccount && rowSpan == 2) { // shrink line edits and add links
                serviceInfoLayout->removeWidget(userName);
                serviceInfoLayout->addWidget(userName, 3, 1, 1, 1);
                serviceInfoLayout->addWidget(registerLabel, 3, 2, 1, 1);
                registerLabel->show();
                serviceInfoLayout->removeWidget(password);
                serviceInfoLayout->addWidget(password, 4, 1, 1, 1);
                serviceInfoLayout->addWidget(resetPasswordLabel, 4, 2, 1, 1);
                resetPasswordLabel->hide();
            } else if (!registerCapableAccount && rowSpan == 1) { // expand line edits and remove links
                serviceInfoLayout->removeWidget(registerLabel);
                serviceInfoLayout->removeWidget(userName);
                serviceInfoLayout->addWidget(userName, 3, 1, 1, 2);
                registerLabel->hide();
                serviceInfoLayout->removeWidget(resetPasswordLabel);
                serviceInfoLayout->removeWidget(password);
                serviceInfoLayout->addWidget(password, 4, 1, 1, 2);
                resetPasswordLabel->hide();
            }

            anonymousCheckBox->setVisible(!anonymousService && anonymousCapableAccount);
            anonymousCheckBox->setChecked(service->isAnonymous());

            if (anonymousService) {
                disclaimer->setText("This is an anonymous service.");
                disclaimer->show();
            } else if (!anonymousCapableAccount) {
                disclaimer->setText("You must log in to use this service.");
                disclaimer->show();
            } else {
                disclaimer->hide();
            }
        }
    }

    void AccountInfoPanel::setServiceName()
    {
        if (serviceNameAlternative) {
            serviceName->setText(service->url().toString());
        } else {
            serviceName->setText(service->serviceName());
        }
    }




    AccountDelegate::AccountDelegate(QObject * parent)
        : QStyledItemDelegate(parent)
    {}

    static int row_height(const QStyleOptionViewItem & option)
    {
        return 5 * option.fontMetrics.height() / 2;
    }

    void AccountDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        painter->save();

        if (/* const QStyleOptionViewItemV4 * itemOptionV4 = */ qstyleoption_cast< const QStyleOptionViewItemV4 * >(&option)) {
		    // Collect option information
            const QStyleOptionViewItemV3 * optionV3 = qstyleoption_cast< const QStyleOptionViewItemV3 * >(&option);
            const QWidget * widget = optionV3 ? optionV3->widget : 0;
            QStyle * style = widget ? widget->style() : QApplication::style();
			QStyleOptionViewItemV4 opt(option);
            initStyleOption(&opt, index);

            // Draw standard background
            style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

            QPalette::ColorGroup group = (option.state & QStyle::State_Active) ? QPalette::Active : QPalette::Inactive;
            if (option.state & QStyle::State_Selected) {
                painter->setPen(option.palette.color(group, QPalette::HighlightedText));
            } else {
                painter->setPen(index.data(Qt::ForegroundRole).value< QBrush >().color());
            }

            // Get textual information
            QString description(index.data(Qt::DisplayRole).toString());
            QString username = "Disabled";
            if (static_cast< Qt::CheckState >(index.sibling(index.row(), 0).data(Qt::CheckStateRole).toInt()) == Qt::Checked) {
                username = index.sibling(index.row(), index.column() + 1).data(Qt::DisplayRole).toString();
            }

            // Draw textual information
            QFont smaller(option.font);
            smaller.setPointSizeF(smaller.pointSizeF() * 0.85);
            smaller.setItalic(true);
            QFontMetrics smallerMetrics(smaller);
            int height = row_height(option);
            int divider = height / 2 + (option.fontMetrics.height() - smallerMetrics.height()) / 2;
            painter->setBrush(Qt::NoBrush);

            QRect descriptionRect = option.rect;
            int flags = Qt::AlignLeft | Qt::AlignVCenter;
            if (!username.isEmpty()) {
                painter->setFont(smaller);
                painter->drawText(option.rect.adjusted(0, divider, 0, 0), (Qt::AlignLeft | Qt::AlignTop), username);
                descriptionRect = descriptionRect.adjusted(0, 0, 0, -divider);
                painter->setFont(option.font);
                flags &= ~Qt::AlignVCenter;
                flags |= Qt::AlignBottom;
            }
            painter->drawText(descriptionRect, flags, description);
        }

        painter->restore();
    }

    QSize AccountDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        return QSize(0, row_height(option));
    }




    AccountsPreferencesPane::AccountsPreferencesPane(QWidget * parent, Qt::WindowFlags f)
        : Utopia::PreferencesPane(parent, f)
    {
        serviceManager = ServiceManager::instance();
        connect(serviceManager.get(), SIGNAL(serviceAdded(Kend::Service*)), this, SLOT(onServiceAdded(Kend::Service*)));
        connect(serviceManager.get(), SIGNAL(serviceRemoved(Kend::Service*)), this, SLOT(onServiceRemoved(Kend::Service*)));
        serviceManagerModel = new ServiceManagerModel(this);
        QVBoxLayout * dialogLayout = new QVBoxLayout(this);
        dialogLayout->setSpacing(8);
        tableView = new QTableView;
        tableView->setModel(serviceManagerModel);
        tableView->setItemDelegateForColumn(1, new AccountDelegate(this));
        tableView->setFocusPolicy(Qt::NoFocus);
        tableView->horizontalHeader()->hide();
        tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        tableView->horizontalHeader()->setSectionHidden(2, true);
        tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        tableView->horizontalHeader()->resizeSection(3, 100);
        tableView->verticalHeader()->hide();
        tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setShowGrid(false);
        tableView->setWordWrap(false);
        connect(tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(onCurrentRowChanged(const QModelIndex &, const QModelIndex &)), Qt::QueuedConnection);
        dialogLayout->addWidget(tableView, 1);

        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(0);
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        addServiceButton = new QToolButton;
        addServiceButton->setIcon(QIcon(":/icons/addaccount.png"));
        addServiceButton->setObjectName("addServiceButton");
        connect(addServiceButton, SIGNAL(clicked()), this, SLOT(onAddServiceButtonClicked()));
        removeServiceButton = new QToolButton;
        removeServiceButton->setIcon(QIcon(":/icons/removeaccount.png"));
        removeServiceButton->setObjectName("removeServiceButton");
        removeServiceButton->setEnabled(false);
        connect(removeServiceButton, SIGNAL(clicked()), this, SLOT(onRemoveServiceButtonClicked()));
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(addServiceButton);
        buttonLayout->addWidget(removeServiceButton);
        dialogLayout->addLayout(buttonLayout, 0);

        detailsGroup = new QGroupBox("Account details");
        dialogLayout->addWidget(detailsGroup, 0);
        detailsGroup->hide();
        infoStack = new QStackedLayout(detailsGroup);

        if (serviceManager->count() > 0) {
            tableView->setCurrentIndex(serviceManagerModel->index(0, 0));
        }

        userInfoEditor = new UserInfoEditor(this);
        userInfoEditor->hide();

        // Populate info panels from service manager
        for (int i = 0; i < serviceManager->count(); ++i) {
            Service * service = serviceManager->serviceAt(i);
            onServiceAdded(service);
        }
    }

    bool AccountsPreferencesPane::apply()
    {
        bool success = true;
        for (int i = 0; i < infoStack->count(); ++i) {
            if (AccountInfoPanel * panel = qobject_cast< AccountInfoPanel * >(infoStack->widget(i))) {
                success = success && panel->apply();
            }
        }
        return success;
    }

    void AccountsPreferencesPane::discard()
    {
        for (int i = 0; i < infoStack->count(); ++i) {
            if (AccountInfoPanel * panel = qobject_cast< AccountInfoPanel * >(infoStack->widget(i))) {
                panel->discard();
            }
        }
    }

    QIcon AccountsPreferencesPane::icon() const
    {
        return QIcon(":/icons/accounts.png");
    }

    bool AccountsPreferencesPane::isModified() const
    {
        // FIXME include whether identity pane has changed
        return PreferencesPane::isModified();
    }

    bool AccountsPreferencesPane::isValid() const
    {
        QVariantMap defaults(Utopia::defaults());
        QString method = defaults.value("service_method").toString();
        return (method != "prevent");
    }

    void AccountsPreferencesPane::onAccountInfoPanelModified()
    {
        setModified(true);
    }

    void AccountsPreferencesPane::onAddServiceButtonClicked()
    {
        // FIXME we should here provide a nice dialog for choosing an appropriate service

        // However, for now, create only that which is allowed
        QVariantMap defaults(Utopia::defaults());
        if (defaults.contains("service_uri")) {
            QString serviceUri = defaults["service_uri"].toString();
            Service * newService = new Service;
            newService->setUrl(serviceUri);
            QVariantMap methods(newService->supportedAuthenticationMethods());
            if (methods.size() > 0) {
                if (methods.size() == 1 || true) {
                    newService->setAuthenticationMethod((--methods.end()).key());
                    bool anon = newService->resourceCapabilities(Service::AuthenticationResource).contains("anonymous");
                    if (anon) {
                        newService->setAnonymous(true);
                    }
                    serviceManager->addService(newService);
                    serviceManager->start(newService);
                } else {
                    // FIXME should make list
                }
            } else {
                delete newService;
            }
        } else {
            NewAccountDialog dialog;
            dialog.exec();
        }
    }

    void AccountsPreferencesPane::onCloseProfileDialog()
    {
        userInfoEditor->hide();
        userInfoEditor->clear();
    }

    void AccountsPreferencesPane::onCurrentRowChanged(const QModelIndex & current, const QModelIndex & /*previous*/)
    {
        int row = current.row();
        if (row >= 0 && row < serviceManager->count()) {
            selectedService = serviceManager->serviceAt(row);
            for (int i = 0; i < infoStack->count(); ++i) {
                if (AccountInfoPanel * panel = qobject_cast< AccountInfoPanel * >(infoStack->widget(i))) {
                    if (panel->service == selectedService) {
                        infoStack->setCurrentWidget(panel);
                    }
                }
            }
            detailsGroup->show();
        } else {
            selectedService = 0;
            detailsGroup->hide();
        }
    }

    void AccountsPreferencesPane::onOpenProfileDialog(Service * service, const QString & id)
    {
        userInfoEditor->setUser(service, id);
        userInfoEditor->exec();
    }

    void AccountsPreferencesPane::onRemoveServiceButtonClicked()
    {
        if (selectedService && QMessageBox::question(this, "Remove account...", "Are you sure you wish to remove this account and all its associated information?", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) == QMessageBox::Yes) {
            serviceManager->removeService(selectedService);
        }
    }

    void AccountsPreferencesPane::onServiceAdded(Service * service)
    {
        AccountInfoPanel * panel = new AccountInfoPanel(service);
        connect(panel, SIGNAL(openProfileDialog(Service *, const QString &)), this, SLOT(onOpenProfileDialog(Service *, const QString &)));
        connect(panel, SIGNAL(closeProfileDialog()), this, SLOT(onCloseProfileDialog()));
        connect(panel, SIGNAL(modified()), this, SLOT(onAccountInfoPanelModified()));
        infoStack->addWidget(panel);
        if (serviceManager->count() == 1) {
            tableView->setCurrentIndex(serviceManagerModel->index(0, 0));
        }
        removeServiceButton->setEnabled(true);
        addServiceButton->setEnabled(false); // FIXME currently only one service allowed
    }

    void AccountsPreferencesPane::onServiceRemoved(Service * service)
    {
        for (int i = 0; i < infoStack->count(); ++i) {
            if (AccountInfoPanel * panel = qobject_cast< AccountInfoPanel * >(infoStack->widget(i))) {
                if (panel->service == service) {
                    delete panel;
                }
            }
        }
        if (serviceManager->count() == 0) {
            removeServiceButton->setEnabled(false);
            addServiceButton->setEnabled(true); // FIXME currently this button can be greyed out (only one service allowed)
        }
    }

    QString AccountsPreferencesPane::title() const
    {
        return "Accounts";
    }

    int AccountsPreferencesPane::weight() const
    {
        return 1000;
    }

}
