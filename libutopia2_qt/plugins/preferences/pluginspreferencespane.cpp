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

#include "pluginspreferencespane_p.h"
#include "pluginspreferencespane.h"

#include <utopia2/configuration.h>
#include <utopia2/qt/webview.h>
#include <utopia2/qt/configurator.h>

#include <QDesktopServices>
#include <QFile>
#include <QHBoxLayout>
#include <QListWidget>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QWebInspector>

#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Q_DECLARE_METATYPE(Utopia::Configurator *)

namespace
{

    static QString serialise(const QVariant & value, const QString & type)
    {
        if (type == "null") { // FIXME other stuff here
            return QString();
        } else {
            return value.toString();
        }
    }

    static QVariant parse(const QString & value, const QString & type)
    {
        if (type == "null") { // FIXME other stuff here
            return QVariant();
        } else {
            return value;
        }
    }




    class PluginDelegate : public QAbstractItemDelegate
    {
    public:
        PluginDelegate(QObject * parent = 0)
            : QAbstractItemDelegate(parent)
        {}

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
        {
            painter->save();
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            if (option.state & QStyle::State_Selected) {
                painter->setBrush(option.palette.highlight());
                painter->setPen(Qt::NoPen);
                painter->drawRect(option.rect);
                painter->setPen(option.palette.highlightedText().color());
            }
            if (Utopia::Configurator * configurator = index.data(Qt::UserRole).value< Utopia::Configurator * >()) {
                QRect rect(option.rect.adjusted(6, 6, -6, -6));
                QImage icon(configurator->icon());
                QString title(configurator->title());
                QFontMetrics fm(option.font);
                QRect textRect(rect.left(), rect.height(), rect.width(), fm.height() * 2 + fm.leading());
                QRect textBounds(fm.boundingRect(textRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, title));
                textRect.setHeight(qMin(textBounds.height(), textRect.height()));
                textRect.moveBottom(rect.bottom());
                QSize iconSize(icon.size());
                iconSize.scale(rect.width(), rect.height() - textRect.height(), Qt::KeepAspectRatio);
                QRect iconRect(rect.left() + (rect.width() - iconSize.width()) / 2, rect.top(), iconSize.width(), iconSize.height());
                painter->drawImage(iconRect, icon);
                QTextOption textOption(Qt::AlignCenter);
                textOption.setWrapMode(QTextOption::WordWrap);
                painter->setBrush(Qt::NoBrush);
                painter->drawText(textRect, title, textOption);
            }
            painter->restore();
        }

        QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
        {
            return QSize(100, 100);
        }

    };

}




ConfiguratorControl::ConfiguratorControl(Utopia::Configurator * configurator,
                                         QObject * parent)
    : QObject(parent), configurator(configurator), blockSignals(false)
{
    connect(configurator->configuration(), SIGNAL(configurationChanged(const QString &)),
            this, SLOT(onConfigurationChanged(const QString &)));

    // Each configurator needs a web page for displaying its UI
    page = new Utopia::WebPage(this);
    connect(page, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onWebPageLinkClicked(const QUrl &)));
    connect(page, SIGNAL(contentsChanged()), this, SLOT(onWebPageContentsChanged()));

    // Load in the template HTML
    QFile htmlTemplate(":/preferences/plugins/form.html");
    htmlTemplate.open(QIODevice::ReadOnly);
    QString html = QString::fromUtf8(htmlTemplate.readAll()).arg(configurator->form());
    page->mainFrame()->setContent(html.toUtf8(), "text/html");
    page->mainFrame()->addToJavaScriptWindowObject("control", this);
    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
}

void ConfiguratorControl::onConfigurationChanged(const QString & key)
{
    if (!blockSignals) {
        QString query = QString("input[name='%1'], textarea[name='%1']").arg(key);
        QWebFrame * frame = page->mainFrame();
        foreach (QWebElement inputElement, frame->findAllElements(query)) {
            revert(inputElement);
        }
    }
}

void ConfiguratorControl::onWebPageContentsChanged()
{
    if (!blockSignals) {
        emit contentsChanged();
    }
}

void ConfiguratorControl::onWebPageLinkClicked(const QUrl & url)
{
    QDesktopServices::openUrl(url);
}

void ConfiguratorControl::revert()
{
    QWebFrame * frame = page->mainFrame();
    foreach (QWebElement inputElement, frame->findAllElements("input, textarea")) {
        revert(inputElement);
    }
}

void ConfiguratorControl::revert(QWebElement & inputElement)
{
    blockSignals = true;
    Utopia::Configuration * configuration = configurator->configuration();
    QString name = inputElement.attribute("name");
    if (configuration->contains(name)) {
        QString type = inputElement.attribute("type");
        if (type == "checkbox") {
            inputElement.evaluateJavaScript(QString("this.checked = %1; $(this).change()").arg(configuration->get(name).toBool() ? "true" : "false"));
        } else if (type == "radio") {
            QString value = inputElement.attribute("value");
            inputElement.evaluateJavaScript(QString("this.checked = %1; $(this).change()").arg(configuration->get(name).toString() == value ? "true" : "false"));
        } else {
            QString value(serialise(configuration->get(name), type).replace("\\", "\\\\").replace("'", "\\'"));
            inputElement.evaluateJavaScript(QString("this.value = '%1'; $(this).change()").arg(value));
        }
    }
    blockSignals = false;
}

void ConfiguratorControl::save()
{
    blockSignals = true;
    Utopia::Configuration * configuration = configurator->configuration();
    QWebFrame * frame = page->mainFrame();
    foreach (QWebElement inputElement, frame->findAllElements("input, textarea")) {
        QVariant value;
        QString name = inputElement.attribute("name");
        QString type = inputElement.attribute("type");
        if (type == "checkbox") {
            value = inputElement.evaluateJavaScript("this.checked");
        } else if (type == "radio") {
            if (inputElement.evaluateJavaScript("this.checked").toBool()) {
                value = inputElement.attribute("value");
            }
            continue; // Don't set value, move onto the next element
        } else {
            value = parse(inputElement.evaluateJavaScript("this.value").toString(), type);
        }
        configuration->set(name, value);
    }
    blockSignals = false;
}




PluginsPreferencesPanePrivate::PluginsPreferencesPanePrivate(QObject * parent)
    : QObject(parent)
{}




PluginsPreferencesPane::PluginsPreferencesPane(QWidget * parent, Qt::WindowFlags f)
    : Utopia::PreferencesPane(parent, f), d(new PluginsPreferencesPanePrivate)
{
    // Load instances of all configurators
    foreach (Utopia::Configurator * configurator, Utopia::instantiateAllExtensionsOnce< Utopia::Configurator >()) {
        ConfiguratorControl * control = new ConfiguratorControl(configurator, d);
        connect(control, SIGNAL(contentsChanged()), this, SLOT(onContentsChanged()));

        // Ensure the background of the web page matches the widget's background
        QString bgcolor = palette().color(QWidget::backgroundRole()).name();
        QString css = QString("html { background-color: %1 !important; }").arg(bgcolor);
        control->page->settings()->setUserStyleSheetUrl(QUrl("data:text/css;charset=utf-8;base64," + css.toUtf8().toBase64()));
        control->page->setPalette(palette());

        // Insert it into the list in the correct order (case-insensitive alphabetical)
        int idx = 0;
        foreach (const ConfiguratorControl * candidate, d->configurators) {
            QString other = candidate->configurator->title().toLower();
            if (other < configurator->title().toLower()) { // HACK less-than makes it reverse order, so Lazarus appears first
                break;
            }
            ++idx;
        }
        d->configurators.insert(idx, control);
    }

    // Layout widget
    QHBoxLayout * layout = new QHBoxLayout(this);
    d->listWidget = new QListWidget;
    d->listWidget->setFixedWidth(120);
    d->listWidget->setResizeMode(QListWidget::Fixed);
    d->listWidget->setItemDelegate(new PluginDelegate);
    connect(d->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onListWidgetCurrentRowChanged(int)));
    layout->addWidget(d->listWidget, 0);
    d->webView = new Utopia::WebView;
    layout->addWidget(d->webView, 1);

    // Populate
    foreach (const ConfiguratorControl * control, d->configurators) {
        Utopia::Configurator * configurator = control->configurator;
        QString title(configurator->title());
        if (title.isEmpty()) {
            title = "Untitled Plugin";
        }
        QListWidgetItem * item = new QListWidgetItem(title);
        item->setSizeHint(QSize(0, 100));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        item->setData(Qt::UserRole, QVariant::fromValue< Utopia::Configurator * >(configurator));

        d->listWidget->addItem(item);
    }

    // Highlight first
    if (d->listWidget->count() > 0) {
        d->listWidget->setCurrentRow(0);
    }
}

bool PluginsPreferencesPane::apply()
{
    foreach (ConfiguratorControl * control, d->configurators) {
        control->save();
    }
    return true;
}

void PluginsPreferencesPane::discard()
{
    foreach (ConfiguratorControl * control, d->configurators) {
        control->revert();
    }
}

QIcon PluginsPreferencesPane::icon() const
{
    return QIcon(":/preferences/plugins/icon.png");
}

bool PluginsPreferencesPane::isValid() const
{
    return d->listWidget->count() > 0;
}

void PluginsPreferencesPane::onContentsChanged()
{
    // Inform the dialog that this pane has been modified
    setModified(true);
    emit modifiedChanged(true);
}

void PluginsPreferencesPane::onListWidgetCurrentRowChanged(int newRow)
{
    if (newRow < 0) {
        if (d->listWidget->count() > 0) {
            d->listWidget->setCurrentRow(0);
        }
    } else if (newRow < d->configurators.size()) {
        // Check current plugin configuration to see if it has changed
        ConfiguratorControl * control = d->configurators.at(newRow);
        d->webView->setPage(control->page);
    }
}

void PluginsPreferencesPane::show(const QVariant & params)
{
    QString uuid = params.toMap().value("uuid").toString().toLower();
    int idx = 0;
    foreach (const ConfiguratorControl * control, d->configurators) {
        if (control->configurator->configurationId() == QUuid(uuid)) {
            d->listWidget->setCurrentRow(idx);
            break;
        }
        ++idx;
    }
}

QString PluginsPreferencesPane::title() const
{
    return "Plugins";
}

int PluginsPreferencesPane::weight() const
{
    return -10;
}
