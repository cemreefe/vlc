/*****************************************************************************
 * subtitle_style_settings.hpp : QML-facing subtitle appearance settings
 *****************************************************************************/

#pragma once

#include "qt.hpp"

#include <QObject>
#include <QColor>
#include <QStringList>
#include <QSettings>

class SubtitleStyleSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList presets READ presets NOTIFY presetsChanged FINAL)
    Q_PROPERTY(QString preset READ preset WRITE setPreset NOTIFY presetChanged FINAL)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY styleChanged FINAL)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY styleChanged FINAL)
    Q_PROPERTY(QColor outlineColor READ outlineColor WRITE setOutlineColor NOTIFY styleChanged FINAL)
    Q_PROPERTY(int outlineThickness READ outlineThickness WRITE setOutlineThickness NOTIFY styleChanged FINAL)
    Q_PROPERTY(int backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity NOTIFY styleChanged FINAL)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY styleChanged FINAL)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY styleChanged FINAL)

public:
    explicit SubtitleStyleSettings(QObject* parent = nullptr);

    QStringList presets() const;
    QString preset() const;
    void setPreset(const QString& preset);

    QString fontFamily() const;
    void setFontFamily(const QString& font);

    QColor textColor() const;
    void setTextColor(const QColor& color);

    QColor outlineColor() const;
    void setOutlineColor(const QColor& color);

    int outlineThickness() const;
    void setOutlineThickness(int value);

    int backgroundOpacity() const;
    void setBackgroundOpacity(int value);

    int offset() const;
    void setOffset(int value);

    int fontSize() const;
    void setFontSize(int value);

    Q_INVOKABLE void saveCurrentAsPreset(const QString& name);

signals:
    void presetChanged();
    void presetsChanged();
    void styleChanged();

private:
    struct Style {
        QString fontFamily;
        QColor textColor;
        QColor outlineColor;
        int outlineThickness;
        int backgroundOpacity;
        int offset;
        int fontSize;
    };

    Style currentStyle() const;
    void applyStyle(const Style& style);
    void savePreset(const QString& name, const Style& style);
    Style loadPreset(const QString& name) const;

    void applyPreset(const QString& name);

    QString m_preset;
    QSettings m_store;
};
