// SPDX-License-Identifier: BSL-1.0

#include "ZPalette.h"
#include "ZPalette_p.h"

#include <QMap>

#include <Tui/ZColor.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

ZPalettePrivate::ZPalettePrivate() {

}

ZPalettePrivate::~ZPalettePrivate() {
}

struct ColorDefAccess {
    const QHash<ZSymbol, ZColor>& get(const ZPalettePrivate &pal) {
        return pal.colorDefinitions;
    }
};

struct AttributeDefAccess {
    const QHash<ZSymbol, ZTextAttributes>& get(const ZPalettePrivate &pal) {
        return pal.attributeDefinitions;
    }
};

template<typename DefType, typename Getter> QHash<ZSymbol, DefType> ZPalettePrivate::getDefs(ZWidget *targetWidget, Getter access) const
{
    QList<ZWidget*> widgets;
    {
        ZWidget *w = targetWidget;
        while (w) {
            widgets.prepend(w);
            w = w->parentWidget();
        }
    }

    QHash<ZSymbol, DefType> defs;
    QList<const ZPalette::RuleDef*> rules;

    for (ZWidget *w : widgets) {
        QSet<QString> widgetClasses = w->paletteClass().toSet();

        const ZPalettePrivate &pal = *w->palette().tuiwidgets_pimpl_ptr;

        for (const auto &rule : qAsConst(pal.rules)) {
            rules.append(&rule);
        }

        QMap<int, QList<const ZPalette::RuleDef*>> matchingRulesByLen;

        for (const ZPalette::RuleDef *rule : rules) {
            if (widgetClasses.contains(rule->classes)) {
                matchingRulesByLen[rule->classes.size()].append(rule);
            }
        }

        for (auto &rs : qAsConst(matchingRulesByLen)) {
            for (const ZPalette::RuleDef *r : qAsConst(rs)) {
                for (const ZPalette::RuleCmd &cmd : r->cmds) {
                    if (cmd.type == ZPalette::Publish || w == targetWidget) {
                        if (defs.contains(cmd.reference)) {
                            defs[cmd.name] = defs[cmd.reference];
                        }
                    }
                }
            }
        }

        for (ZSymbol key : access.get(pal).keys()) {
            defs[key] = access.get(pal)[key];
        }
    }

    return defs;
}
template<> QHash<ZSymbol, ZPalette::ColorDef> ZPalettePrivate::getDefs<>(ZWidget *targetWidget, ColorDefAccess access) const;
template<> QHash<ZSymbol, ZPalette::AttributeDef> ZPalettePrivate::getDefs<>(ZWidget *targetWidget, AttributeDefAccess access) const;


ZPalette::ZPalette() {
}

ZPalette::ZPalette(const ZPalette &other) = default;

ZPalette::~ZPalette() {

}

ZColor ZPalette::getColor(ZWidget *targetWidget, ZImplicitSymbol x) {
    // TODO Most stupid impl., needs optimization

    const QHash<ZSymbol, ZColor> defs = targetWidget->palette().tuiwidgets_pimpl_ptr.get()->getDefs<ZColor, ColorDefAccess>(targetWidget, ColorDefAccess());

    if (defs.contains(x)) {
        return defs.value(x);
    }

    return {0xff, 0, 0};
}

ZTextAttributes ZPalette::getAttributes(ZWidget *targetWidget, ZImplicitSymbol x)
{
    const QHash<ZSymbol, ZTextAttributes> defs = targetWidget->palette().tuiwidgets_pimpl_ptr.get()->getDefs<ZTextAttributes, AttributeDefAccess>(targetWidget, AttributeDefAccess());

    if (defs.contains(x)) {
        return defs.value(x);
    }

    return {};
}

void ZPalette::setAttributes(QList<AttributeDef> newAttributes)
{
    auto *const p = tuiwidgets_impl();

    for (const auto &na : newAttributes) {
        p->attributeDefinitions[na.name] = na.attributes;
    }
}

void ZPalette::setColors(QList<ZPalette::ColorDef> newColors) {
    auto *const p = tuiwidgets_impl();

    for (const auto &nc : newColors) {
        p->colorDefinitions[nc.name] = nc.color;
    }
}

//void ZPalette::addLocalAlias(QList<ZPalette::AliasDef> newAliases) {
//    auto *const p = tuiwidgets_impl();
//
//    for (const auto &nc : newAliases) {
//        p->localAlias[nc.name] = nc.fallback;
//    }
//}

void ZPalette::addRules(QList<ZPalette::RuleDef> newRules) {
    auto *const p = tuiwidgets_impl();

    p->rules += newRules;
}

bool ZPalette::isNull() const {
    auto *const p = tuiwidgets_impl();

    return p->colorDefinitions.isEmpty() && p->rules.isEmpty() && p->localAlias.isEmpty();
}

ZPalette &ZPalette::operator=(const ZPalette &other) = default;

void ZPalette::setDefaultRules(ZPalette &p) {
    p.addRules({
        {{ QStringLiteral("window") }, {
             { p.Publish, "bg", "window.bg" },
             { p.Publish, "attrs", "window.attrs" },

             { p.Publish, "window.bg", "window.default.bg" },
             { p.Publish, "window.attrs", "window.default.attrs" },
             { p.Publish, "window.frame.focused.bg", "window.bg" },
             { p.Publish, "window.frame.focused.fg", "window.default.frame.focused.fg" },
             { p.Publish, "window.frame.focused.attrs", "window.attrs" },
             { p.Publish, "window.frame.focused.control.bg", "window.bg" },
             { p.Publish, "window.frame.focused.control.fg", "window.default.frame.focused.control.fg" },
             { p.Publish, "window.frame.focused.control.attrs", "window.attrs" },
             { p.Publish, "window.frame.unfocused.bg", "window.bg" },
             { p.Publish, "window.frame.unfocused.fg", "window.default.frame.unfocused.fg" },
             { p.Publish, "window.frame.unfocused.attrs", "window.attrs" },

             { p.Publish, "scrollbar.bg", "window.default.scrollbar.bg" },
             { p.Publish, "scrollbar.fg", "window.default.scrollbar.fg" },
             { p.Publish, "scrollbar.attrs", "window.default.scrollbar.attrs" },
             { p.Publish, "scrollbar.control.bg", "window.default.scrollbar.control.bg" },
             { p.Publish, "scrollbar.control.fg", "window.default.scrollbar.control.fg" },
             { p.Publish, "scrollbar.control.attrs", "window.default.scrollbar.control.attrs" },

             { p.Publish, "text.bg", "window.default.text.bg" },
             { p.Publish, "text.fg", "window.default.text.fg" },
             { p.Publish, "text.attrs", "window.default.text.attrs" },
             { p.Publish, "text.selected.bg", "window.default.text.selected.bg" },
             { p.Publish, "text.selected.fg", "window.default.text.selected.fg" },
             { p.Publish, "text.selected.attrs", "window.default.text.selected.attrs" },

             { p.Publish, "control.bg", "window.default.control.bg" },
             { p.Publish, "control.fg", "window.default.control.fg" },
             { p.Publish, "control.attrs", "window.default.control.attrs" },
             { p.Publish, "control.focused.bg", "window.default.control.focused.bg" },
             { p.Publish, "control.focused.fg", "window.default.control.focused.fg" },
             { p.Publish, "control.focused.attrs", "window.default.control.focused.attrs" },
             { p.Publish, "control.disabled.bg", "window.default.control.disabled.bg" },
             { p.Publish, "control.disabled.fg", "window.default.control.disabled.fg" },
             { p.Publish, "control.disabled.attrs", "window.default.control.disabled.attrs" },
             { p.Publish, "control.shortcut.bg", "window.default.control.shortcut.bg" },
             { p.Publish, "control.shortcut.fg", "window.default.control.shortcut.fg" },
             { p.Publish, "control.shortcut.attrs", "window.default.control.shortcut.attrs" },

             { p.Publish, "dataview.bg", "window.default.dataview.bg" },
             { p.Publish, "dataview.fg", "window.default.dataview.fg" },
             { p.Publish, "dataview.attrs", "window.default.dataview.attrs" },
             { p.Publish, "dataview.selected.bg", "window.default.dataview.selected.bg" },
             { p.Publish, "dataview.selected.fg", "window.default.dataview.selected.fg" },
             { p.Publish, "dataview.selected.attrs", "window.default.dataview.selected.attrs" },
             { p.Publish, "dataview.selected.focused.bg", "window.default.dataview.selected.focused.bg" },
             { p.Publish, "dataview.selected.focused.fg", "window.default.dataview.selected.focused.fg" },
             { p.Publish, "dataview.selected.focused.attrs", "window.default.dataview.selected.focused.attrs" },
             { p.Publish, "dataview.disabled.bg", "window.default.dataview.disabled.bg" },
             { p.Publish, "dataview.disabled.fg", "window.default.dataview.disabled.fg" },
             { p.Publish, "dataview.disabled.attrs", "window.default.dataview.disabled.attrs" },
             { p.Publish, "dataview.disabled.selected.bg", "window.default.dataview.disabled.selected.bg" },
             { p.Publish, "dataview.disabled.selected.fg", "window.default.dataview.disabled.selected.fg" },
             { p.Publish, "dataview.disabled.selected.attrs", "window.default.dataview.disabled.selected.attrs" },

             { p.Publish, "button.bg", "window.default.button.bg" },
             { p.Publish, "button.fg", "window.default.button.fg" },
             { p.Publish, "button.attrs", "window.default.button.attrs" },
             { p.Publish, "button.default.bg", "window.default.button.default.bg" },
             { p.Publish, "button.default.fg", "window.default.button.default.fg" },
             { p.Publish, "button.default.attrs", "window.default.button.default.attrs" },
             { p.Publish, "button.focused.bg", "window.default.button.focused.bg" },
             { p.Publish, "button.focused.fg", "window.default.button.focused.fg" },
             { p.Publish, "button.focused.attrs", "window.default.button.focused.attrs" },
             { p.Publish, "button.disabled.bg", "window.default.button.disabled.bg" },
             { p.Publish, "button.disabled.fg", "window.default.button.disabled.fg" },
             { p.Publish, "button.disabled.attrs", "window.default.button.disabled.attrs" },
             { p.Publish, "button.shortcut.bg", "window.default.button.shortcut.bg" },
             { p.Publish, "button.shortcut.fg", "window.default.button.shortcut.fg" },
             { p.Publish, "button.shortcut.attrs", "window.default.button.shortcut.attrs" },

             { p.Publish, "lineedit.bg", "window.default.lineedit.bg" },
             { p.Publish, "lineedit.fg", "window.default.lineedit.fg" },
             { p.Publish, "lineedit.attrs", "window.default.lineedit.attrs" },
             { p.Publish, "lineedit.focused.bg", "window.default.lineedit.focused.bg" },
             { p.Publish, "lineedit.focused.fg", "window.default.lineedit.focused.fg" },
             { p.Publish, "lineedit.focused.attrs", "window.default.lineedit.focused.attrs" },
             { p.Publish, "lineedit.disabled.bg", "window.default.lineedit.disabled.bg" },
             { p.Publish, "lineedit.disabled.fg", "window.default.lineedit.disabled.fg" },
             { p.Publish, "lineedit.disabled.attrs", "window.default.lineedit.disabled.attrs" },

             { p.Publish, "textedit.bg", "window.default.textedit.bg" },
             { p.Publish, "textedit.fg", "window.default.textedit.fg" },
             { p.Publish, "textedit.attrs", "window.default.textedit.attrs" },
             { p.Publish, "textedit.focused.bg", "window.default.textedit.focused.bg" },
             { p.Publish, "textedit.focused.fg", "window.default.textedit.focused.fg" },
             { p.Publish, "textedit.focused.attrs", "window.default.textedit.focused.attrs" },
             { p.Publish, "textedit.disabled.bg", "window.default.textedit.disabled.bg" },
             { p.Publish, "textedit.disabled.fg", "window.default.textedit.disabled.fg" },
             { p.Publish, "textedit.disabled.attrs", "window.default.textedit.disabled.attrs" },
             { p.Publish, "textedit.selected.bg", "window.default.textedit.selected.bg" },
             { p.Publish, "textedit.selected.fg", "window.default.textedit.selected.fg" },
             { p.Publish, "textedit.selected.attrs", "window.default.textedit.selected.attrs" },
             { p.Publish, "textedit.linenumber.bg", "window.default.textedit.linenumber.bg" },
             { p.Publish, "textedit.linenumber.fg", "window.default.textedit.linenumber.fg" },
             { p.Publish, "textedit.linenumber.attrs", "window.default.textedit.linenumber.attrs" },
             { p.Publish, "textedit.focused.linenumber.bg", "window.default.textedit.focused.linenumber.bg" },
             { p.Publish, "textedit.focused.linenumber.fg", "window.default.textedit.focused.linenumber.fg" },
             { p.Publish, "textedit.focused.linenumber.attrs", "window.default.textedit.focused.linenumber.attrs" },
        }},

        {{ QStringLiteral("window"), QStringLiteral("dialog") }, {
             { p.Publish, "bg", "window.bg" },
             { p.Publish, "attrs", "window.attrs" },

             { p.Publish, "window.bg", "window.gray.bg" },
             { p.Publish, "window.attrs", "window.gray.attrs" },
             { p.Publish, "window.frame.focused.bg", "window.bg" },
             { p.Publish, "window.frame.focused.fg", "window.gray.frame.focused.fg" },
             { p.Publish, "window.frame.focused.attrs", "window.attrs" },
             { p.Publish, "window.frame.focused.control.bg", "window.bg" },
             { p.Publish, "window.frame.focused.control.fg", "window.gray.frame.focused.control.fg" },
             { p.Publish, "window.frame.focused.control.attrs", "window.attrs" },
             { p.Publish, "window.frame.unfocused.bg", "window.bg" },
             { p.Publish, "window.frame.unfocused.fg", "window.gray.frame.unfocused.fg" },
             { p.Publish, "window.frame.unfocused.attrs", "window.attrs" },

             { p.Publish, "scrollbar.bg", "window.gray.scrollbar.control.bg" },
             { p.Publish, "scrollbar.fg", "window.gray.scrollbar.control.fg" },
             { p.Publish, "scrollbar.attrs", "window.gray.scrollbar.control.attrs" },
             { p.Publish, "scrollbar.control.bg", "window.gray.scrollbar.control.bg" },
             { p.Publish, "scrollbar.control.fg", "window.gray.scrollbar.control.fg" },
             { p.Publish, "scrollbar.control.attrs", "window.gray.scrollbar.control.attrs" },

             { p.Publish, "text.bg", "window.gray.text.bg" },
             { p.Publish, "text.fg", "window.gray.text.fg" },
             { p.Publish, "text.attrs", "window.gray.text.attrs" },
             { p.Publish, "text.selected.bg", "window.gray.text.selected.bg" },
             { p.Publish, "text.selected.fg", "window.gray.text.selected.fg" },
             { p.Publish, "text.selected.attrs", "window.gray.text.selected.attrs" },

             { p.Publish, "control.bg", "window.gray.control.bg" },
             { p.Publish, "control.fg", "window.gray.control.fg" },
             { p.Publish, "control.attrs", "window.gray.control.attrs" },
             { p.Publish, "control.focused.bg", "window.gray.control.focused.bg" },
             { p.Publish, "control.focused.fg", "window.gray.control.focused.fg" },
             { p.Publish, "control.focused.attrs", "window.gray.control.focused.attrs" },
             { p.Publish, "control.disabled.bg", "window.gray.control.disabled.bg" },
             { p.Publish, "control.disabled.fg", "window.gray.control.disabled.fg" },
             { p.Publish, "control.disabled.attrs", "window.gray.control.disabled.attrs" },
             { p.Publish, "control.shortcut.bg", "window.gray.control.shortcut.bg" },
             { p.Publish, "control.shortcut.fg", "window.gray.control.shortcut.fg" },
             { p.Publish, "control.shortcut.attrs", "window.gray.control.shortcut.attrs" },

             { p.Publish, "dataview.bg", "window.gray.dataview.bg" },
             { p.Publish, "dataview.fg", "window.gray.dataview.fg" },
             { p.Publish, "dataview.attrs", "window.gray.dataview.attrs" },
             { p.Publish, "dataview.selected.bg", "window.gray.dataview.selected.bg" },
             { p.Publish, "dataview.selected.fg", "window.gray.dataview.selected.fg" },
             { p.Publish, "dataview.selected.attrs", "window.gray.dataview.selected.attrs" },
             { p.Publish, "dataview.selected.focused.bg", "window.gray.dataview.selected.focused.bg" },
             { p.Publish, "dataview.selected.focused.fg", "window.gray.dataview.selected.focused.fg" },
             { p.Publish, "dataview.selected.focused.attrs", "window.gray.dataview.selected.focused.attrs" },
             { p.Publish, "dataview.disabled.bg", "window.gray.dataview.disabled.bg" },
             { p.Publish, "dataview.disabled.fg", "window.gray.dataview.disabled.fg" },
             { p.Publish, "dataview.disabled.attrs", "window.gray.dataview.disabled.attrs" },
             { p.Publish, "dataview.disabled.selected.bg", "window.gray.dataview.disabled.selected.bg" },
             { p.Publish, "dataview.disabled.selected.fg", "window.gray.dataview.disabled.selected.fg" },
             { p.Publish, "dataview.disabled.selected.attrs", "window.gray.dataview.disabled.selected.attrs" },

             { p.Publish, "button.bg", "window.gray.button.bg" },
             { p.Publish, "button.fg", "window.gray.button.fg" },
             { p.Publish, "button.attrs", "window.gray.button.attrs" },
             { p.Publish, "button.default.bg", "window.gray.button.default.bg" },
             { p.Publish, "button.default.fg", "window.gray.button.default.fg" },
             { p.Publish, "button.default.attrs", "window.gray.button.default.attrs" },
             { p.Publish, "button.focused.bg", "window.gray.button.focused.bg" },
             { p.Publish, "button.focused.fg", "window.gray.button.focused.fg" },
             { p.Publish, "button.focused.attrs", "window.gray.button.focused.attrs" },
             { p.Publish, "button.disabled.bg", "window.gray.button.disabled.bg" },
             { p.Publish, "button.disabled.fg", "window.gray.button.disabled.fg" },
             { p.Publish, "button.disabled.attrs", "window.gray.button.disabled.attrs" },
             { p.Publish, "button.shortcut.bg", "window.gray.button.shortcut.bg" },
             { p.Publish, "button.shortcut.fg", "window.gray.button.shortcut.fg" },
             { p.Publish, "button.shortcut.attrs", "window.gray.button.shortcut.attrs" },

             { p.Publish, "lineedit.bg", "window.gray.lineedit.bg" },
             { p.Publish, "lineedit.fg", "window.gray.lineedit.fg" },
             { p.Publish, "lineedit.attrs", "window.gray.lineedit.attrs" },
             { p.Publish, "lineedit.focused.bg", "window.gray.lineedit.focused.bg" },
             { p.Publish, "lineedit.focused.fg", "window.gray.lineedit.focused.fg" },
             { p.Publish, "lineedit.focused.attrs", "window.gray.lineedit.focused.attrs" },
             { p.Publish, "lineedit.disabled.bg", "window.gray.lineedit.disabled.bg" },
             { p.Publish, "lineedit.disabled.fg", "window.gray.lineedit.disabled.fg" },
             { p.Publish, "lineedit.disabled.attrs", "window.gray.lineedit.disabled.attrs" },

             { p.Publish, "textedit.bg", "window.gray.textedit.bg" },
             { p.Publish, "textedit.fg", "window.gray.textedit.fg" },
             { p.Publish, "textedit.attrs", "window.gray.textedit.attrs" },
             { p.Publish, "textedit.focused.bg", "window.gray.textedit.focused.bg" },
             { p.Publish, "textedit.focused.fg", "window.gray.textedit.focused.fg" },
             { p.Publish, "textedit.focused.attrs", "window.gray.textedit.focused.attrs" },
             { p.Publish, "textedit.disabled.bg", "window.gray.textedit.disabled.bg" },
             { p.Publish, "textedit.disabled.fg", "window.gray.textedit.disabled.fg" },
             { p.Publish, "textedit.disabled.attrs", "window.gray.textedit.disabled.attrs" },
             { p.Publish, "textedit.selected.bg", "window.gray.textedit.selected.bg" },
             { p.Publish, "textedit.selected.fg", "window.gray.textedit.selected.fg" },
             { p.Publish, "textedit.selected.attrs", "window.gray.textedit.selected.attrs" },
             { p.Publish, "textedit.linenumber.bg", "window.gray.textedit.linenumber.bg" },
             { p.Publish, "textedit.linenumber.fg", "window.gray.textedit.linenumber.fg" },
             { p.Publish, "textedit.linenumber.attrs", "window.gray.textedit.linenumber.attrs" },
             { p.Publish, "textedit.focused.linenumber.bg", "window.gray.textedit.focused.linenumber.bg" },
             { p.Publish, "textedit.focused.linenumber.fg", "window.gray.textedit.focused.linenumber.fg" },
             { p.Publish, "textedit.focused.linenumber.attrs", "window.gray.textedit.focused.linenumber.attrs" },
       }},

       {{ QStringLiteral("window"), QStringLiteral("cyan") }, {
            { p.Publish, "bg", "window.bg" },
            { p.Publish, "attrs", "window.attrs" },

            { p.Publish, "window.bg", "window.cyan.bg" },
            { p.Publish, "window.attrs", "window.cyan.attrs" },
            { p.Publish, "window.frame.focused.bg", "window.bg" },
            { p.Publish, "window.frame.focused.fg", "window.cyan.frame.focused.fg" },
            { p.Publish, "window.frame.focused.attrs", "window.attrs" },
            { p.Publish, "window.frame.focused.control.bg", "window.bg" },
            { p.Publish, "window.frame.focused.control.fg", "window.cyan.frame.focused.control.fg" },
            { p.Publish, "window.frame.focused.control.attrs", "window.attrs" },
            { p.Publish, "window.frame.unfocused.bg", "window.bg" },
            { p.Publish, "window.frame.unfocused.fg", "window.cyan.frame.unfocused.fg" },
            { p.Publish, "window.frame.unfocused.attrs", "window.attrs" },

            { p.Publish, "scrollbar.bg", "window.cyan.scrollbar.control.bg" },
            { p.Publish, "scrollbar.fg", "window.cyan.scrollbar.control.fg" },
            { p.Publish, "scrollbar.attrs", "window.cyan.scrollbar.control.attrs" },
            { p.Publish, "scrollbar.control.bg", "window.cyan.scrollbar.control.bg" },
            { p.Publish, "scrollbar.control.fg", "window.cyan.scrollbar.control.fg" },
            { p.Publish, "scrollbar.control.attrs", "window.cyan.scrollbar.control.attrs" },

            { p.Publish, "text.bg", "window.cyan.text.bg" },
            { p.Publish, "text.fg", "window.cyan.text.fg" },
            { p.Publish, "text.attrs", "window.cyan.text.attrs" },
            { p.Publish, "text.selected.bg", "window.cyan.text.selected.bg" },
            { p.Publish, "text.selected.fg", "window.cyan.text.selected.fg" },
            { p.Publish, "text.selected.attrs", "window.cyan.text.selected.attrs" },

            { p.Publish, "control.bg", "window.cyan.control.bg" },
            { p.Publish, "control.fg", "window.cyan.control.fg" },
            { p.Publish, "control.attrs", "window.cyan.control.attrs" },
            { p.Publish, "control.focused.bg", "window.cyan.control.focused.bg" },
            { p.Publish, "control.focused.fg", "window.cyan.control.focused.fg" },
            { p.Publish, "control.focused.attrs", "window.cyan.control.focused.attrs" },
            { p.Publish, "control.disabled.bg", "window.cyan.control.disabled.bg" },
            { p.Publish, "control.disabled.fg", "window.cyan.control.disabled.fg" },
            { p.Publish, "control.disabled.attrs", "window.cyan.control.disabled.attrs" },
            { p.Publish, "control.shortcut.bg", "window.cyan.control.shortcut.bg" },
            { p.Publish, "control.shortcut.fg", "window.cyan.control.shortcut.fg" },
            { p.Publish, "control.shortcut.attrs", "window.cyan.control.shortcut.attrs" },

            { p.Publish, "dataview.bg", "window.cyan.dataview.bg" },
            { p.Publish, "dataview.fg", "window.cyan.dataview.fg" },
            { p.Publish, "dataview.attrs", "window.cyan.dataview.attrs" },
            { p.Publish, "dataview.selected.bg", "window.cyan.dataview.selected.bg" },
            { p.Publish, "dataview.selected.fg", "window.cyan.dataview.selected.fg" },
            { p.Publish, "dataview.selected.attrs", "window.cyan.dataview.selected.attrs" },
            { p.Publish, "dataview.selected.focused.bg", "window.cyan.dataview.selected.focused.bg" },
            { p.Publish, "dataview.selected.focused.fg", "window.cyan.dataview.selected.focused.fg" },
            { p.Publish, "dataview.selected.focused.attrs", "window.cyan.dataview.selected.focused.attrs" },
            { p.Publish, "dataview.disabled.bg", "window.cyan.dataview.disabled.bg" },
            { p.Publish, "dataview.disabled.fg", "window.cyan.dataview.disabled.fg" },
            { p.Publish, "dataview.disabled.attrs", "window.cyan.dataview.disabled.attrs" },
            { p.Publish, "dataview.disabled.selected.bg", "window.cyan.dataview.disabled.selected.bg" },
            { p.Publish, "dataview.disabled.selected.fg", "window.cyan.dataview.disabled.selected.fg" },
            { p.Publish, "dataview.disabled.selected.attrs", "window.cyan.dataview.disabled.selected.attrs" },

            { p.Publish, "button.bg", "window.cyan.button.bg" },
            { p.Publish, "button.fg", "window.cyan.button.fg" },
            { p.Publish, "button.attrs", "window.cyan.button.attrs" },
            { p.Publish, "button.default.bg", "window.cyan.button.default.bg" },
            { p.Publish, "button.default.fg", "window.cyan.button.default.fg" },
            { p.Publish, "button.default.attrs", "window.cyan.button.default.attrs" },
            { p.Publish, "button.focused.bg", "window.cyan.button.focused.bg" },
            { p.Publish, "button.focused.fg", "window.cyan.button.focused.fg" },
            { p.Publish, "button.focused.attrs", "window.cyan.button.focused.attrs" },
            { p.Publish, "button.disabled.bg", "window.cyan.button.disabled.bg" },
            { p.Publish, "button.disabled.fg", "window.cyan.button.disabled.fg" },
            { p.Publish, "button.disabled.attrs", "window.cyan.button.disabled.attrs" },
            { p.Publish, "button.shortcut.bg", "window.cyan.button.shortcut.bg" },
            { p.Publish, "button.shortcut.fg", "window.cyan.button.shortcut.fg" },
            { p.Publish, "button.shortcut.attrs", "window.cyan.button.shortcut.attrs" },

            { p.Publish, "lineedit.bg", "window.cyan.lineedit.bg" },
            { p.Publish, "lineedit.fg", "window.cyan.lineedit.fg" },
            { p.Publish, "lineedit.attrs", "window.cyan.lineedit.attrs" },
            { p.Publish, "lineedit.focused.bg", "window.cyan.lineedit.focused.bg" },
            { p.Publish, "lineedit.focused.fg", "window.cyan.lineedit.focused.fg" },
            { p.Publish, "lineedit.focused.attrs", "window.cyan.lineedit.focused.attrs" },
            { p.Publish, "lineedit.disabled.bg", "window.cyan.lineedit.disabled.bg" },
            { p.Publish, "lineedit.disabled.fg", "window.cyan.lineedit.disabled.fg" },
            { p.Publish, "lineedit.disabled.attrs", "window.cyan.lineedit.disabled.attrs" },

            { p.Publish, "textedit.bg", "window.cyan.textedit.bg" },
            { p.Publish, "textedit.fg", "window.cyan.textedit.fg" },
            { p.Publish, "textedit.attrs", "window.cyan.textedit.attrs" },
            { p.Publish, "textedit.focused.bg", "window.cyan.textedit.focused.bg" },
            { p.Publish, "textedit.focused.fg", "window.cyan.textedit.focused.fg" },
            { p.Publish, "textedit.focused.attrs", "window.cyan.textedit.focused.attrs" },
            { p.Publish, "textedit.disabled.bg", "window.cyan.textedit.disabled.bg" },
            { p.Publish, "textedit.disabled.fg", "window.cyan.textedit.disabled.fg" },
            { p.Publish, "textedit.disabled.attrs", "window.cyan.textedit.disabled.attrs" },
            { p.Publish, "textedit.selected.bg", "window.cyan.textedit.selected.bg" },
            { p.Publish, "textedit.selected.fg", "window.cyan.textedit.selected.fg" },
            { p.Publish, "textedit.selected.attrs", "window.cyan.textedit.selected.attrs" },
            { p.Publish, "textedit.linenumber.bg", "window.cyan.textedit.linenumber.bg" },
            { p.Publish, "textedit.linenumber.fg", "window.cyan.textedit.linenumber.fg" },
            { p.Publish, "textedit.linenumber.attrs", "window.cyan.textedit.linenumber.attrs" },
            { p.Publish, "textedit.focused.linenumber.bg", "window.cyan.textedit.focused.linenumber.bg" },
            { p.Publish, "textedit.focused.linenumber.fg", "window.cyan.textedit.focused.linenumber.fg" },
            { p.Publish, "textedit.focused.linenumber.attrs", "window.cyan.textedit.focused.linenumber.attrs" },
       }},

    });
}

ZPalette ZPalette::classic() {
    ZPalette p;
    p.setColors({
        { "root.bg", Colors::black},
        { "root.fg", {0x80, 0x80, 0x80}},

        { "menu.bg", Colors::lightGray},
        { "menu.fg", Colors::black},
        { "menu.disabled.bg", Colors::lightGray},
        { "menu.disabled.fg", Colors::darkGray},
        { "menu.shortcut.bg", Colors::lightGray},
        { "menu.shortcut.fg", Colors::red},
        { "menu.selected.bg", Colors::green},
        { "menu.selected.fg", Colors::black},
        { "menu.selected.disabled.bg", Colors::green},
        { "menu.selected.disabled.fg", Colors::darkGray},
        { "menu.selected.shortcut.bg", Colors::green},
        { "menu.selected.shortcut.fg", Colors::red},

        { "window.default.bg", Colors::blue},
        { "window.default.frame.focused.fg", Colors::brightWhite},
        { "window.default.frame.focused.control.fg", Colors::brightGreen},
        { "window.default.frame.unfocused.fg", Colors::lightGray},
        { "window.default.scrollbar.fg", Colors::blue},
        { "window.default.scrollbar.bg", { 0, 0x55, 0xaa}},
        { "window.default.scrollbar.control.fg", Colors::blue},
        { "window.default.scrollbar.control.bg", Colors::cyan},
        { "window.default.text.fg", Colors::brightYellow},
        { "window.default.text.bg", Colors::blue},
        { "window.default.text.selected.fg", Colors::blue},
        { "window.default.text.selected.bg", Colors::lightGray},
        { "window.default.control.bg", Colors::blue},
        { "window.default.control.fg", Colors::lightGray},
        { "window.default.control.focused.bg", Colors::blue},
        { "window.default.control.focused.fg", Colors::brightWhite},
        { "window.default.control.disabled.bg", Colors::blue},
        { "window.default.control.disabled.fg", Colors::black},
        { "window.default.control.shortcut.bg", Colors::blue},
        { "window.default.control.shortcut.fg", Colors::brightYellow},
        { "window.default.dataview.bg", Colors::cyan},
        { "window.default.dataview.fg", Colors::black},
        { "window.default.dataview.selected.bg", Colors::cyan},
        { "window.default.dataview.selected.fg", Colors::brightYellow},
        { "window.default.dataview.selected.focused.bg", Colors::green},
        { "window.default.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.default.dataview.disabled.bg", Colors::lightGray},
        { "window.default.dataview.disabled.fg", Colors::darkGray},
        { "window.default.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.default.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.default.button.bg", Colors::green},
        { "window.default.button.fg", Colors::black},
        { "window.default.button.default.bg", Colors::green},
        { "window.default.button.default.fg", Colors::brightCyan},
        { "window.default.button.focused.bg", Colors::green},
        { "window.default.button.focused.fg", Colors::brightWhite},
        { "window.default.button.disabled.bg", Colors::lightGray},
        { "window.default.button.disabled.fg", Colors::darkGray},
        { "window.default.button.shortcut.bg", Colors::green},
        { "window.default.button.shortcut.fg", Colors::brightYellow},
        { "window.default.lineedit.bg", Colors::lightGray},
        { "window.default.lineedit.fg", Colors::black},
        { "window.default.lineedit.focused.bg", Colors::lightGray},
        { "window.default.lineedit.focused.fg", Colors::brightWhite},
        { "window.default.lineedit.disabled.bg", Colors::darkGray},
        { "window.default.lineedit.disabled.fg", Colors::lightGray},
        { "window.default.textedit.bg", Colors::lightGray},
        { "window.default.textedit.fg", Colors::black},
        { "window.default.textedit.focused.bg", Colors::lightGray},
        { "window.default.textedit.focused.fg", Colors::brightWhite},
        { "window.default.textedit.disabled.bg", Colors::darkGray},
        { "window.default.textedit.disabled.fg", Colors::lightGray},
        { "window.default.textedit.selected.bg", Colors::brightWhite},
        { "window.default.textedit.selected.fg", Colors::darkGray},
        { "window.default.textedit.linenumber.bg", Colors::darkGray},
        { "window.default.textedit.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
        { "window.default.textedit.focused.linenumber.bg", Colors::darkGray},
        { "window.default.textedit.focused.linenumber.fg", { 0xdd, 0xdd, 0xdd}},

        { "window.gray.bg", Colors::lightGray},
        { "window.gray.frame.focused.fg", Colors::brightWhite},
        { "window.gray.frame.focused.control.fg", Colors::brightGreen},
        { "window.gray.frame.unfocused.fg", Colors::black},
        { "window.gray.scrollbar.fg", Colors::cyan},
        { "window.gray.scrollbar.bg", Colors::blue},
        { "window.gray.scrollbar.control.fg", Colors::cyan},
        { "window.gray.scrollbar.control.bg", Colors::blue},
        { "window.gray.text.fg", Colors::lightGray},
        { "window.gray.text.bg", Colors::black},
        { "window.gray.text.selected.fg", Colors::lightGray},
        { "window.gray.text.selected.bg", Colors::brightWhite},
        { "window.gray.control.bg", Colors::lightGray},
        { "window.gray.control.fg", Colors::black},
        { "window.gray.control.focused.bg", Colors::lightGray},
        { "window.gray.control.focused.fg", Colors::brightWhite},
        { "window.gray.control.disabled.bg", Colors::lightGray},
        { "window.gray.control.disabled.fg", Colors::darkGray},
        { "window.gray.control.shortcut.bg", Colors::lightGray},
        { "window.gray.control.shortcut.fg", Colors::brightYellow},
        { "window.gray.dataview.bg", Colors::cyan},
        { "window.gray.dataview.fg", Colors::black},
        { "window.gray.dataview.selected.bg", Colors::cyan},
        { "window.gray.dataview.selected.fg", Colors::brightYellow},
        { "window.gray.dataview.selected.focused.bg", Colors::green},
        { "window.gray.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.gray.dataview.disabled.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.fg", Colors::lightGray},
        { "window.gray.dataview.disabled.selected.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.gray.button.bg", Colors::green},
        { "window.gray.button.fg", Colors::black},
        { "window.gray.button.default.bg", Colors::green},
        { "window.gray.button.default.fg", Colors::brightCyan},
        { "window.gray.button.focused.bg", Colors::green},
        { "window.gray.button.focused.fg", Colors::brightWhite},
        { "window.gray.button.disabled.bg", Colors::lightGray},
        { "window.gray.button.disabled.fg", Colors::darkGray},
        { "window.gray.button.shortcut.bg", Colors::green},
        { "window.gray.button.shortcut.fg", Colors::brightYellow},
        { "window.gray.lineedit.bg", Colors::blue},
        { "window.gray.lineedit.fg", Colors::brightWhite},
        { "window.gray.lineedit.focused.bg", Colors::green},
        { "window.gray.lineedit.focused.fg", Colors::brightWhite},
        { "window.gray.lineedit.disabled.bg", Colors::darkGray},
        { "window.gray.lineedit.disabled.fg", Colors::lightGray},
        { "window.gray.textedit.bg", Colors::blue},
        { "window.gray.textedit.fg", Colors::brightWhite},
        { "window.gray.textedit.focused.bg", Colors::green},
        { "window.gray.textedit.focused.fg", Colors::brightWhite},
        { "window.gray.textedit.disabled.bg", Colors::darkGray},
        { "window.gray.textedit.disabled.fg", Colors::lightGray},
        { "window.gray.textedit.selected.bg", Colors::brightWhite},
        { "window.gray.textedit.selected.fg", Colors::darkGray},
        { "window.gray.textedit.linenumber.bg", { 0, 0, 0x80}},
        { "window.gray.textedit.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
        { "window.gray.textedit.focused.linenumber.bg", { 0, 0x80, 0}},
        { "window.gray.textedit.focused.linenumber.fg", { 0xdd, 0xdd, 0xdd}},

        { "window.cyan.bg", Colors::cyan},
        { "window.cyan.frame.focused.fg", Colors::brightWhite},
        { "window.cyan.frame.focused.control.fg", Colors::brightGreen},
        { "window.cyan.frame.unfocused.fg", Colors::lightGray},
        { "window.cyan.scrollbar.fg", Colors::cyan},
        { "window.cyan.scrollbar.bg", Colors::blue},
        { "window.cyan.scrollbar.control.fg", Colors::cyan},
        { "window.cyan.scrollbar.control.bg", Colors::blue},
        { "window.cyan.text.fg", Colors::black},
        { "window.cyan.text.bg", Colors::cyan},
        { "window.cyan.text.selected.fg", Colors::brightWhite},
        { "window.cyan.text.selected.bg", Colors::cyan},
        { "window.cyan.control.bg", Colors::cyan},
        { "window.cyan.control.fg", Colors::black},
        { "window.cyan.control.focused.bg", Colors::cyan},
        { "window.cyan.control.focused.fg", Colors::brightWhite},
        { "window.cyan.control.disabled.bg", Colors::cyan},
        { "window.cyan.control.disabled.fg", Colors::darkGray},
        { "window.cyan.control.shortcut.bg", Colors::cyan},
        { "window.cyan.control.shortcut.fg", Colors::brightYellow},
        { "window.cyan.dataview.bg", Colors::brightBlue},
        { "window.cyan.dataview.fg", Colors::black},
        { "window.cyan.dataview.selected.bg", Colors::brightBlue},
        { "window.cyan.dataview.selected.fg", Colors::brightYellow},
        { "window.cyan.dataview.selected.focused.bg", Colors::green},
        { "window.cyan.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.cyan.dataview.disabled.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.fg", Colors::black},
        { "window.cyan.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.cyan.button.bg", Colors::green},
        { "window.cyan.button.fg", Colors::black},
        { "window.cyan.button.default.bg", Colors::green},
        { "window.cyan.button.default.fg", Colors::brightCyan},
        { "window.cyan.button.focused.bg", Colors::green},
        { "window.cyan.button.focused.fg", Colors::brightWhite},
        { "window.cyan.button.disabled.bg", Colors::lightGray},
        { "window.cyan.button.disabled.fg", Colors::darkGray},
        { "window.cyan.button.shortcut.bg", Colors::green},
        { "window.cyan.button.shortcut.fg", Colors::brightYellow},
        { "window.cyan.lineedit.bg", Colors::blue},
        { "window.cyan.lineedit.fg", Colors::brightWhite},
        { "window.cyan.lineedit.focused.bg", Colors::green},
        { "window.cyan.lineedit.focused.fg", Colors::brightWhite},
        { "window.cyan.lineedit.disabled.bg", Colors::darkGray},
        { "window.cyan.lineedit.disabled.fg", Colors::lightGray},
        { "window.cyan.textedit.bg", Colors::blue},
        { "window.cyan.textedit.fg", Colors::brightWhite},
        { "window.cyan.textedit.focused.bg", Colors::green},
        { "window.cyan.textedit.focused.fg", Colors::brightWhite},
        { "window.cyan.textedit.disabled.bg", Colors::darkGray},
        { "window.cyan.textedit.disabled.fg", Colors::lightGray},
        { "window.cyan.textedit.selected.bg", Colors::brightWhite},
        { "window.cyan.textedit.selected.fg", Colors::darkGray},
        { "window.cyan.textedit.linenumber.bg", { 0, 0, 0x80}},
        { "window.cyan.textedit.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
        { "window.cyan.textedit.focused.linenumber.bg", { 0, 0x80, 0}},
        { "window.cyan.textedit.focused.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
    });

    setDefaultRules(p);
    return p;
}

ZPalette ZPalette::black() {
    ZPalette p;
    p.setColors({
        { "root.bg", Colors::black},
        { "root.fg", {0x80, 0x80, 0x80}},

        { "menu.bg", Colors::lightGray},
        { "menu.fg", Colors::black},
        { "menu.disabled.bg", Colors::lightGray},
        { "menu.disabled.fg", Colors::darkGray},
        { "menu.shortcut.bg", Colors::lightGray},
        { "menu.shortcut.fg", Colors::red},
        { "menu.selected.bg", Colors::green},
        { "menu.selected.fg", Colors::black},
        { "menu.selected.disabled.bg", Colors::green},
        { "menu.selected.disabled.fg", Colors::darkGray},
        { "menu.selected.shortcut.bg", Colors::green},
        { "menu.selected.shortcut.fg", Colors::red},

        { "window.default.bg", Colors::black},
        { "window.default.frame.focused.fg", Colors::brightWhite},
        { "window.default.frame.focused.control.fg", Colors::brightGreen},
        { "window.default.frame.unfocused.fg", Colors::lightGray},
        { "window.default.scrollbar.fg", Colors::black},
        { "window.default.scrollbar.bg", { 0, 0x55, 0xaa}},
        { "window.default.scrollbar.control.fg", Colors::brightWhite},
        { "window.default.scrollbar.control.bg", Colors::darkGray},
        { "window.default.text.fg", Colors::brightYellow},
        { "window.default.text.bg", Colors::black},
        { "window.default.text.selected.fg", Colors::black},
        { "window.default.text.selected.bg", Colors::lightGray},
        { "window.default.control.bg", Colors::black},
        { "window.default.control.fg", Colors::lightGray},
        { "window.default.control.focused.bg", Colors::black},
        { "window.default.control.focused.fg", Colors::brightWhite},
        { "window.default.control.disabled.bg", Colors::black},
        { "window.default.control.disabled.fg", Colors::darkGray},
        { "window.default.control.shortcut.bg", Colors::black},
        { "window.default.control.shortcut.fg", Colors::brightYellow},
        { "window.default.dataview.bg", Colors::brightWhite},
        { "window.default.dataview.fg", Colors::darkGray},
        { "window.default.dataview.selected.bg", Colors::brightWhite},
        { "window.default.dataview.selected.fg", Colors::black},
        { "window.default.dataview.selected.focused.bg", Colors::brightGreen},
        { "window.default.dataview.selected.focused.fg", Colors::black},
        { "window.default.dataview.disabled.bg", Colors::lightGray},
        { "window.default.dataview.disabled.fg", Colors::darkGray},
        { "window.default.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.default.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.default.button.bg", Colors::green},
        { "window.default.button.fg", Colors::black},
        { "window.default.button.default.bg", Colors::green},
        { "window.default.button.default.fg", Colors::brightCyan},
        { "window.default.button.focused.bg", Colors::green},
        { "window.default.button.focused.fg", Colors::brightWhite},
        { "window.default.button.disabled.bg", Colors::lightGray},
        { "window.default.button.disabled.fg", Colors::darkGray},
        { "window.default.button.shortcut.bg", Colors::green},
        { "window.default.button.shortcut.fg", Colors::brightYellow},
        { "window.default.lineedit.bg", Colors::lightGray},
        { "window.default.lineedit.fg", Colors::black},
        { "window.default.lineedit.focused.bg", Colors::lightGray},
        { "window.default.lineedit.focused.fg", Colors::brightWhite},
        { "window.default.lineedit.disabled.bg", Colors::darkGray},
        { "window.default.lineedit.disabled.fg", Colors::lightGray},
        { "window.default.textedit.bg", Colors::lightGray},
        { "window.default.textedit.fg", Colors::black},
        { "window.default.textedit.focused.bg", Colors::lightGray},
        { "window.default.textedit.focused.fg", Colors::brightWhite},
        { "window.default.textedit.disabled.bg", Colors::darkGray},
        { "window.default.textedit.disabled.fg", Colors::lightGray},
        { "window.default.textedit.selected.bg", Colors::brightWhite},
        { "window.default.textedit.selected.fg", Colors::darkGray},
        { "window.default.textedit.linenumber.bg", { 0x22, 0x22, 0x22}},
        { "window.default.textedit.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
        { "window.default.textedit.focused.linenumber.bg", { 0x22, 0x22, 0x22}},
        { "window.default.textedit.focused.linenumber.fg", { 0xdd, 0xdd, 0xdd}},

        { "window.gray.bg", Colors::lightGray},
        { "window.gray.frame.focused.fg", Colors::brightWhite},
        { "window.gray.frame.focused.control.fg", Colors::brightGreen},
        { "window.gray.frame.unfocused.fg", Colors::black},
        { "window.gray.scrollbar.fg", Colors::cyan},
        { "window.gray.scrollbar.bg", Colors::black},
        { "window.gray.scrollbar.control.fg", Colors::cyan},
        { "window.gray.scrollbar.control.bg", Colors::black},
        { "window.gray.text.fg", Colors::lightGray},
        { "window.gray.text.bg", Colors::black},
        { "window.gray.text.selected.fg", Colors::lightGray},
        { "window.gray.text.selected.bg", Colors::brightWhite},
        { "window.gray.control.bg", Colors::lightGray},
        { "window.gray.control.fg", Colors::black},
        { "window.gray.control.focused.bg", Colors::lightGray},
        { "window.gray.control.focused.fg", Colors::brightWhite},
        { "window.gray.control.disabled.bg", Colors::lightGray},
        { "window.gray.control.disabled.fg", Colors::darkGray},
        { "window.gray.control.shortcut.bg", Colors::lightGray},
        { "window.gray.control.shortcut.fg", Colors::brightYellow},
        { "window.gray.dataview.bg", Colors::brightWhite},
        { "window.gray.dataview.fg", Colors::darkGray},
        { "window.gray.dataview.selected.bg", Colors::brightWhite},
        { "window.gray.dataview.selected.fg", Colors::black},
        { "window.gray.dataview.selected.focused.bg", Colors::brightGreen},
        { "window.gray.dataview.selected.focused.fg", Colors::black},
        { "window.gray.dataview.disabled.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.fg", Colors::lightGray},
        { "window.gray.dataview.disabled.selected.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.gray.button.bg", Colors::green},
        { "window.gray.button.fg", Colors::black},
        { "window.gray.button.default.bg", Colors::green},
        { "window.gray.button.default.fg", Colors::brightCyan},
        { "window.gray.button.focused.bg", Colors::green},
        { "window.gray.button.focused.fg", Colors::brightWhite},
        { "window.gray.button.disabled.bg", Colors::lightGray},
        { "window.gray.button.disabled.fg", Colors::darkGray},
        { "window.gray.button.shortcut.bg", Colors::green},
        { "window.gray.button.shortcut.fg", Colors::brightYellow},
        { "window.gray.lineedit.bg", Colors::black},
        { "window.gray.lineedit.fg", Colors::brightWhite},
        { "window.gray.lineedit.focused.bg", Colors::green},
        { "window.gray.lineedit.focused.fg", Colors::brightWhite},
        { "window.gray.lineedit.disabled.bg", Colors::darkGray},
        { "window.gray.lineedit.disabled.fg", Colors::lightGray},
        { "window.gray.textedit.bg", Colors::black},
        { "window.gray.textedit.fg", Colors::brightWhite},
        { "window.gray.textedit.focused.bg", Colors::green},
        { "window.gray.textedit.focused.fg", Colors::brightWhite},
        { "window.gray.textedit.disabled.bg", Colors::darkGray},
        { "window.gray.textedit.disabled.fg", Colors::lightGray},
        { "window.gray.textedit.selected.bg", Colors::brightWhite},
        { "window.gray.textedit.selected.fg", Colors::darkGray},
        { "window.gray.textedit.linenumber.bg", { 0x22, 0x22, 0x22}},
        { "window.gray.textedit.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
        { "window.gray.textedit.focused.linenumber.bg", { 0, 0x80, 0}},
        { "window.gray.textedit.focused.linenumber.fg", { 0xdd, 0xdd, 0xdd}},

        { "window.cyan.bg", Colors::cyan},
        { "window.cyan.frame.focused.fg", Colors::brightWhite},
        { "window.cyan.frame.focused.control.fg", Colors::brightGreen},
        { "window.cyan.frame.unfocused.fg", Colors::lightGray},
        { "window.cyan.scrollbar.fg", Colors::cyan},
        { "window.cyan.scrollbar.bg", Colors::black},
        { "window.cyan.scrollbar.control.fg", Colors::cyan},
        { "window.cyan.scrollbar.control.bg", Colors::black},
        { "window.cyan.text.fg", Colors::black},
        { "window.cyan.text.bg", Colors::cyan},
        { "window.cyan.text.selected.fg", Colors::brightWhite},
        { "window.cyan.text.selected.bg", Colors::cyan},
        { "window.cyan.control.bg", Colors::cyan},
        { "window.cyan.control.fg", Colors::black},
        { "window.cyan.control.focused.bg", Colors::cyan},
        { "window.cyan.control.focused.fg", Colors::brightWhite},
        { "window.cyan.control.disabled.bg", Colors::cyan},
        { "window.cyan.control.disabled.fg", Colors::darkGray},
        { "window.cyan.control.shortcut.bg", Colors::cyan},
        { "window.cyan.control.shortcut.fg", Colors::brightYellow},
        { "window.cyan.dataview.bg", Colors::blue},
        { "window.cyan.dataview.fg", Colors::black},
        { "window.cyan.dataview.selected.bg", Colors::blue},
        { "window.cyan.dataview.selected.fg", Colors::brightYellow},
        { "window.cyan.dataview.selected.focused.bg", Colors::green},
        { "window.cyan.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.cyan.dataview.disabled.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.fg", Colors::black},
        { "window.cyan.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.cyan.button.bg", Colors::green},
        { "window.cyan.button.fg", Colors::black},
        { "window.cyan.button.default.bg", Colors::green},
        { "window.cyan.button.default.fg", Colors::brightCyan},
        { "window.cyan.button.focused.bg", Colors::green},
        { "window.cyan.button.focused.fg", Colors::brightWhite},
        { "window.cyan.button.disabled.bg", Colors::lightGray},
        { "window.cyan.button.disabled.fg", Colors::darkGray},
        { "window.cyan.button.shortcut.bg", Colors::green},
        { "window.cyan.button.shortcut.fg", Colors::brightYellow},
        { "window.cyan.lineedit.bg", Colors::black},
        { "window.cyan.lineedit.fg", Colors::brightWhite},
        { "window.cyan.lineedit.focused.bg", Colors::green},
        { "window.cyan.lineedit.focused.fg", Colors::brightWhite},
        { "window.cyan.lineedit.disabled.bg", Colors::darkGray},
        { "window.cyan.lineedit.disabled.fg", Colors::lightGray},
        { "window.cyan.textedit.bg", Colors::black},
        { "window.cyan.textedit.fg", Colors::brightWhite},
        { "window.cyan.textedit.focused.bg", Colors::green},
        { "window.cyan.textedit.focused.fg", Colors::brightWhite},
        { "window.cyan.textedit.disabled.bg", Colors::darkGray},
        { "window.cyan.textedit.disabled.fg", Colors::lightGray},
        { "window.cyan.textedit.selected.bg", Colors::brightWhite},
        { "window.cyan.textedit.selected.fg", Colors::darkGray},
        { "window.cyan.textedit.linenumber.bg", { 0x22, 0x22, 0x22}},
        { "window.cyan.textedit.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
        { "window.cyan.textedit.focused.linenumber.bg", { 0,    0x80,    0}},
        { "window.cyan.textedit.focused.linenumber.fg", { 0xdd, 0xdd, 0xdd}},
    });

    setDefaultRules(p);
    return p;
}

TUIWIDGETS_NS_END
