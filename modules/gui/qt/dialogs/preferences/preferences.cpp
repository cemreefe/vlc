/*****************************************************************************
 * preferences.cpp : Preferences
 *****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "dialogs/preferences/preferences.hpp"

#include "expert_view.hpp"
#include "dialogs/preferences/complete_preferences.hpp"
#include "dialogs/preferences/simple_preferences.hpp"
#include "dialogs/dialogs/dialogmodel.hpp"
#include "dialogs/dialogs_provider.hpp"
#include "dialogs/plugins/plugins.hpp"
#include "widgets/native/searchlineedit.hpp"
#include "widgets/native/qvlcframe.hpp"
#include "widgets/native/vlcdarkstyle.hpp"
#include "maininterface/mainctx.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QRadioButton>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QSplitter>
#include <QShortcut>

#include <functional>
#include <cstring>

#include <vlc_configuration.h>
#include <vlc_modules.h>

static QString preferencesStyle(qt_intf_t *p_intf)
{
    const QString accent = vlcWidgetAccentColor(VLC_OBJECT(p_intf));
    const QColor accentColor(accent);
    const QString accentHover =
            accentColor.lighter(110).name(QColor::HexRgb).toUpper();

    return vlcDarkWidgetStyle(accent) + QStringLiteral(
        "QDialog#settingsDialog { background: #161615; color: #ECECEC; }"
        "QWidget#settingsSidebarPanel { background: #121211; }"
        "QWidget#settingsContent { background: #161615; }"
        "QStackedWidget#settingsPanelsStack { background: #161615; }"
        "QLineEdit#settingsSearch { background: #1E1D1C; border: 1px solid #2B2927; border-radius: 4px; color: #D0D0D0; padding: 4px 8px; }"
        "QScrollArea { background: #161615; border: none; }"
        "QLabel#settingsSectionLabel { color: #8F8F8F; font-size: 11px; margin: 12px 4px 4px; }"
        "QToolButton#settingsNavItem { border: none; text-align: left; padding: 0 8px; color: #D0D0D0; }"
        "QToolButton#settingsNavItem:hover { background: #1E1D1C; border-radius: 6px; }"
        "QToolButton#settingsNavItem:checked { background: #262423; border-radius: 6px; color: #ECECEC; }"
        "QLabel#settingsPageTitle { color: #ECECEC; font-size: 15px; }"
        "QToolButton#settingsCloseButton { border: none; color: #8F8F8F; font-size: 20px; padding: 0 8px; }"
        "QGroupBox { border: none; margin-top: 16px; color: #ECECEC; font-size: 14px; }"
        "QGroupBox::title { subcontrol-origin: margin; top: 0; left: 0; }"
        "QCheckBox { color: #D0D0D0; spacing: 10px; direction: rtl; text-align: left; }"
        "QCheckBox::indicator { width: 40px; height: 20px; image: url(:/settings/toggle-off.svg); background: #2B2927; border-radius: 10px; }"
        "QCheckBox::indicator:checked { image: url(:/settings/toggle-on.svg); background: %1; }"
        "QComboBox, QLineEdit, QSpinBox { background: #1E1D1C; border: 1px solid #2B2927; border-radius: 4px; color: #D0D0D0; min-height: 28px; padding: 2px 8px; }"
        "QComboBox QAbstractItemView { background: #1E1D1C; color: #D0D0D0; selection-background-color: #262423; }"
        "QSlider::groove:horizontal { height: 4px; background: #2B2927; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: %1; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 14px; margin: -5px 0; border-radius: 7px; background: %1; }"
        "QPushButton#settingsSaveButton { background: %1; border: none; border-radius: 5px; color: #161615; padding: 7px 18px; }"
        "QPushButton#settingsSaveButton:hover { background: %2; }"
        "QPushButton#settingsAdvancedButton, QPushButton#settingsBackButton { background: transparent; border: none; color: %1; padding: 7px 10px; }"
        "QLabel#settingsThemeSectionLabel { color: #ECECEC; font-size: 14px; }"
        "QPushButton#settingsThemeSegment { background: #1E1D1C; border: 1px solid #2B2927; border-right: none; color: #8F8F8F; padding: 6px 16px; min-height: 30px; }"
        "QPushButton#settingsThemeSegment[segmentPosition=\"first\"] { border-top-left-radius: 5px; border-bottom-left-radius: 5px; }"
        "QPushButton#settingsThemeSegment[segmentPosition=\"last\"] { border-right: 1px solid #2B2927; border-top-right-radius: 5px; border-bottom-right-radius: 5px; }"
        "QPushButton#settingsThemeSegment:checked { background: #3A2410; color: %1; border-color: %1; }"
    ).arg(accent, accentHover);
}

PrefsDialog::PrefsDialog( QWindow *parent, qt_intf_t *_p_intf )
            : QVLCDialog( parent, _p_intf )
{
    setWindowTitle( qtr( "Settings" ) );
    setObjectName("settingsDialog");
    setWindowRole( "vlc-preferences" );
    setWindowModality( Qt::WindowModal );
    setMinimumSize(850, 650);
    setStyleSheet(preferencesStyle(_p_intf));
    if (_p_intf->p_mi)
        connect(_p_intf->p_mi, &MainCtx::accentColorChanged, this,
                [this, _p_intf]() { setStyleSheet(preferencesStyle(_p_intf)); });

    /* Whether we want it or not, we need to destroy on close to get
       consistency when reset */
    setAttribute( Qt::WA_DeleteOnClose );

    p_list = NULL;

    auto *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    /* View (panel) selection */
    types = new QGroupBox( qtr("Show settings") );
    types->setAlignment( Qt::AlignHCenter );
    QHBoxLayout *types_l = new QHBoxLayout;
    types_l->setSpacing( 3 );
    types_l->setContentsMargins(3, 3, 3, 3);
    simple = new QRadioButton( qtr( "Simple" ), types );
    simple->setToolTip( qtr( "Switch to simple preferences view" ) );
    all = new QRadioButton( qtr("All"), types );
    all->setToolTip( qtr( "Switch to full preferences view" ) );
    expert = new QRadioButton( qtr("Expert"), types );
    expert->setToolTip( qtr( "Switch to expert preferences view" ) );
    types_l->addWidget( simple );
    types_l->addWidget( all );
    types_l->addWidget( expert );
    types->setLayout( types_l );
    simple->setChecked( true );

    /* Buttons */
    QDialogButtonBox *buttonsBox = new QDialogButtonBox();
    QPushButton *save = new QPushButton( qtr( "&Save" ) );
    save->setObjectName("settingsSaveButton");
    save->setToolTip( qtr( "Save and close the dialog" ) );
    QPushButton *cancel = new QPushButton( qtr( "&Cancel" ) );
    QPushButton *reset = new QPushButton( qtr( "&Reset Preferences" ) );

    buttonsBox->addButton( save, QDialogButtonBox::AcceptRole );
    buttonsBox->addButton( cancel, QDialogButtonBox::RejectRole );
    buttonsBox->addButton( reset, QDialogButtonBox::ResetRole );

    /* View (panel) stack */
    stack = new QStackedWidget();

    /* Simple view (panel) */
    simple_split_widget = new QWidget();
    simple_split_widget->setObjectName("settingsSimpleView");
    auto *simple_layout = new QVBoxLayout(simple_split_widget);
    simple_layout->setContentsMargins(0, 0, 0, 0);
    simple_layout->setSpacing(0);

    auto *simple_row = new QHBoxLayout;
    simple_row->setContentsMargins(0, 0, 0, 0);
    simple_row->setSpacing(0);

    simple_tree_panel = new QWidget(simple_split_widget);
    simple_tree_panel->setObjectName("settingsSidebarPanel");
    simple_tree_panel->setLayout(new QVBoxLayout);
    simple_tree_panel->layout()->setContentsMargins(0, 0, 0, 0);
    simple_tree = nullptr;
    simple_content = new QWidget(simple_split_widget);
    simple_content->setObjectName("settingsContent");
    auto *content_layout = new QVBoxLayout(simple_content);
    content_layout->setContentsMargins(24, 18, 24, 10);
    content_layout->setSpacing(10);

    auto *header = new QHBoxLayout;
    simple_header_title = new QLabel(qtr("General Preferences"), simple_content);
    simple_header_title->setObjectName("settingsPageTitle");
    auto *close_button = new QToolButton(simple_content);
    close_button->setObjectName("settingsCloseButton");
    close_button->setText(QString::fromUtf8("×"));
    close_button->setFixedSize(32, 32);
    header->addWidget(simple_header_title);
    header->addStretch(1);
    header->addWidget(close_button);
    content_layout->addLayout(header);

    simple_panels_stack = new QStackedWidget(simple_content);
    simple_panels_stack->setObjectName("settingsPanelsStack");
    content_layout->addWidget(simple_panels_stack, 1);

    simple_row->addWidget(simple_tree_panel);
    simple_row->addWidget(simple_content, 1);
    simple_layout->addLayout(simple_row, 1);
    simple_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    simple_tree_panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    for (int i = 0; i < SettingsPageMax; ++i)
        simple_pages[i] = nullptr;
    for( int i = 0; i < SPrefsMax ; i++ )
        simple_panels[i] = NULL;

    stack->insertWidget( SIMPLE, simple_split_widget );

    /* Advanced view (panel) */
    advanced_split_widget = new QSplitter();

    advanced_tree_panel = new QWidget;
    advanced_tree_panel->setObjectName("settingsAdvancedSidebar");
    advanced_tree_panel->setLayout( new QVBoxLayout );
    tree_filter = NULL;
    current_filter = NULL;
    advanced_tree = NULL;
    advanced_panels_stack = new QStackedWidget;

    advanced_split_widget->addWidget( advanced_tree_panel );
    advanced_split_widget->addWidget( advanced_panels_stack );

    advanced_split_widget->setSizes(QList<int>() << 300 << 550);
    advanced_tree_panel->sizePolicy().setHorizontalStretch(1);
    advanced_panels_stack->sizePolicy().setHorizontalStretch(2);

    stack->insertWidget( ADVANCED, advanced_split_widget );

    /* Expert view (panel) */
    expert_widget = new QWidget;
    expert_widget_layout = new QGridLayout;
    expert_widget->setLayout( expert_widget_layout );

    expert_table_filter = nullptr;
    expert_table = nullptr;

    expert_text = new QLabel;
    expert_longtext = new QLabel;

    expert_text->setWordWrap(true);
    expert_longtext->setWordWrap(true);

    QFont textFont = QApplication::font();
    textFont.setPointSize( textFont.pointSize() + 2 );
    textFont.setUnderline( true );
    expert_text->setFont( textFont );

    expert_widget_layout->addWidget( expert_text, 2, 0, 1, 2 );
    expert_widget_layout->addWidget( expert_longtext, 3, 0, 1, 2 );

    stack->insertWidget( EXPERT, expert_widget );

    /* Layout  */
    main_layout->addWidget(stack, 1);
    auto *footer = new QHBoxLayout;
    footer->setContentsMargins(12, 8, 12, 8);
    advanced_button = new QPushButton(qtr("Advanced settings"));
    advanced_button->setObjectName("settingsAdvancedButton");
    back_button = new QPushButton(qtr("← Back"));
    back_button->setObjectName("settingsBackButton");
    back_button->hide();
    types->hide();
    footer->addWidget(advanced_button);
    footer->addWidget(back_button);
    footer->addStretch(1);
    footer->addWidget(buttonsBox);
    main_layout->addLayout(footer);

    connect(close_button, &QToolButton::clicked, this, &PrefsDialog::save);
    connect(advanced_button, &QPushButton::clicked, this, &PrefsDialog::setAdvanced);
    connect(back_button, &QPushButton::clicked, this, &PrefsDialog::setSimple);
    createAdvancedTree();

    switch( var_InheritInteger( p_intf, "qt-initial-prefs-view" ) )
    {
        case 2:  setExpert();   break;
        case 1:  setAdvanced(); break;
        default: setSimple();   break;
    }

    BUTTONACT( save, &PrefsDialog::save );
    BUTTONACT( cancel, &PrefsDialog::cancel );
    BUTTONACT( reset, &PrefsDialog::reset );

    BUTTONACT( simple, &PrefsDialog::setSimple );
    BUTTONACT( all, &PrefsDialog::setAdvanced );
    BUTTONACT( expert, &PrefsDialog::setExpert );

    QVLCTools::restoreWidgetPosition( p_intf, "Preferences", this, QSize( 850, 700 ) );
}

PrefsDialog::~PrefsDialog()
{
    module_list_free( p_list );
}

void PrefsDialog::setExpert()
{
    /* Lazy creation */
    if( !expert_table )
    {
        if ( !p_list )
            p_list = module_list_get( &count );

        expert_table_filter = new SearchLineEdit( expert_widget );
        expert_table_filter->setMinimumHeight( 26 );
        expert_table_filter_modified = new QCheckBox( qtr( "Modified only" ), expert_widget );

        QShortcut *search = new QShortcut( QKeySequence( QKeySequence::Find ), expert_table_filter );

        ExpertPrefsTableModel *table_model = new ExpertPrefsTableModel( p_list, count, this );
        expert_table = new ExpertPrefsTable( expert_widget );
        expert_table->setModel( table_model );
        expert_table->resizeColumnToContents( ExpertPrefsTableModel::NameField );

        expert_widget_layout->addWidget( expert_table_filter, 0, 0 );
        expert_widget_layout->addWidget( expert_table_filter_modified, 0, 1 );
        expert_widget_layout->addWidget( expert_table, 1, 0, 1, 2 );

        connect( expert_table->selectionModel(), &QItemSelectionModel::currentChanged,
                 this, &PrefsDialog::changeExpertDesc );
        connect( expert_table_filter, &SearchLineEdit::textChanged,
                 this, &PrefsDialog::expertTableFilterChanged );
        connect( expert_table_filter_modified, &QCheckBox::toggled,
                 this, &PrefsDialog::expertTableFilterModifiedToggled );
        connect( search, &QShortcut::activated,
                 expert_table_filter, QOverload<>::of(&SearchLineEdit::setFocus) );

        /* Set initial selection */
        expert_table->setCurrentIndex(
                expert_table->model()->index( 0, 0, QModelIndex() ) );
    }

    expert->setChecked( true );
    stack->setCurrentIndex( EXPERT );
    advanced_button->hide();
    back_button->show();
    setWindowTitle( qtr( "Settings" ) );
}

void PrefsDialog::createAdvancedTree()
{
    if (advanced_tree)
        return;

    if (!p_list)
        p_list = module_list_get(&count);

    advanced_tree = new PrefsTree(p_intf, advanced_tree_panel, p_list, count);

    tree_filter = new SearchLineEdit(advanced_tree_panel);
    tree_filter->setMinimumHeight(26);

    current_filter = new QCheckBox(qtr("Only show current"));
    current_filter->setToolTip(qtr("Only show modules related to current playback"));

    QShortcut *search = new QShortcut(QKeySequence(QKeySequence::Find), tree_filter);

    advanced_tree_panel->layout()->addWidget(tree_filter);
    advanced_tree_panel->layout()->addWidget(current_filter);
    advanced_tree_panel->layout()->addWidget(advanced_tree);
    advanced_tree_panel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    connect(advanced_tree, &PrefsTree::currentItemChanged, this, &PrefsDialog::changeAdvPanel);
    connect(tree_filter, &SearchLineEdit::textChanged,
            this, &PrefsDialog::advancedTreeFilterChanged);
    connect(current_filter, &QtCheckboxChanged, this, &PrefsDialog::onlyLoadedToggled);
    connect(search, &QShortcut::activated,
            tree_filter, QOverload<>::of(&SearchLineEdit::setFocus));

    advanced_tree->setCurrentIndex(
            advanced_tree->model()->index(0, 0, QModelIndex()));
}

void PrefsDialog::setAdvanced()
{
    createAdvancedTree();
    all->setChecked(true);
    stack->setCurrentIndex(ADVANCED);
    advanced_button->hide();
    back_button->show();
    setWindowTitle(qtr("Settings"));
}

void PrefsDialog::setSimple()
{
    if (!simple_tree)
    {
        simple_tree = new SPrefsCatList(p_intf, simple_tree_panel);
        connect(simple_tree, &SPrefsCatList::currentItemChanged,
                this, &PrefsDialog::changeSimplePanel);
        connect(simple_tree, &SPrefsCatList::searchRequested, this,
                [this](const QString& text) {
                    if (text.isEmpty())
                        return;
                    setAdvanced();
                    tree_filter->setText(text);
                    tree_filter->setFocus();
                });
        simple_tree_panel->layout()->addWidget(simple_tree);
    }

    if (!simple_pages[SettingsPageGeneral])
    {
        auto *panel = new SPrefsPanel(p_intf, simple_panels_stack, SPrefsInterface);
        simple_panels[SPrefsInterface] = panel;
        simple_pages[SettingsPageGeneral] = panel;
        simple_panels_stack->addWidget(panel);

        auto *privacyPage = new QWidget(simple_panels_stack);
        auto *privacyLayout = new QVBoxLayout(privacyPage);
        privacyLayout->setContentsMargins(0, 8, 0, 0);
        if (QGroupBox *privacyGroup = panel->takePrivacyGroup())
            privacyLayout->addWidget(privacyGroup);
        privacyLayout->addStretch(1);
        simple_pages[SettingsPagePrivacy] = privacyPage;
        simple_panels_stack->addWidget(privacyPage);
    }

    changeSimplePanel(SettingsPageGeneral);

    simple->setChecked(true);
    stack->setCurrentIndex(SIMPLE);
    advanced_button->show();
    back_button->hide();
    setWindowTitle(qtr("Settings"));
}

void PrefsDialog::showHotkeysPage()
{
    setSimple();
    simple_tree->switchPanel(SettingsPageControls);
    show();
    raise();
    activateWindow();
}

/* Switching from on simple panel to another */
void PrefsDialog::changeSimplePanel(int number)
{
    if (number < 0 || number >= SettingsPageMax)
        return;

    auto addPanel = [this](int page, int category) {
        if (!simple_panels[category])
            simple_panels[category] = new SPrefsPanel(p_intf, simple_panels_stack, category);
        simple_pages[page] = simple_panels[category];
        if (simple_panels_stack->indexOf(simple_pages[page]) < 0)
            simple_panels_stack->addWidget(simple_pages[page]);
    };

    auto findNode = [this](const char *module, int subcategory) -> PrefsTreeItem * {
        std::function<PrefsTreeItem *(PrefsTreeItem *)> search =
                [&](PrefsTreeItem *node) -> PrefsTreeItem * {
            if (!node)
                return nullptr;
            if ((module && node->module_name && std::strcmp(node->module_name, module) == 0)
                    || (subcategory >= 0 && node->subcat_id == subcategory))
                return node;
            for (int i = 0; i < node->childCount(); ++i)
                if (PrefsTreeItem *found = search(node->child(i)))
                    return found;
            return nullptr;
        };
        for (int i = 0; i < advanced_tree->topLevelItemCount(); ++i)
            if (PrefsTreeItem *found = search(advanced_tree->topLevelItem(i)))
                return found;
        return nullptr;
    };

    switch (number)
    {
        case SettingsPageControls:
            addPanel(number, SPrefsHotkeys);
            break;
        case SettingsPagePlayback:
            addPanel(number, SPrefsInputAndCodecs);
            break;
        case SettingsPageVideo:
            addPanel(number, SPrefsVideo);
            break;
        case SettingsPageAudio:
            addPanel(number, SPrefsAudio);
            break;
        case SettingsPageSubtitles:
            addPanel(number, SPrefsSubtitles);
            break;
        case SettingsPageLibrary:
            addPanel(number, SPrefsMediaLibrary);
            break;
        case SettingsPageInterface:
        case SettingsPageNetwork:
        case SettingsPagePerformance:
        case SettingsPageRecording:
        {
            const char *module = nullptr;
            int subcategory = -1;
            if (number == SettingsPageInterface)
                module = "qt";
            else if (number == SettingsPageNetwork)
                subcategory = SUBCAT_INPUT_ACCESS;
            else if (number == SettingsPagePerformance)
                subcategory = SUBCAT_ADVANCED_MISC;
            else
                subcategory = SUBCAT_SOUT_GENERAL;

            if (!simple_pages[number])
            {
                PrefsTreeItem *node = findNode(module, subcategory);
                if (!node)
                    return;
                auto *panel = new AdvPrefsPanel(p_intf, simple_panels_stack, node);
                simple_pages[number] = panel;
                simple_panels_stack->addWidget(panel);
            }
            break;
        }
        case SettingsPageExtensions:
            if (!simple_pages[number])
            {
                auto *extensions = new ExtensionTab(p_intf);
                extensions->setParent(simple_panels_stack);
                simple_pages[number] = extensions;
                simple_panels_stack->addWidget(extensions);
            }
            break;
        case SettingsPageGeneral:
        case SettingsPagePrivacy:
            break;
        default:
            return;
    }

    static const char *pageNames[] = {
        "General", "Interface", "Privacy", "Controls", "Playback", "Video",
        "Audio", "Subtitles", "Library", "Network", "Performance", "Recording",
        "Extensions"
    };
    simple_header_title->setText(qtr(pageNames[number]) + qtr(" Preferences"));
    if (simple_pages[number])
        simple_panels_stack->setCurrentWidget(simple_pages[number]);
}

/* Changing from one Advanced Panel to another */
void PrefsDialog::changeAdvPanel( QTreeWidgetItem *item )
{
    if( item == NULL ) return;
    PrefsTreeItem *node = static_cast<PrefsTreeItem *>( item );

    if( !node->panel )
    {
        node->panel = new AdvPrefsPanel( p_intf, advanced_panels_stack, node );
        advanced_panels_stack->addWidget( node->panel );
    }
    advanced_panels_stack->setCurrentWidget( node->panel );
}

/* Changing from one Expert item description to another */
void PrefsDialog::changeExpertDesc( const QModelIndex &current, const QModelIndex &previous )
{
    Q_UNUSED( previous );
    if( !current.isValid() )
       return;
    ExpertPrefsTableItem *item = expert_table->myModel()->itemAt( current );

    expert_text->setText( item->getTitle() );
    expert_longtext->setText( item->getDescription() );
}

/* Actual apply and save for the preferences */
void PrefsDialog::save()
{
    if( simple->isChecked() && simple_tree->isVisible() )
    {
        msg_Dbg( p_intf, "Saving the simple preferences" );
        if (advanced_tree)
            advanced_tree->applyAll();
        for (SPrefsPanel *panel : simple_panels)
            if (panel)
                panel->apply();
        for (int i = 0; i < SettingsPageMax; ++i)
            if (auto *panel = qobject_cast<AdvPrefsPanel *>(simple_pages[i]))
                panel->apply();
    }
    else if( all->isChecked() && advanced_tree->isVisible() )
    {
        msg_Dbg( p_intf, "Saving the advanced preferences" );
        advanced_tree->applyAll();
    }
    else if( expert->isChecked() && expert_table->isVisible() )
    {
        msg_Dbg( p_intf, "Saving the expert preferences" );
        expert_table->applyAll();
    }

    /* Save to file */
    if( config_SaveConfigFile( p_intf ) != 0 )
    {
        DialogErrorModel::getInstance(p_intf)->pushError(
            qtr( "Cannot save Configuration" ),
            qtr("Preferences file could not be saved")
        );
        if (p_intf->b_isDialogProvider)
        {
            //at this point we are closing the preference dialog which is modal,
            //we need to postpone the dialog raising up
            QMetaObject::invokeMethod(DialogsProvider::getInstance(), [](){
                DialogsProvider::getInstance()->messagesDialog(1);
            }, Qt::QueuedConnection);
        }
    }

    if( p_intf->p_mi )
        p_intf->p_mi->reloadPrefs();
    accept();

    // Don't save widget position if reset is pending
    if( !p_intf->preferencesResetPending )
        QVLCTools::saveWidgetPosition( p_intf, "Preferences", this );

    if( p_intf->preferencesResetPending )
    {
        int restart = QMessageBox::question(
                          this,
                          qtr( "Restart Required" ),
                          qtr( "VLC needs to be restarted for the reset to take full effect.\n\nRestart now?" ),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::Yes);
        
        if( restart == QMessageBox::Yes )
        {
            // Quit VLC
            emit p_intf->p_mi->askToQuit();
        }
        else
        {
            // User chose not to restart - clear the flag
            p_intf->preferencesResetPending = false;
        }
    }

}

/* Clean the preferences, dunno if it does something really */
void PrefsDialog::cancel()
{
    bool wasResetting = p_intf->preferencesResetPending;
    
    // Clear the reset flag
    p_intf->preferencesResetPending = false;
    
    if( !wasResetting )
        QVLCTools::saveWidgetPosition( p_intf, "Preferences", this );
    reject();
}

/* Reset all the preferences, when you click the button */
void PrefsDialog::reset()
{
    int ret = QMessageBox::question(
                 this,
                 qtr( "Reset Preferences" ),
                 qtr( "Are you sure you want to reset your VLC media player preferences?" ),
                 QMessageBox::Ok | QMessageBox::Cancel,
                 QMessageBox::Ok);

    if( ret == QMessageBox::Ok )
    {
        config_ResetAll();
        config_SaveConfigFile( p_intf );
        getSettings()->clear();
        p_intf->preferencesResetPending = true;
        p_intf->p_mi->reloadPrefs();
        p_intf->p_mi->reloadFromSettings();

#ifdef _WIN32
        if (simple_panels[0])
            simple_panels[0]->cleanLang();
#endif

    }
}

void PrefsDialog::expertTableFilterModifiedToggled( bool checked )
{
    expert_table->filter( expert_table_filter->text(), checked );
}

void PrefsDialog::expertTableFilterChanged( const QString & text )
{
    expert_table->filter( text, expert_table_filter_modified->isChecked() );
}

void PrefsDialog::advancedTreeFilterChanged( const QString & text )
{
    advanced_tree->filter( text );
}

void PrefsDialog::onlyLoadedToggled()
{
    advanced_tree->setLoadedOnly( current_filter->isChecked() );
}
