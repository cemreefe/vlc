/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "firstrunwizard.hpp"
#include "util/color_scheme_model.hpp"
#include "maininterface/mainctx.hpp"
#include "dialogs/toolbar/controlbar_profile_model.hpp"
#include "dialogs/dialogs_provider.hpp"
#include "medialibrary/medialib.hpp"
#include "widgets/native/vlcdarkstyle.hpp"

#include <QPushButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDir>

#include <vlc_common.h>
#include <vlc_configuration.h>
#include <vlc_url.h>
#include <vlc_cxx_helpers.hpp>

FirstRunWizard::FirstRunWizard( qt_intf_t *_p_intf, QWidget *parent)
               : QWizard( parent )
{
    p_intf = _p_intf;

    setWindowTitle( qtr( "Welcome to VLC" ) );
    setWindowModality( Qt::WindowModal );
    setWizardStyle(QWizard::ClassicStyle);
    ui.setupUi( this );
    setFixedSize(760, 520);
    const QString accent = vlcWidgetAccentColor(VLC_OBJECT(p_intf));
    setStyleSheet(vlcDarkWidgetStyle(accent) + QStringLiteral(
        "QLabel#shortcutKeycap { background: #2B2927; border: 1px solid #3A3836; border-radius: 5px; padding: 6px 10px; color: #ECECEC; font-family: monospace; font-size: 12px; }"
        "QLabel#shortcutAction { color: #D0D0D0; font-size: 13px; }"
        "QFrame#shortcutCard { background: #1E1D1C; border: 1px solid #2B2927; border-radius: 7px; }"
        "QToolButton#themeCard:checked { border: 2px solid %1; border-radius: 8px; }"
        "QToolButton#detailsButton { background: transparent; border: none; color: %1; padding: 0; }"
    ).arg(accent));

    ui.welcomePage->setTitle(QString());
    ui.welcomePage->setSubTitle(QString());
    ui.colorSchemePage->setTitle(QString());
    ui.layoutPage->setTitle(QString());
    ui.layoutPage->setSubTitle(QString());
    setStartId(WELCOME_PAGE);
    setButtonText(QWizard::BackButton, qtr("Back"));
    setButtonText(QWizard::NextButton, qtr("Continue"));
    setButtonText(QWizard::FinishButton, qtr("Done"));

    ui.policy->setHtml( qtr( "<p>In order to protect your privacy, <i>VLC media player</i> "
        "does <b>not</b> collect personal data or transmit them, "
        "not even in anonymized form, to anyone."
        "</p>\n"
        "<p>Nevertheless, <i>VLC</i> is able to automatically retrieve "
        "information about the media in your play queue from third party "
        "Internet-based services. This includes cover art, track names, "
        "artist names and other meta-data."
        "</p>\n"
        "<p>Consequently, this may entail identifying some of your media files to third party "
        "entities. Therefore the <i>VLC</i> developers require your express "
        "consent for the media player to access the Internet automatically."
        "</p>\n" ) );
    ui.policy->setReadOnly( true );
    ui.policy->hide();
    ui.horizontalFrame_policy->hide();
    ui.enableMl->hide();
    ui.horizontalFrame_prefs->hide();

    delete ui.welcomePage->layout();
    auto *welcomePageLayout = new QVBoxLayout(ui.welcomePage);
    welcomePageLayout->setContentsMargins(0, 8, 0, 8);
    auto *welcomeContent = new QWidget(ui.welcomePage);
    welcomeContent->setMaximumWidth(420);
    auto *welcomeLayout = new QVBoxLayout(welcomeContent);
    welcomeLayout->setContentsMargins(0, 0, 0, 0);
    welcomeLayout->setSpacing(10);
    auto *cone = new QLabel(welcomeContent);
    cone->setPixmap(QPixmap(":/logo/vlc128.png"));
    cone->setFixedSize(96, 96);
    cone->setScaledContents(true);
    welcomeLayout->addWidget(cone, 0, Qt::AlignHCenter);
    auto *welcomeTitle = new QLabel(qtr("Welcome to VLC!"), welcomeContent);
    welcomeTitle->setAlignment(Qt::AlignCenter);
    welcomeTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #ECECEC;");
    welcomeLayout->addWidget(welcomeTitle);
    auto *welcomeSubtitle = new QLabel(qtr("Free, open source and plays everything."), welcomeContent);
    welcomeSubtitle->setAlignment(Qt::AlignCenter);
    welcomeLayout->addWidget(welcomeSubtitle);

    if (vlc_ml_instance_get(p_intf))
    {
        auto *folderSection = new QWidget(welcomeContent);
        auto *folderLayout = new QVBoxLayout(folderSection);
        folderLayout->setContentsMargins(0, 4, 0, 0);
        auto *folderRow = new QHBoxLayout;
        auto *folderTitle = new QLabel(qtr("Choose a Media Folder"), folderSection);
        folderRow->addWidget(folderTitle);
        folderRow->addStretch(1);
        auto *videosButton = new QPushButton(qtr("Videos"), folderSection);
        auto *browseButton = new QPushButton(qtr("Browse..."), folderSection);
        folderRow->addWidget(videosButton);
        folderRow->addWidget(browseButton);
        folderLayout->addLayout(folderRow);
        ui.entryPoints->setParent(folderSection);
        ui.entryPoints->setMaximumHeight(76);
        folderLayout->addWidget(ui.entryPoints);
        welcomeLayout->addWidget(folderSection);
        connect(videosButton, &QPushButton::clicked, this, &FirstRunWizard::MLaddVideosFolder);
        connect(browseButton, &QPushButton::clicked, this, &FirstRunWizard::MLaddNewFolder);

        auto *foldersModel = new MLFoldersModel(this);
        foldersModel->setCtx(p_intf->p_mi);
        ui.entryPoints->setMLFoldersModel(foldersModel);
        mlFoldersEditor = ui.entryPoints;
        mlFoldersModel = foldersModel;
    }

    auto *privacySection = new QWidget(welcomeContent);
    auto *privacyLayout = new QVBoxLayout(privacySection);
    privacyLayout->setContentsMargins(0, 0, 0, 0);
    privacyLayout->setSpacing(3);
    ui.privacy->setParent(privacySection);
    ui.privacy->hide();
    privacyLayout->addWidget(ui.privacy);
    ui.privacyCheckbox->setParent(privacySection);
    ui.privacyCheckbox->setText(qtr("Allow metadata network access"));
    privacyLayout->addWidget(ui.privacyCheckbox, 0, Qt::AlignHCenter);
    auto *privacyNote = new QLabel(qtr("VLC may retrieve media metadata and artwork from online services."), privacySection);
    privacyNote->setAlignment(Qt::AlignCenter);
    privacyLayout->addWidget(privacyNote);
    auto *detailsButton = new QToolButton(privacySection);
    detailsButton->setObjectName("detailsButton");
    detailsButton->setText(qtr("Details"));
    detailsButton->setCheckable(true);
    privacyLayout->addWidget(detailsButton, 0, Qt::AlignHCenter);
    ui.policy->setParent(privacySection);
    privacyLayout->addWidget(ui.policy);
    connect(detailsButton, &QToolButton::toggled, ui.privacy, &QWidget::setVisible);
    connect(detailsButton, &QToolButton::toggled, ui.policy, &QWidget::setVisible);
    welcomeLayout->addWidget(privacySection);
    welcomePageLayout->addStretch(1);
    welcomePageLayout->addWidget(welcomeContent, 0, Qt::AlignHCenter);
    welcomePageLayout->addStretch(1);

    colorSchemeGroup = new QButtonGroup( this );
    colorSchemeGroup->addButton( ui.systemButton );
    colorSchemeGroup->addButton( ui.lightButton );
    colorSchemeGroup->addButton( ui.darkButton );
    colorSchemeGroup->setExclusive( true );
    ui.darkButton->setChecked(true);

    ui.systemButton->hide();
    ui.lightButton->hide();
    ui.darkButton->hide();
    ui.daynightImage->hide();
    ui.lightImage->hide();
    ui.darkImage->hide();
    for (QWidget *child : ui.colorSchemePage->findChildren<QWidget *>(
             QString(), Qt::FindDirectChildrenOnly))
        child->hide();
    delete ui.colorSchemePage->layout();
    auto *themePageLayout = new QVBoxLayout(ui.colorSchemePage);
    themePageLayout->setContentsMargins(24, 22, 24, 18);
    themePageLayout->setSpacing(14);
    auto *themeTitle = new QLabel(qtr("Select your preferred theme and color."), ui.colorSchemePage);
    themeTitle->setAlignment(Qt::AlignCenter);
    themeTitle->setStyleSheet("font-size: 18px; font-weight: 600; color: #ECECEC;");
    themePageLayout->addWidget(themeTitle);
    auto *themeRow = new QHBoxLayout;
    themeRow->setSpacing(18);
    colorSchemeImages = new QButtonGroup(this);
    colorSchemeImages->setExclusive(true);
    const QList<QPair<QString, QString>> themes = {
        { qtr("System"), QStringLiteral(":/misc/theme_daynight.svg") },
        { qtr("Light"), QStringLiteral(":/misc/theme_light.svg") },
        { qtr("Dark"), QStringLiteral(":/misc/theme_dark.svg") }
    };
    for (int i = 0; i < themes.size(); ++i)
    {
        auto *card = new QToolButton(ui.colorSchemePage);
        card->setObjectName("themeCard");
        card->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        card->setIcon(QIcon(themes[i].second));
        card->setIconSize(QSize(178, 110));
        card->setText(themes[i].first);
        card->setFixedSize(200, 150);
        card->setCheckable(true);
        colorSchemeImages->addButton(card, i);
        themeRow->addWidget(card);
    }
    colorSchemeImages->button(ColorSchemeModel::Night)->setChecked(true);
    themePageLayout->addStretch(1);
    themePageLayout->addLayout(themeRow);
    themePageLayout->addStretch(1);

    const QStringList accentColors = {
        "#E87808", "#C0392B", "#A08A1E", "#1E8E3E", "#1690A0",
        "#1E4FB4", "#7A3CC8", "#C0398A", "#8F8F8F"
    };
    selectedAccent = p_intf->p_mi->accentColor();
    if (!accentColors.contains(selectedAccent, Qt::CaseInsensitive))
        selectedAccent = QStringLiteral("#E87808");
    auto *accentSection = new QWidget(ui.colorSchemePage);
    accentSection->setMaximumWidth(420);
    auto *accentLayout = new QVBoxLayout(accentSection);
    accentLayout->setContentsMargins(0, 4, 0, 0);
    auto *accentLabel = new QLabel(qtr("Accent color"), accentSection);
    accentLabel->setAlignment(Qt::AlignCenter);
    accentLayout->addWidget(accentLabel);
    auto *swatchRow = new QHBoxLayout;
    swatchRow->setSpacing(8);
    swatchRow->addStretch(1);
    auto *accentGroup = new QButtonGroup(this);
    accentGroup->setExclusive(true);
    for (int i = 0; i < accentColors.size(); ++i)
    {
        auto *swatch = new QToolButton(accentSection);
        swatch->setObjectName("onboardingAccentSwatch");
        swatch->setFixedSize(28, 28);
        swatch->setCheckable(true);
        swatch->setAccessibleName(accentColors[i]);
        accentGroup->addButton(swatch, i);
        swatchRow->addWidget(swatch);
        if (p_intf->p_mi->accentColor().compare(accentColors[i], Qt::CaseInsensitive) == 0)
            swatch->setChecked(true);
        swatch->setStyleSheet(
            QStringLiteral("QToolButton { background: %1; border: %2px solid %3; border-radius: 4px; padding: 0; }")
                .arg(accentColors[i])
                .arg(swatch->isChecked() ? 2 : 0)
                .arg(swatch->isChecked() ? QStringLiteral("#ECECEC") : QStringLiteral("transparent")));
    }
    swatchRow->addStretch(1);
    connect(accentGroup, &QButtonGroup::idClicked, this,
            [this, accentColors, accentGroup](int index) {
        selectedAccent = accentColors[index];
        for (int i = 0; i < accentColors.size(); ++i)
        {
            auto *swatch = qobject_cast<QToolButton *>(accentGroup->button(i));
            swatch->setStyleSheet(
                QStringLiteral("QToolButton { background: %1; border: %2px solid %3; border-radius: 4px; padding: 0; }")
                    .arg(accentColors[i])
                    .arg(i == index ? 2 : 0)
                    .arg(i == index ? QStringLiteral("#ECECEC") : QStringLiteral("transparent")));
        }
    });
    accentLayout->addLayout(swatchRow);
    themePageLayout->addWidget(accentSection, 0, Qt::AlignHCenter);

    for (QWidget *child : ui.layoutPage->findChildren<QWidget *>(
             QString(), Qt::FindDirectChildrenOnly))
        child->hide();
    delete ui.layoutPage->layout();
    auto *shortcutsPageLayout = new QVBoxLayout(ui.layoutPage);
    shortcutsPageLayout->setContentsMargins(24, 20, 24, 16);
    shortcutsPageLayout->setSpacing(8);
    auto *shortcutsTitle = new QLabel(qtr("Learn the basic keyboard shortcuts"), ui.layoutPage);
    shortcutsTitle->setAlignment(Qt::AlignCenter);
    shortcutsTitle->setStyleSheet("font-size: 18px; font-weight: 600; color: #ECECEC;");
    shortcutsPageLayout->addWidget(shortcutsTitle);
    auto *shortcutsContent = new QWidget(ui.layoutPage);
    shortcutsContent->setMaximumWidth(700);
    auto *shortcutsLayout = new QVBoxLayout(shortcutsContent);
    shortcutsLayout->setContentsMargins(0, 0, 0, 0);
    auto *shortcutGrid = new QGridLayout;
    shortcutGrid->setHorizontalSpacing(12);
    shortcutGrid->setVerticalSpacing(8);
    const QList<QPair<QString, QString>> shortcuts = {
        { qtr("Play / Pause"), "key-play-pause" },
        { qtr("Fullscreen"), "key-toggle-fullscreen" },
        { qtr("Jump forward"), "key-jump+short" },
        { qtr("Jump backward"), "key-jump-short" },
        { qtr("Volume up"), "key-vol-up" },
        { qtr("Volume down"), "key-vol-down" },
        { qtr("Mute"), "key-vol-mute" },
        { qtr("Next track"), "key-next" },
        { qtr("Previous track"), "key-prev" },
        { qtr("Snapshot"), "key-snapshot" }
    };
    for (int i = 0; i < shortcuts.size(); ++i)
    {
        char *configured = var_InheritString(p_intf, qtu(shortcuts[i].second));
        const QString key = configured && *configured
                ? qfu(configured).split(QChar('\t'), Qt::SkipEmptyParts).value(0, qtr("Unassigned"))
                : qtr("Unassigned");
        free(configured);

        auto *card = new QFrame(shortcutsContent);
        card->setObjectName("shortcutCard");
        card->setFixedHeight(44);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(8, 4, 8, 4);
        cardLayout->setSpacing(10);
        auto *keyLabel = new QLabel(key, card);
        keyLabel->setObjectName("shortcutKeycap");
        keyLabel->setAlignment(Qt::AlignCenter);
        QFont keyFont(QStringLiteral("monospace"));
        keyFont.setPointSize(9);
        keyLabel->setFont(keyFont);
        keyLabel->setFixedSize(keyLabel->fontMetrics().horizontalAdvance(key) + 20, 28);
        auto *actionLabel = new QLabel(shortcuts[i].first, card);
        actionLabel->setObjectName("shortcutAction");
        cardLayout->addWidget(keyLabel);
        cardLayout->addWidget(actionLabel, 1);
        shortcutGrid->addWidget(card, i / 2, i % 2);
    }
    shortcutsLayout->addLayout(shortcutGrid);
    auto *fullList = new QHBoxLayout;
    fullList->addStretch(1);
    fullList->addWidget(new QLabel(qtr("Full list:"), shortcutsContent));
    auto *shortcutsButton = new QPushButton(qtr("Keyboard Shortcuts"), shortcutsContent);
    fullList->addWidget(shortcutsButton);
    fullList->addStretch(1);
    shortcutsLayout->addLayout(fullList);
    shortcutsPageLayout->addStretch(1);
    shortcutsPageLayout->addWidget(shortcutsContent, 0, Qt::AlignHCenter);
    shortcutsPageLayout->addStretch(1);
    connect(shortcutsButton, &QPushButton::clicked,
            DialogsProvider::getInstance(), &DialogsProvider::prefsHotkeysDialog);

    setOption( QWizard::NoCancelButton );

    connect( colorSchemeGroup, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::updateColorLabel );
    connect( colorSchemeImages, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::imageColorSchemeClick );
    connect( this->button(QWizard::FinishButton), &QPushButton::clicked, this, &FirstRunWizard::finish );
}

/**
 * Function called when the finish button is pressed.
 * Processes all inputted settings according to chosen options
 */
void FirstRunWizard::finish()
{
    config_PutInt( "metadata-network-access", ui.privacyCheckbox->isChecked() );
    config_PutInt( "qt-privacy-ask", 0 );
    p_intf->p_mi->getColorScheme()->setCurrentIndex( colorSchemeGroup->checkedId() );
    p_intf->p_mi->setAccentColor(selectedAccent);
    config_PutInt("qt-menubar", 0);
    config_PutInt("qt-titlebar", 0);
    config_PutInt("qt-pin-controls", 0);

    if( vlc_ml_instance_get( p_intf ) && mlFoldersEditor )
        mlFoldersEditor->commit();

    if( p_intf->p_mi->getMediaLibrary() )
        p_intf->p_mi->getMediaLibrary()->reload();

    p_intf->p_mi->reloadPrefs();
    p_intf->p_mi->controlbarProfileModel()->save();
    config_SaveConfigFile( p_intf );
}

/**
 * Opens a QFileDialog for the user to select a folder to add to the Media Library
 * If a folder is selected, add it to the list of folders under MLFoldersEditor
 */
void FirstRunWizard::MLaddNewFolder()
{
    QUrl newEntryPoint = QFileDialog::getExistingDirectoryUrl( this, qtr("Choose a media folder"),
                                                               QUrl( QDir::homePath() ));

    if( !newEntryPoint.isEmpty() && !mlFoldersEditor->contains( newEntryPoint ) )
        mlFoldersEditor->add( newEntryPoint );
}

void FirstRunWizard::MLaddVideosFolder()
{
    auto folder = vlc::wrap_cptr(config_GetUserDir(VLC_VIDEOS_DIR));
    if (!folder)
        return;
    auto folderMrl = vlc::wrap_cptr(vlc_path2uri(folder.get(), nullptr));
    if (!folderMrl)
        return;
    const QUrl url(folderMrl.get());
    if (mlFoldersEditor && !mlFoldersEditor->contains(url))
        mlFoldersEditor->add(url);
}

/**
 * Automatically updates the label on the color scheme page depending
 * on the currently selected radio button choice
 * @param id The id of the button we are updating the label of
 */
void FirstRunWizard::updateColorLabel( QAbstractButton* btn )
{
    if (QAbstractButton *image = colorSchemeImages->button(colorSchemeGroup->id(btn)))
        image->setChecked(true);

    switch ( colorSchemeGroup->id(btn) )
    {
        case ColorSchemeModel::System:
            ui.explainerLabel->setText( qtr( "<i>VLC will automatically switch to dark mode accordingly with system settings</i>" ) );
            break;
        case ColorSchemeModel::Day:
            ui.explainerLabel->setText( qtr( "<i>VLC will automatically use light mode</i>" ) );
            break;
        case ColorSchemeModel::Night:
            ui.explainerLabel->setText( qtr( "<i>VLC will automatically use dark mode</i>" ) );
            break;
    }
}

/**
 * Checks the correct button when the corresponding image is clicked
 * for the color scheme page.
 * @param id The id of the image that was clicked
 */
void FirstRunWizard::imageColorSchemeClick( QAbstractButton* btn )
{
    QAbstractButton* groupBtn = colorSchemeGroup->button( colorSchemeImages->id( btn ) );
    assert( groupBtn );
    groupBtn->setChecked( true );
    updateColorLabel( groupBtn );
}

/**
 * Defines the navigation of pages for the wizard
 * @return int - the page id to go to or -1 if we are done
 */
int FirstRunWizard::nextId() const
{
    switch ( currentId() )
    {
        case WELCOME_PAGE:
            return COLOR_SCHEME_PAGE;
        case COLOR_SCHEME_PAGE:
            return SHORTCUTS_PAGE;
        default:
            return -1;
    }
}

/**
 * Sets up the buttons and options for the color scheme page
 * Needs to be set up later or else the main interface hasn't been created yet
 * Color schemes are always in the order system/auto, day then night
 */
void FirstRunWizard::initializePage( int id )
{
    if(id == COLOR_SCHEME_PAGE)
    {
        QVector<ColorSchemeModel::Item> schemes = p_intf->p_mi->getColorScheme()->getSchemes();
        auto schemeButtons = colorSchemeGroup->buttons();
        auto schemeImages = colorSchemeImages->buttons();

        /* Number of buttons should be equal to the schemes we can choose from */
        assert( schemes.size() == schemeButtons.size() );
        assert( schemes.size() == schemeImages.size() );

        for( int i = 0; i < schemes.size(); i++ )
        {
            colorSchemeGroup->setId( schemeButtons.at(i), schemes.at(i).scheme );
            colorSchemeImages->setId( schemeImages.at(i), schemes.at(i).scheme );
        }
        QAbstractButton *dark = colorSchemeGroup->button(ColorSchemeModel::Night);
        if (dark)
        {
            dark->setChecked(true);
            updateColorLabel(dark);
        }
    }
}

/**
 * Processes the default options on rejection of the FirstRun Wizard.
 * The default options are:
 * - Yes to metadata
 * - System/Auto colour scheme
 * - Modern VLC layout
 */
void FirstRunWizard::reject()
{
    assert(p_intf->p_mi);
    /* Welcome Page settings  */
    config_PutInt( "metadata-network-access", 1 );
    config_PutInt( "qt-privacy-ask", 0 );

    /* Colour Page settings */
    p_intf->p_mi->getColorScheme()->setCurrentIndex( 0 );

    config_PutInt( "qt-menubar", 0 );
    config_PutInt( "qt-titlebar", 0 );
    config_PutInt( "qt-pin-controls", 0 );
    p_intf->p_mi->setAccentColor(QStringLiteral("#E87808"));

    if( p_intf->p_mi->getMediaLibrary() )
        p_intf->p_mi->getMediaLibrary()->reload();

    config_SaveConfigFile( p_intf );
    p_intf->p_mi->reloadPrefs();
    p_intf->p_mi->controlbarProfileModel()->save();

    done( QDialog::Rejected );
}
