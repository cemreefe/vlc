/*****************************************************************************
 * subtitle_style_settings.cpp : QML-facing subtitle appearance settings
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "subtitle_style_settings.hpp"

#include <vlc_configuration.h>
#include <vlc_variables.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

SubtitleStyleSettings::SubtitleStyleSettings(QObject* parent)
    : QObject(parent)
    , m_store("vlc", "vlc-subtitle-styles")
{
    m_preset = m_store.value("preset", QStringLiteral("Default")).toString();
}

static const QStringList kDefaultPresets = {
    QStringLiteral("Default"),
    QStringLiteral("Custom 01"),
    QStringLiteral("Custom 02"),
    QStringLiteral("Anime"),
    QStringLiteral("Loud and then Quiet Movies"),
    QStringLiteral("The Wand Subtitle Preset"),
};

QStringList SubtitleStyleSettings::presets() const
{
    const QByteArray raw = m_store.value("names").toByteArray();
    QStringList names;
    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (doc.isArray()) {
        for (const QJsonValue& v : doc.array())
            names << v.toString();
    }
    if (names.isEmpty())
        names = kDefaultPresets;
    names.prepend(QStringLiteral("New Subtitle Preset"));
    names.removeDuplicates();
    return names;
}

QString SubtitleStyleSettings::preset() const { return m_preset; }

void SubtitleStyleSettings::setPreset(const QString& preset)
{
    if (preset == m_preset || preset.isEmpty() || preset == QStringLiteral("New Subtitle Preset")) {
        if (m_preset.isEmpty()) return;
        if (preset.isEmpty()) return;
    }
    m_preset = preset;
    m_store.setValue("preset", preset);
    applyPreset(preset);
    emit presetChanged();
}

static QString cfgStr(const char* name)
{
    char* v = config_GetPsz(name);
    const QString out = v ? QString::fromUtf8(v) : QString();
    free(v);
    return out;
}

SubtitleStyleSettings::Style SubtitleStyleSettings::currentStyle() const
{
    Style s;
    s.fontFamily       = cfgStr("freetype-font");
    s.textColor        = QColor(config_GetInt("freetype-color"));
    s.outlineColor     = QColor(config_GetInt("freetype-outline-color"));
    s.outlineThickness = config_GetInt("freetype-outline-thickness");
    s.backgroundOpacity = config_GetInt("freetype-background-opacity");
    s.offset           = config_GetInt("sub-margin");
    s.fontSize         = config_GetInt("freetype-rel-fontsize");
    return s;
}

void SubtitleStyleSettings::applyStyle(const Style& s)
{
    config_PutPsz("freetype-font", s.fontFamily.toUtf8().constData());
    config_PutInt("freetype-color", s.textColor.rgb() & 0xFFFFFF);
    config_PutInt("freetype-outline-color", s.outlineColor.rgb() & 0xFFFFFF);
    config_PutInt("freetype-outline-thickness", s.outlineThickness);
    config_PutInt("freetype-background-opacity", s.backgroundOpacity);
    config_PutInt("sub-margin", s.offset);
    config_PutInt("freetype-rel-fontsize", s.fontSize);
    config_SaveConfigFile(nullptr);
}

void SubtitleStyleSettings::savePreset(const QString& name, const Style& s)
{
    QJsonObject o;
    o["fontFamily"] = s.fontFamily;
    o["textColor"] = s.textColor.name(QColor::HexRgb);
    o["outlineColor"] = s.outlineColor.name(QColor::HexRgb);
    o["outlineThickness"] = s.outlineThickness;
    o["backgroundOpacity"] = s.backgroundOpacity;
    o["offset"] = s.offset;
    o["fontSize"] = s.fontSize;
    m_store.setValue(QStringLiteral("styles/") + name,
                     QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));

    QByteArray raw = m_store.value("names").toByteArray();
    QJsonArray names = QJsonDocument::fromJson(raw).array();
    names << name;
    QJsonArray dedup;
    QStringList seen;
    for (const QJsonValue& v : names) {
        const QString n = v.toString();
        if (!seen.contains(n)) { seen << n; dedup << n; }
    }
    m_store.setValue("names", QJsonDocument(dedup).toJson(QJsonDocument::Compact));
}

SubtitleStyleSettings::Style SubtitleStyleSettings::loadPreset(const QString& name) const
{
    const QString raw = m_store.value(QStringLiteral("styles/") + name).toString();
    const QJsonObject o = QJsonDocument::fromJson(raw.toUtf8()).object();
    Style s = currentStyle();
    if (o.isEmpty()) return s;
    if (o.contains("fontFamily")) s.fontFamily = o["fontFamily"].toString();
    if (o.contains("textColor")) s.textColor = QColor(o["textColor"].toString());
    if (o.contains("outlineColor")) s.outlineColor = QColor(o["outlineColor"].toString());
    if (o.contains("outlineThickness")) s.outlineThickness = o["outlineThickness"].toInt();
    if (o.contains("backgroundOpacity")) s.backgroundOpacity = o["backgroundOpacity"].toInt();
    if (o.contains("offset")) s.offset = o["offset"].toInt();
    if (o.contains("fontSize")) s.fontSize = o["fontSize"].toInt();
    return s;
}

void SubtitleStyleSettings::applyPreset(const QString& name)
{
    applyStyle(loadPreset(name));
    emit styleChanged();
}

void SubtitleStyleSettings::saveCurrentAsPreset(const QString& name)
{
    if (name.isEmpty()) return;
    savePreset(name, currentStyle());
    emit presetsChanged();
    m_preset = name;
    m_store.setValue("preset", name);
    emit presetChanged();
}

QString SubtitleStyleSettings::fontFamily() const { return cfgStr("freetype-font"); }
void SubtitleStyleSettings::setFontFamily(const QString& font)
{
    config_PutPsz("freetype-font", font.toUtf8().constData());
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}

QColor SubtitleStyleSettings::textColor() const { return QColor(config_GetInt("freetype-color")); }
void SubtitleStyleSettings::setTextColor(const QColor& c)
{
    config_PutInt("freetype-color", c.rgb() & 0xFFFFFF);
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}

QColor SubtitleStyleSettings::outlineColor() const { return QColor(config_GetInt("freetype-outline-color")); }
void SubtitleStyleSettings::setOutlineColor(const QColor& c)
{
    config_PutInt("freetype-outline-color", c.rgb() & 0xFFFFFF);
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}

int SubtitleStyleSettings::outlineThickness() const { return config_GetInt("freetype-outline-thickness"); }
void SubtitleStyleSettings::setOutlineThickness(int v)
{
    config_PutInt("freetype-outline-thickness", v);
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}

int SubtitleStyleSettings::backgroundOpacity() const { return config_GetInt("freetype-background-opacity"); }
void SubtitleStyleSettings::setBackgroundOpacity(int v)
{
    config_PutInt("freetype-background-opacity", v);
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}

int SubtitleStyleSettings::offset() const { return config_GetInt("sub-margin"); }
void SubtitleStyleSettings::setOffset(int v)
{
    config_PutInt("sub-margin", v);
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}

int SubtitleStyleSettings::fontSize() const { return config_GetInt("freetype-rel-fontsize"); }
void SubtitleStyleSettings::setFontSize(int v)
{
    config_PutInt("freetype-rel-fontsize", v);
    config_SaveConfigFile(nullptr);
    emit styleChanged();
}
