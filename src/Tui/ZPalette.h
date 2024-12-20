// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZPALETTE_INCLUDED
#define TUIWIDGETS_ZPALETTE_INCLUDED

#include <memory>

#include <QSet>
#include <QString>
#include <QStringList>

#include <Tui/ZColor.h>
#include <Tui/ZCommon.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class ZPalettePrivate;

class TUIWIDGETS_EXPORT ZPalette {
public:
    enum Type : int {
        Publish,
        Local
    };

    struct ColorDef {
        ZImplicitSymbol name;
        ZColor color;
    };

    struct AttributeDef {
        ZImplicitSymbol name;
        ZTextAttributes attributes;
    };

    struct AliasDef {
        ZImplicitSymbol name;
        ZImplicitSymbol fallback;
    };

    struct RuleCmd {
        Type type;
        ZImplicitSymbol name;
        ZImplicitSymbol reference;
    };

    struct RuleDef {
        QSet<QString> classes;
        QList<RuleCmd> cmds;
    };

public:
    ZPalette();
    ZPalette(const ZPalette &other);
    virtual ~ZPalette();

    static ZPalette classic();
    static ZPalette black();

public:
    static ZColor getColor(ZWidget *targetWidget, ZImplicitSymbol x);
    static ZTextAttributes getAttributes(ZWidget *targetWidget, ZImplicitSymbol x);

    void setColors(QList<ColorDef> newColors);
    //void addLocalAlias(QList<AliasDef> newAliases);
    void addRules(QList<RuleDef> newRules);
    void setAttributes(QList<AttributeDef>);

    ZPalette& operator=(const ZPalette& other);

    bool isNull() const;

protected:
    ZValuePtr<ZPalettePrivate> tuiwidgets_pimpl_ptr;

public:
    static void setDefaultRules(ZPalette &p);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPalette)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPALETTE_INCLUDED
