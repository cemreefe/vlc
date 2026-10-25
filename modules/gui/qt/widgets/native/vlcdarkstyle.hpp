/*****************************************************************************
 * vlcdarkstyle.hpp : Dark widget stylesheet for the redesigned UI
 *****************************************************************************
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

#ifndef VLC_QT_VLCDARKSTYLE_HPP_
#define VLC_QT_VLCDARKSTYLE_HPP_

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QString>

#include <cstdlib>

#include <vlc_common.h>
#include <vlc_configuration.h>

inline QString vlcWidgetAccentColor(vlc_object_t *object)
{
    if (!object)
        return QStringLiteral("#E87808");

    char *option = var_InheritString(object, "qt-accent-color");
    if (!option)
        return QStringLiteral("#E87808");

    const QString value = QString::fromUtf8(option);
    free(option);

    if (value == QLatin1String("default") || value.isEmpty())
        return QStringLiteral("#E87808");

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (value == QLatin1String("system") && qApp)
        return qApp->palette().color(QPalette::Accent).name(QColor::HexRgb).toUpper();
#endif

    const QColor accent(value);
    return accent.isValid()
            ? accent.name(QColor::HexRgb).toUpper()
            : QStringLiteral("#E87808");
}

/* Shared dark stylesheet for widget-based dialogs, matching the
 * redesigned interface palette (topbar #1B1B1B, panels #10100F,
 * content #2D2B29, accent #E87808). */
inline QString vlcDarkWidgetStyle(const QString &accentColor = QStringLiteral("#E87808"))
{
    const QColor accent(accentColor);
    const QColor validAccent = accent.isValid() ? accent : QColor(QStringLiteral("#E87808"));
    QString style = QStringLiteral(
        "QWidget { background-color: #1B1B1B; color: #ECECEC; selection-color: #14120F; selection-background-color: #E87808; }"
        "QDialog { background-color: #1B1B1B; }"
        "QWizard, QWizardPage { background-color: #1B1B1B; color: #ECECEC; }"
        "QWizard QLabel { color: #ECECEC; }"
        "QWizard QTextBrowser { background-color: #1B1B1B; color: #ECECEC; border: none; }"
        "QWizard QDialogButtonBox { background-color: #1B1B1B; }"

        "QLabel { background-color: transparent; }"
        "QLabel:disabled { color: #6F6F6F; }"
        "QToolTip { background-color: #2B2927; color: #ECECEC; border: 1px solid #3A3836; padding: 4px 6px; }"

        "QPushButton, QToolButton {"
        "   background-color: #2B2927; color: #ECECEC;"
        "   border: none; border-radius: 6px; padding: 6px 14px; }"
        "QPushButton:hover, QToolButton:hover { background-color: #353230; }"
        "QPushButton:pressed, QToolButton:pressed { background-color: #1F1D1B; }"
        "QPushButton:disabled, QToolButton:disabled { color: #6F6F6F; background-color: #242220; }"
        "QPushButton:default, QToolButton:default { background-color: #E87808; color: #14120F; }"
        "QPushButton:default:hover, QToolButton:default:hover { background-color: #F08A22; }"
        "QPushButton:default:pressed, QToolButton:default:pressed { background-color: #C96608; }"
        "QToolButton::menu-indicator { image: none; width: 10px; }"
        "QToolButton#playButton { background-color: #E87808; color: #14120F; font-weight: 600; }"
        "QToolButton#playButton:hover { background-color: #F08A22; }"
        "QToolButton#playButton:pressed { background-color: #C96608; }"
        "QToolButton#playButton:disabled { background-color: #3A2A18; color: #8F8F8F; }"
        "QToolButton::menu-arrow { image: none; }"

        "QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QDoubleSpinBox,"
        "QTimeEdit, QDateEdit, QDateTimeEdit, QComboBox {"
        "   background-color: #262423; color: #ECECEC;"
        "   border: 1px solid #3A3836; border-radius: 6px; padding: 5px 8px; }"
        "QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus, QSpinBox:focus,"
        "QDoubleSpinBox:focus, QTimeEdit:focus, QDateEdit:focus, QDateTimeEdit:focus, QComboBox:focus {"
        "   border-color: #E87808; }"
        "QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {"
        "   color: #6F6F6F; background-color: #242220; }"
        "QLineEdit[readOnly=\"true\"] { color: #8F8F8F; }"
        "QComboBox::drop-down { border: none; width: 22px; }"
        "QComboBox::down-arrow { image: none; border-left: 4px solid transparent;"
        "   border-right: 4px solid transparent; border-top: 5px solid #8F8F8F; }"
        "QComboBox QAbstractItemView { background-color: #1B1B1B; color: #ECECEC;"
        "   border: 1px solid #2B2927; selection-background-color: #353230; }"
        "QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {"
        "   background: transparent; width: 16px; }"

        "QCheckBox, QRadioButton { background-color: transparent; spacing: 6px; }"
        "QCheckBox::indicator, QRadioButton::indicator { width: 14px; height: 14px; }"
        "QCheckBox::indicator { border: 1px solid #8F8F8F; border-radius: 3px; background: #262423; }"
        "QCheckBox::indicator:checked { background: #E87808; border-color: #E87808; }"
        "QCheckBox::indicator:indeterminate { background: #8F8F8F; }"
        "QRadioButton::indicator { border: 1px solid #8F8F8F; border-radius: 7px; background: #262423; }"
        "QRadioButton::indicator:checked { background: #E87808; border-color: #E87808; }"

        "QGroupBox {"
        "   background-color: transparent; color: #ECECEC;"
        "   border: 1px solid #2B2927; border-radius: 8px; margin-top: 14px; padding-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; color: #8F8F8F; }"
        "QGroupBox::indicator { width: 14px; height: 14px; border: 1px solid #8F8F8F;"
        "   border-radius: 3px; background: #262423; }"
        "QGroupBox::indicator:checked { background: #E87808; border-color: #E87808; }"

        "QTabWidget::pane { border: none; }"
        "QTabWidget::tab-bar { left: 0px; }"
        "QTabBar::tab {"
        "   background-color: transparent; color: #8F8F8F;"
        "   padding: 7px 14px; border-bottom: 2px solid transparent; }"
        "QTabBar::tab:selected { color: #FFFFFF; border-bottom-color: #E87808; }"
        "QTabBar::tab:hover:!selected { color: #CFCFCF; }"

        "QListWidget, QListView, QTreeWidget, QTreeView, QTableWidget, QTableView {"
        "   background-color: #141312; color: #ECECEC;"
        "   border: 1px solid #2B2927; border-radius: 6px; }"
        "QListWidget::item, QListView::item, QTreeWidget::item, QTreeView::item,"
        "QTableWidget::item, QTableView::item {"
        "   background-color: transparent; color: #ECECEC; padding: 3px; }"
        "QListWidget::item:selected, QListView::item:selected,"
        "QTreeWidget::item:selected, QTreeView::item:selected,"
        "QTableWidget::item:selected, QTableView::item:selected {"
        "   background-color: #353230; color: #FFFFFF; }"
        "QListWidget::item:hover, QTreeWidget::item:hover, QTableWidget::item:hover {"
        "   background-color: #242220; }"
        "QHeaderView::section {"
        "   background-color: #1B1B1B; color: #8F8F8F; border: none;"
        "   border-bottom: 1px solid #2B2927; padding: 5px 8px; }"

        "QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: #3A3836; min-height: 24px; border-radius: 5px; }"
        "QScrollBar::handle:vertical:hover { background: #4A4744; }"
        "QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }"
        "QScrollBar::handle:horizontal { background: #3A3836; min-width: 24px; border-radius: 5px; }"
        "QScrollBar::handle:horizontal:hover { background: #4A4744; }"
        "QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }"
        "QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }"

        "QMenu { background-color: #1B1B1B; border: 1px solid #2B2927; border-radius: 10px; padding: 6px 0px; }"
        "QMenu::item { color: #ECECEC; background-color: transparent;"
        "   padding: 7px 30px 7px 16px; border-radius: 6px; margin: 0px 6px; }"
        "QMenu::item:selected { background-color: rgba(255,255,255,0.09); color: #FFFFFF; }"
        "QMenu::item:disabled { color: #6F6F6F; }"
        "QMenu::separator { height: 1px; background: #2B2927; margin: 4px 12px; }"
        "QMenu::indicator { width: 16px; height: 16px; }"

        "QProgressBar { background-color: #262423; border: none; border-radius: 4px;"
        "   text-align: center; color: #ECECEC; max-height: 8px; }"
        "QProgressBar::chunk { background-color: #E87808; border-radius: 4px; }"

        "QSlider::groove:horizontal { height: 6px; background: #3A3836; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 14px; height: 14px; margin: -4px 0;"
        "   background: #E87808; border-radius: 7px; }"
        "QSlider::sub-page:horizontal { background: #E87808; border-radius: 3px; }"
        "QSlider::groove:vertical { width: 6px; background: #3A3836; border-radius: 3px; }"
        "QSlider::handle:vertical { width: 14px; height: 14px; margin: 0 -4px;"
        "   background: #E87808; border-radius: 7px; }"
        "QSlider::add-page:vertical { background: #E87808; border-radius: 3px; }"

        "QSplitter::handle { background: #2B2927; }"
        "QSplitter::handle:horizontal { width: 1px; }"
        "QSplitter::handle:vertical { height: 1px; }"

        "QDialogButtonBox { button-layout: 2; }"
        "QStatusBar { background: #1B1B1B; color: #8F8F8F; }"
        "QFrame[frameShape=\"4\"], QFrame[frameShape=\"5\"] { color: #2B2927; }"
    );

    return style
        .replace(QStringLiteral("#E87808"), validAccent.name(QColor::HexRgb).toUpper())
        .replace(QStringLiteral("#F08A22"), validAccent.lighter(110).name(QColor::HexRgb).toUpper())
        .replace(QStringLiteral("#C96608"), validAccent.darker(115).name(QColor::HexRgb).toUpper())
        .replace(QStringLiteral("#3A2A18"), validAccent.darker(160).name(QColor::HexRgb).toUpper());
}

#endif /* VLC_QT_VLCDARKSTYLE_HPP_ */
