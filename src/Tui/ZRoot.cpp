#include "ZRoot.h"
#include "ZRoot_p.h"

#include <QSize>

#include <Tui/ZPalette.h>
#include <Tui/ZPainter.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindowFacet.h>

TUIWIDGETS_NS_START

ZRoot::ZRoot() : ZWidget(nullptr, std::make_unique<ZRootPrivate>(this)) {
    setPalette(ZPalette::classic());
    setMinimumSize(40, 7);
}

void ZRoot::setFillChar(int fillChar) {
    auto *const p = tuiwidgets_impl();
    p->fillChar = fillChar;
}

int ZRoot::fillChar() {
    auto *const p = tuiwidgets_impl();
    return p->fillChar;
}

void ZRoot::paintEvent(ZPaintEvent *event) {
    auto *painter = event->painter();
    painter->clear(getColor("root.fg"), getColor("root.bg"), fillChar());
}

void ZRoot::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->key() == Qt::Key_F6 && (event->modifiers() == 0 || event->modifiers() == Qt::Modifier::SHIFT)) {
        ZWidget *first = nullptr;
        bool arm = false;
        bool found = false;

        QList<ZWidget*> childWindows;
        for (QObject* obj : p->windows) {
            auto childWidget = qobject_cast<ZWidget*>(obj);
            if (childWidget && childWidget->paletteClass().contains(QStringLiteral("window")) && childWidget->isVisible()) {
                childWindows.append(childWidget);
            }
        }

        if (event->modifiers() == Qt::Modifier::SHIFT) {
            std::reverse(childWindows.begin(), childWindows.end());
        }
        for(ZWidget *win : childWindows) {
            if (!first && win->placeFocus()) {
                first = win;
            }
            if (arm) {
                ZWidget *w = win->placeFocus();
                if (w) {
                    w->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
                    if(win->paletteClass().contains(QStringLiteral("dialog"))) {
                        win->raise();
                    }
                    found = true;
                    break;
                }
            }
            if (win->isInFocusPath()) {
                arm = true;
            }
        }
        if (!found && first) {
            first->placeFocus()->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
            if(first->paletteClass().contains(QStringLiteral("dialog"))) {
                first->raise();
            }
        }
    }
}

bool ZRoot::event(QEvent *event) {
    if (event->type() == Tui::ZEventType::otherChange()) {
        if (!static_cast<Tui::ZOtherChangeEvent*>(event)->unchanged().contains(TUISYM_LITERAL("terminal"))) {
            terminalChanged();
        }
    }
    return ZWidget::event(event);
}

void ZRoot::resizeEvent(ZResizeEvent *event) {
    const int height = geometry().height();
    const int width = geometry().width();
    for (QObject *o: children()) {
        auto childWidget = qobject_cast<ZWidget*>(o);
        if (childWidget) {
            ZWindowFacet *windowFacet = static_cast<ZWindowFacet*>(childWidget->facet(ZWindowFacet::staticMetaObject));
            if (windowFacet) {
                if (!windowFacet->isManuallyPlaced()) {
                    windowFacet->autoPlace({width, height}, childWidget);
                    continue;
                }
            }
        }
    }
    ZWidget::resizeEvent(event);
}

bool ZRoot::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

void ZRoot::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

QSize ZRoot::minimumSizeHint() const {
    QSize hint;
    for (QObject *o: children()) {
        auto childWidget = qobject_cast<ZWidget*>(o);
        if (childWidget) {
            ZWindowFacet *windowFacet = static_cast<ZWindowFacet*>(childWidget->facet(ZWindowFacet::staticMetaObject));
            if (windowFacet) {
                if (windowFacet->isExtendViewport()) {
                    hint = hint.expandedTo(QSize(childWidget->geometry().right(), childWidget->geometry().bottom() + 1));
                }
            }
        }
    }
    return hint;
}

QRect ZRoot::layoutArea() const {
    QRect tmp = geometry();
    if (terminal() && terminal()->mainWidget() == this) {
        // keep base layout inside the area we would get without children with extend viewport enabled.
        QSize terminalSize = { terminal()->width(), terminal()->height() };

        tmp = { {0, 0}, terminalSize.expandedTo(minimumSize()) };
    }
    tmp.moveTo(0, 0);
    return tmp.marginsRemoved(contentsMargins());
}

void ZRoot::childEvent(QChildEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->added()) {
        p->windows.prepend(event->child());
    }
    if (event->removed()) {
        p->windows.removeOne(event->child());
    }
    ZWidget::childEvent(event);
}

void ZRoot::terminalChanged() {
    // for derived classes to override
}

void ZRoot::customEvent(QEvent *event) {
    ZWidget::customEvent(event);
}

void ZRoot::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    ZWidget::connectNotify(signal);
}

void ZRoot::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    ZWidget::disconnectNotify(signal);
}

QObject *ZRoot::facet(const QMetaObject metaObject) {
    return ZWidget::facet(metaObject);
}

QSize ZRoot::sizeHint() const {
    return ZWidget::sizeHint();
}

void ZRoot::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZRoot::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZRoot::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

void ZRoot::pasteEvent(ZPasteEvent *event) {
    ZWidget::pasteEvent(event);
}

Tui::ZRootPrivate::ZRootPrivate(ZRoot *pub) : ZWidgetPrivate(pub) {

}

TUIWIDGETS_NS_END
