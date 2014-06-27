/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include "pluginspreferencespane.h"

#include <utopia2/qt/webview.h>
#include <utopia2/configuration.h>

#include <QDesktopServices>
#include <QFile>
#include <QHBoxLayout>
#include <QListWidget>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QWebPage>
#include <QWebView>

#include <QDebug>

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
        PluginDelegate(PluginsPreferencesPane * pane, QObject * parent = 0)
            : QAbstractItemDelegate(parent), pane(pane)
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
            if (Utopia::Configurator * configurator = pane->configuratorAt(index.row())) {
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

    protected:
        PluginsPreferencesPane * pane;
    };

}


PluginsPreferencesPane::PluginsPreferencesPane(QWidget * parent, Qt::WindowFlags f)
    : Utopia::PreferencesPane(parent, f), _previousPluginIndex(-1)
{
    // Load user CSS in order to override background colour
    QFile cssTemplate(":/preferences/plugins/form.css");
    cssTemplate.open(QIODevice::ReadOnly);
    QString css(QString::fromUtf8(cssTemplate.readAll()));
    css = css.arg(
        palette().window().color().name(),
        QString("%1").arg(font().pointSizeF() * 72 / 96.0)
    );
    QUrl cssUrl("data:text/css;charset=utf-8;base64," + css.toUtf8().toBase64());

    // Load configurators
    foreach (Utopia::Configurator * configurator, Utopia::instantiateAllExtensionsOnce< Utopia::Configurator >()) {
        QWebPage * page = new QWebPage(this);
        page->settings()->setUserStyleSheetUrl(cssUrl);
        page->mainFrame()->setContent(configurator->form().toUtf8());
        page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
        connect(page, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onWebPageLinkClicked(const QUrl &)));
        connect(page, SIGNAL(contentsChanged()), this, SLOT(onWebPageContentsChanged()));

        _configurators.append(qMakePair(configurator, page));
    }

    // Layout widget
    QHBoxLayout * layout = new QHBoxLayout(this);
    _listWidget = new QListWidget;
    _listWidget->setFixedWidth(120);
    _listWidget->setResizeMode(QListWidget::Fixed);
    _listWidget->setItemDelegate(new PluginDelegate(this));
    connect(_listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onListWidgetCurrentRowChanged(int)));
    layout->addWidget(_listWidget, 0);
    _webView = new Utopia::WebView;
    layout->addWidget(_webView, 1);

    // Populate
    QListIterator< QPair< Utopia::Configurator *, QWebPage * > > iter(_configurators);
    while (iter.hasNext()) {
        Utopia::Configurator * configurator = iter.next().first;
        QString title(configurator->title());
        if (title.isEmpty()) {
            title = "Untitled Plugin";
        }
        QListWidgetItem * item = new QListWidgetItem(title);
        item->setSizeHint(QSize(0, 100));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        _listWidget->addItem(item);
    }

    // Highlight first
    if (_listWidget->count() > 0) {
        _listWidget->setCurrentRow(0);
    }
}

bool PluginsPreferencesPane::apply()
{
    save(_listWidget->currentRow());
    return true;
}

Utopia::Configurator * PluginsPreferencesPane::configuratorAt(int idx)
{
    return _configurators.at(idx).first;
}

void PluginsPreferencesPane::discard()
{
    load(_listWidget->currentRow());
}

QIcon PluginsPreferencesPane::icon() const
{
    return QIcon(":/preferences/plugins/icon.png");
}

bool PluginsPreferencesPane::isValid() const
{
    return _listWidget->count() > 0;
}

void PluginsPreferencesPane::load(int idx)
{
    Utopia::Configuration * configuration = _configurators.at(idx).first->configuration();
    QWebPage * page = _configurators.at(idx).second;
    QWebFrame * frame = page->mainFrame();

    // Deal with simple input elements
    foreach (QWebElement inputElement, frame->findAllElements("input, textarea")) {
        QString name = inputElement.attribute("name");
        QString type = inputElement.attribute("type");
        if (configuration->contains(name)) {
            if (type == "checkbox") {
                inputElement.evaluateJavaScript(QString("this.checked = %1").arg(configuration->get(name).toBool() ? "true" : "false"));
            } else if (type == "radio") {
                QString value = inputElement.attribute("value");
                inputElement.evaluateJavaScript(QString("this.checked = %1").arg(configuration->get(name).toString() == value ? "true" : "false"));
            } else {
                QString value(serialise(configuration->get(name), type).replace("'", "\'"));
                inputElement.evaluateJavaScript(QString("this.value = '%1'").arg(value));
            }
        }
    }

    // Due to a problem disconnecting signals from old pages, we cannot do the following:
    //    _webView->setPage(page);
    // We must instead manually remove the view from the page. This Qt bug is reportedly
    // still present in 4.8. https://bugs.webkit.org/show_bug.cgi?id=49215
    if (_webView->page()) {
        _webView->page()->setView(0);
    }
    _webView->setPage(page);
    _previousPluginIndex = idx;
}

void PluginsPreferencesPane::onListWidgetCurrentRowChanged(int newRow)
{
    if (newRow < 0) {
        if (_listWidget->count() > 0) {
            _listWidget->setCurrentRow(0);
        }
    } else if (newRow < _configurators.size()) {
        // Check current plugin configuration to see if it has changed
        load(newRow);
    }
}

void PluginsPreferencesPane::onWebPageContentsChanged()
{
    QWebPage * page = qobject_cast< QWebPage * >(sender());
    // Inform the dialog that this pane has been modified
    setModified(true);
    emit modifiedChanged(true);
}

void PluginsPreferencesPane::onWebPageLinkClicked(const QUrl & url)
{
    QDesktopServices::openUrl(url);
}

void PluginsPreferencesPane::save(int idx)
{
    Utopia::Configuration * configuration = _configurators.at(idx).first->configuration();
    QWebPage * page = _configurators.at(idx).second;
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
}

QString PluginsPreferencesPane::title() const
{
    return "Plugins";
}

int PluginsPreferencesPane::weight() const
{
    return -10;
}
