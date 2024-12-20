// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZPALETTE_P_INCLUDED
#define TUIWIDGETS_ZPALETTE_P_INCLUDED

#include <QHash>

#include <Tui/ZPalette.h>

#include <Tui/tuiwidgets_internal.h>


TUIWIDGETS_NS_START

class ZPalettePrivate {
public:
    ZPalettePrivate();
    virtual ~ZPalettePrivate();

    QHash<ZSymbol, ZColor> colorDefinitions;
    QHash<ZSymbol, ZTextAttributes> attributeDefinitions;
    QHash<ZSymbol, ZSymbol> localAlias;
    QList<ZPalette::RuleDef> rules;

    template<typename DefType, typename Getter> QHash<ZSymbol, DefType> getDefs(ZWidget *targetWidget, Getter getter) const;

    ZPalette *pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZPalette)
};

TUIWIDGETS_NS_END


#endif // TUIWIDGETS_ZPALETTE_P_INCLUDED
