// SPDX-License-Identifier: BSL-1.0

#include "ZMenu.h"
#include "ZMenu_p.h"

#include <Tui/ZCommandManager.h>
#include <Tui/ZStyledTextLine.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START


ZMenu::ZMenu(ZWidget *parent) : ZWidget(parent, std::make_unique<ZMenuPrivate>(this)) {
    setStackingLayer(20000);
}

ZMenu::~ZMenu() = default;

void ZMenu::setItems(QVector<ZMenuItem> items) {
    auto *const p = tuiwidgets_impl();
    p->items = items;
    p->selected = 0;
    int width = 0;
    int shortcutWidth = 0;

    auto *term = terminal();

    for (int i = 0; i < p->items.size(); i++) {
        const auto& item = qAsConst(p->items)[i];
        if (item.markup().size()) {
            ZStyledTextLine st;
            st.setMarkup(QStringLiteral("<body> ") + item.markup() + QStringLiteral(" </body>"));
            width = std::max(width, term ? st.width(term->textMetrics()) : 0);
        }
        if (item.fakeShortcut().size()) {
            ZStyledTextLine st;
            st.setMarkup(QStringLiteral("<body>") + item.fakeShortcut() + QStringLiteral("</body>"));
            shortcutWidth = std::max(shortcutWidth, term ? st.width(term->textMetrics()) : 0);
        }
    }
    p->textWidth = width;
    p->shortcutWidth = shortcutWidth;
    update();
}

QVector<ZMenuItem> ZMenu::items() const {
    auto *const p = tuiwidgets_impl();
    return p->items;
}

void ZMenu::setParentMenu(ZMenubar *menu) {
    auto *const p = tuiwidgets_impl();
    p->parentMenu = menu;
}

void ZMenu::popup(const QPoint &p) {
    Q_EMIT aboutToShow();

    QSize size = sizeHint();
    QRect rect = {p.x(), p.y(), size.width(), size.height()};
    if (rect.right() >= parentWidget()->geometry().width()) {
        rect.moveRight(parentWidget()->geometry().width() - 1);
    }
    if (rect.left() < 0) {
        rect.moveLeft(0);
    }
    setGeometry(rect);
    grabKeyboard();
}

QSize ZMenu::sizeHint() const {
    return minimumSizeHint();
}

QSize ZMenu::minimumSizeHint() const {
    auto *const p = tuiwidgets_impl();
    int width = p->textWidth;
    if (p->shortcutWidth) {
        width += 1 + p->shortcutWidth;
    }
    width += 4;
    width = std::max(width, 20);
    return {width, 2 + p->items.size()};
}

QObject *ZMenu::facet(const QMetaObject &metaObject) const {
    auto *const p = tuiwidgets_impl();
    if (metaObject.className() == ZWindowFacet::staticMetaObject.className()) {
        return &p->windowFacet;
    } else {
        return ZWidget::facet(metaObject);
    }
}

void ZMenu::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    auto *painter = event->painter();
    ZTextStyle baseStyle = {getColor("menu.fg"), getColor("menu.bg"), getAttributes("menu.attrs")};
    ZTextStyle shortcut = {getColor("menu.shortcut.fg"), getColor("menu.shortcut.bg"), getAttributes("menu.shortcut.attrs") | ZTextAttribute::Underline};
    // For better support of terminals that don't do color or that don't understand the color format used, apply
    // inverse attribute but also swap background and foreground color. This ensures that these terminals show the
    // selected entry in inverse. By also swapping the colors terminals with full color support will still show the
    // intended colors.
    ZTextStyle selectedStyle = {getColor("menu.selected.bg"), getColor("menu.selected.fg"), getAttributes("menu.selected.attrs") | ZTextAttribute::Inverse};
    ZTextStyle selectedShortcut = {getColor("menu.selected.shortcut.bg"), getColor("menu.selected.shortcut.fg"),
                                  getAttributes("menu.selected.shortcut.attrs") | ZTextAttribute::Inverse | ZTextAttribute::Underline};
    ZTextStyle disabledStyle = {getColor("menu.disabled.fg"), getColor("menu.disabled.bg"), getAttributes("menu.disabled.attrs")};
    ZTextStyle selectedDisabledStyle = {getColor("menu.selected.disabled.fg"), getColor("menu.selected.disabled.bg"), getAttributes("menu.selected.disabled.attrs")};
    painter->clear(baseStyle.foregroundColor(), baseStyle.backgroundColor());
    ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();

    int w = geometry().width();
    int h = geometry().height();
    painter->writeWithAttributes(1, 0, QStringLiteral("┌"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
    painter->writeWithAttributes(w - 2, 0, QStringLiteral("┐"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
    painter->writeWithAttributes(w - 2, h - 1, QStringLiteral("┘"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
    painter->writeWithAttributes(1, h - 1, QStringLiteral("└"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());

    QString hline = QStringLiteral("─").repeated(w - 4);
    painter->writeWithAttributes(2, 0, hline, baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
    painter->writeWithAttributes(2, h - 1, hline, baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());

    for (int i = 1; i < h - 1; i++) {
        painter->writeWithAttributes(1, i, QStringLiteral("│"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
        painter->writeWithAttributes(w - 2, i, QStringLiteral("│"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
    }

    int y = 1;
    for (int i = 0; i < p->items.size(); i++) {
        const auto& item = qAsConst(p->items)[i];
        if (item.markup().size()) {
            ZStyledTextLine st;
            st.setMarkup(QStringLiteral("<body> ") + item.markup() + QStringLiteral(" </body>"));
            bool isEnabled = item.hasSubitems() || cmdMgr->isCommandEnabled(item.command());
            if (i == p->selected) {
                if (isEnabled) {
                    st.setMnemonicStyle(selectedStyle, selectedShortcut);
                } else {
                    st.setMnemonicStyle(selectedDisabledStyle, selectedDisabledStyle);
                }
            } else {
                if (isEnabled) {
                    st.setMnemonicStyle(baseStyle, shortcut);
                } else {
                    st.setMnemonicStyle(disabledStyle, disabledStyle);
                }
            }
            st.write(painter, 2, y, w - 4);
            st.setMarkup(QStringLiteral("<body>") + item.fakeShortcut() + QStringLiteral("</body>"));
            st.write(painter, w - 2 - p->shortcutWidth - 1, y, p->shortcutWidth);
        } else {
            painter->writeWithAttributes(1, y, QStringLiteral("├") + hline + QStringLiteral("┤"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
        }
        ++y;
    }
}

void ZMenu::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->key() == Key_Left && event->modifiers() == 0) {
        if (p->parentMenu) {
            p->parentMenu->left();
        }
    } else if (event->key() == Key_Right && event->modifiers() == 0) {
        if (p->parentMenu) {
            p->parentMenu->right();
        }
    } else if (event->key() == Key_Up && event->modifiers() == 0) {
        int newSelected = p->selected;
        for (int i = 0; i < p->items.size(); i++) { // loop at most once through all elements
            if (newSelected > 0) {
                --newSelected;
            } else {
                newSelected = p->items.size() - 1;
            }
            if (qAsConst(p->items)[newSelected].markup().size()) {
                p->selected = newSelected;
                break;
            }
        }
        update();
    } else if (event->key() == Key_Down && event->modifiers() == 0) {
        int newSelected = p->selected;
        for (int i = 0; i < p->items.size(); i++) { // loop at most once through all elements
            if (newSelected < p->items.size() - 1) {
                ++newSelected;
            } else {
                newSelected = 0;
            }
            if (qAsConst(p->items)[newSelected].markup().size()) {
                p->selected = newSelected;
                break;
            }
        }
        update();
    } else if (event->key() == Key_Enter && event->modifiers() == 0) {
        ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
        const auto& item = qAsConst(p->items)[p->selected];
        bool isEnabled = item.command() && cmdMgr->isCommandEnabled(item.command());
        if (isEnabled) {
            if (p->parentMenu) {
                p->parentMenu->close();
            } else {
                releaseKeyboard();
                setVisible(false);
                Q_EMIT aboutToHide();
            }
            cmdMgr->activateCommand(item.command());
        }
    } else if ((event->key() == Key_Escape || event->key() == Key_F10) && event->modifiers() == 0) {
        if (p->parentMenu) {
            p->parentMenu->close();
        } else {
            releaseKeyboard();
            setVisible(false);
            Q_EMIT aboutToHide();
        }
    } else if (event->modifiers() == AltModifier && event->text().size()) {
        // actualy this should be the toplevel menu bar
        if (p->parentMenu) {
            p->parentMenu->keyActivate(event);
        }
    } else if (event->modifiers() == 0 && event->text().size()) {
        ZCommandManager *const cmdMgr = parentWidget()->ensureCommandManager();
        for (int i = 0; i < p->items.size(); i++) {
            const auto& item = qAsConst(p->items)[i];
            ZStyledTextLine st;
            st.setMarkup(QStringLiteral("<body> ") + item.markup() + QStringLiteral(" </body>"));
            bool isEnabled = item.command() || cmdMgr->isCommandEnabled(item.command());
            if (isEnabled && st.mnemonic().toLower() == event->text().toLower()) {
                if (p->parentMenu) {
                    p->parentMenu->close();
                } else {
                    releaseKeyboard();
                    setVisible(false);
                    Q_EMIT aboutToHide();
                }
                cmdMgr->activateCommand(item.command());
                break;
            }
        }
    } else {
        ZWidget::keyEvent(event);
    }
}

bool ZMenu::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZMenu::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QRect ZMenu::layoutArea() const {
    return ZWidget::layoutArea();
}

ZWidget *ZMenu::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZMenu::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

void ZMenu::childEvent(QChildEvent *event) {
    ZWidget::childEvent(event);
}

void ZMenu::customEvent(QEvent *event) {
    ZWidget::customEvent(event);
}

void ZMenu::connectNotify(const QMetaMethod &signal) {
    ZWidget::connectNotify(signal);
}

void ZMenu::disconnectNotify(const QMetaMethod &signal) {
    ZWidget::disconnectNotify(signal);
}

void ZMenu::pasteEvent(ZPasteEvent *event) {
    ZWidget::pasteEvent(event);
}

void ZMenu::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZMenu::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZMenu::resizeEvent(ZResizeEvent *event) {
    ZWidget::resizeEvent(event);
}

void ZMenu::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

ZMenuPrivate::ZMenuPrivate(ZWidget *pub) : ZWidgetPrivate (pub) {
}

ZMenuPrivate::~ZMenuPrivate() {
}


TUIWIDGETS_NS_END
