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

#include "cslengineprefspane.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <QSpinBox>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>

CSLEnginePreferencesPane::CSLEnginePreferencesPane(QWidget * parent, Qt::WindowFlags f)
    : Utopia::PreferencesPane(parent, f), cslengine(Papyro::CSLEngine::instance())
{
    QVBoxLayout * dialogLayout = new QVBoxLayout(this);
    QGroupBox * cslGroupBox = new QGroupBox("Preferred citation style");
    dialogLayout->addWidget(cslGroupBox);
    QVBoxLayout * optionsLayout = new QVBoxLayout(cslGroupBox);
    optionsLayout->setSpacing(8);

    stylePreviewLabel = new QLabel;
    stylePreviewLabel->setWordWrap(true);
    stylePreviewLabel->setObjectName("csl-preview");

    styleComboBox = new QComboBox;
    QVariantMap styles = cslengine->availableStyles();
    QMapIterator< QString, QVariant > iter(styles);
    while (iter.hasNext()) {
        iter.next();
        styleComboBox->addItem(iter.value().toString(), iter.key());
    }

    optionsLayout->addWidget(styleComboBox);
    optionsLayout->addWidget(new QLabel("Example:"));
    optionsLayout->addWidget(stylePreviewLabel);

    connect(styleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDefaultStyleChanged(int)));

    // Info about adding own styles
    QString userStylePath = QDir(Utopia::profile_path() + "/csl").absolutePath();

    QGroupBox * userCslGroupBox = new QGroupBox("Using your own CSL styles");
    dialogLayout->addWidget(userCslGroupBox);

    QVBoxLayout * userCslGroupBoxLayout = new QVBoxLayout(userCslGroupBox);
    userCslGroupBoxLayout->setSpacing(8);

    QLabel * userCslInfoLabel = new QLabel;
    userCslInfoLabel->setWordWrap(true);
    userCslInfoLabel->setText("<p>If you wish to use your own citation styles, you can "
                              "put CSL style files (*.csl) into the following directory "
                              "and restart Utopia Documents.</p>");
    userCslGroupBoxLayout->addWidget(userCslInfoLabel);
    userCslGroupBoxLayout->addWidget(new QLabel("<em>" + userStylePath + "/</em>"));
    QPushButton * openCslPathButton = new QPushButton("Open Folder...");
    connect(openCslPathButton, SIGNAL(pressed()), this, SLOT(onOpenCslPathButtonPressed()));
    userCslGroupBoxLayout->addWidget(openCslPathButton, 0, Qt::AlignLeft);

    dialogLayout->addStretch(1);

    load();

    updateExample();
}

bool CSLEnginePreferencesPane::apply()
{
    save();
    return true;
}

void CSLEnginePreferencesPane::discard()
{
    load();
}

QIcon CSLEnginePreferencesPane::icon() const
{
    return QIcon(":/preferences/citations/icon.png");
}

void CSLEnginePreferencesPane::load()
{
    // Populate from settings
    QSettings conf;
    conf.sync();
    conf.beginGroup("CSLEngine");

    QString defaultStyle = conf.value("Default Style", "apa").toString();
    config["Default Style"] = defaultStyle;

    // Update the drop-down
    for (int i = 0; i < styleComboBox->count(); ++i) {
        if (styleComboBox->itemData(i).toString() == defaultStyle) {
            styleComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void CSLEnginePreferencesPane::onDefaultStyleChanged(int index)
{
    QString defaultStyle(styleComboBox->itemData(index).toString());
    setValue("Default Style", defaultStyle);
    updateExample();
}

void CSLEnginePreferencesPane::onOpenCslPathButtonPressed()
{
    QString userStylePath = QDir(Utopia::profile_path() + "/csl").absolutePath();
    QDir dir(userStylePath);
    if (!dir.exists()) {
        // Make
        dir.cdUp();
        dir.mkdir("csl");
    }
    QUrl url = QUrl::fromLocalFile(userStylePath);
    QDesktopServices::openUrl(url);
}

void CSLEnginePreferencesPane::save()
{
    // Save to settings
    QSettings conf;
    conf.sync();
    conf.beginGroup("CSLEngine");

    QString defaultStyle(config.value("Default Style").toString());
    conf.setValue("Default Style", defaultStyle);
    cslengine->setDefaultStyle(defaultStyle);
}

void CSLEnginePreferencesPane::setValue(const QString & key, const QVariant & value)
{
    if (config[key] != value) {
        config[key] = value;
        setModified(true);
    }
}

QString CSLEnginePreferencesPane::title() const
{
    return "Citations";
}

void CSLEnginePreferencesPane::updateExample()
{
    int index = styleComboBox->currentIndex();
    QString defaultStyle(styleComboBox->itemData(index).toString());

    // Recreate label
    QVariantMap metadata;
    {
        QVariantList authors;
        {
            QVariantMap author;
            author["family"] = "Attwood";
            author["given"] = "Teresa";
            authors.push_back(author);
        }
        {
            QVariantMap author;
            author["family"] = "Kell";
            author["given"] = "Douglas";
            authors.push_back(author);
        }
        {
            QVariantMap author;
            author["family"] = "McDermott";
            author["given"] = "Philip";
            authors.push_back(author);
        }
        {
            QVariantMap author;
            author["family"] = "Marsh";
            author["given"] = "James";
            authors.push_back(author);
        }
        {
            QVariantMap author;
            author["family"] = "Pettifer";
            author["given"] = "Steve";
            authors.push_back(author);
        }
        {
            QVariantMap author;
            author["family"] = "Thorne";
            author["given"] = "Dave";
            authors.push_back(author);
        }
        metadata["author"] = authors;
    }
    metadata["title"] = "Calling International Rescue: knowledge lost in literature and data landslide!";
    metadata["volume"] = "424";
    metadata["issue"] = "3";
    metadata["container-title"] = "Biochem J";
    metadata["publisher"] = "Portland Press";
    {
        QVariantMap issued;
        QVariantList parts;
        QVariantList from;
        from << "2009" << "12" << "10";
        parts.push_back(from);
        issued["date-parts"] = parts;
        metadata["issued"] = issued;
    }
    metadata["DOI"] = "10.1042/BJ20091474";
    metadata["PMID"] = "19929850";
    metadata["PMCID"] = "PMC2805925";
    metadata["page"] = "317-333";
    metadata["citation-label"] = "1";

    QString csl = cslengine->format(metadata, defaultStyle);
    stylePreviewLabel->setText(csl);
}

int CSLEnginePreferencesPane::weight() const
{
    return 0;
}
