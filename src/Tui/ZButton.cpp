// SPDX-License-Identifier: BSL-1.0

#include "ZButton.h"
#include "ZButton_p.h"

#include <Tui/ZDefaultWidgetManager.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZButton::ZButton(ZWidget *parent) : ZWidget(parent, std::make_unique<ZButtonPrivate>(this)) {
    setFocusPolicy(StrongFocus);
    setSizePolicyV(SizePolicy::Fixed);
    setSizePolicyH(SizePolicy::Minimum);
}

ZButton::ZButton(const QString &text, ZWidget *parent) : ZButton(parent) {
    setText(text);
}

ZButton::ZButton(WithMarkupTag, const QString &markup, ZWidget *parent) : ZButton(parent) {
    setMarkup(markup);
}

ZButton::~ZButton() = default;

QString ZButton::text() const {
    auto *p = tuiwidgets_impl();
    return p->styledText.text();
}

void ZButton::setText(const QString &t) {
    auto *p = tuiwidgets_impl();
    p->styledText.setText(t);
    removeShortcut();
    update();
}

QString ZButton::markup() const {
    auto *p = tuiwidgets_impl();
    return p->styledText.markup();
}

void ZButton::setMarkup(const QString &m) {
    auto *p = tuiwidgets_impl();
    p->styledText.setMarkup(m);
    if (p->styledText.mnemonic().size()) {
        setShortcut(ZKeySequence::forMnemonic(p->styledText.mnemonic()));
    } else {
        removeShortcut();
    }
    update();
}

void ZButton::removeShortcut() {
    for(ZShortcut *s : findChildren<ZShortcut*>(QString(), Qt::FindDirectChildrenOnly)) {
        delete s;
    }
}

void ZButton::setShortcut(const ZKeySequence &key) {
    removeShortcut();
    ZShortcut *s = new ZShortcut(key, this);
    QObject::connect(s, &ZShortcut::activated, this, &ZButton::click);
}

void ZButton::setDefault(bool d) {
    ZDefaultWidgetManager *defaultManager = findFacet<ZDefaultWidgetManager>();
    if (!defaultManager) return;
    if (d) {
        defaultManager->setDefaultWidget(this);
    } else {
        if (isDefault()) {
            defaultManager->setDefaultWidget(nullptr);
        }
    }
}

bool ZButton::isDefault() const {
    ZDefaultWidgetManager *defaultManager = findFacet<ZDefaultWidgetManager>();
    if (defaultManager == nullptr) {
        return false;
    } else {
        return defaultManager->defaultWidget() == this;
    }
}

bool ZButton::event(QEvent *event) {
    if (event->type() == ZEventType::queryAcceptsEnter()) {
        if (isEnabled()) {
            event->accept();
        }
        return true;
    } else {
        return ZWidget::event(event);
    }
}

QSize ZButton::sizeHint() const {
    auto *p = tuiwidgets_impl();
    auto *term = terminal();
    if (!term) return {};
    auto cm = contentsMargins();
    QSize sh = { p->styledText.width(term->textMetrics()) + 6 + cm.left() + cm.right(),
             1 + cm.top() + cm.bottom() };
    return sh;
}

void ZButton::paintEvent(ZPaintEvent *event) {
    auto *p = tuiwidgets_impl();
    ZTextStyle baseStyle;
    ZTextStyle shortcut;
    ZTextStyle markerStyle = {getColor("control.fg"), getColor("control.bg"), getAttributes("control.attrs")};
    QRect r = contentsRect();
    ZPainter painter = event->painter()->translateAndClip(r.left(), r.top(), r.width(), r.height());

    if (!isEnabled()) {
        baseStyle = {getColor("button.disabled.fg"), getColor("button.disabled.bg"), getAttributes("control.disabled.attrs")};
        shortcut = baseStyle;
    } else {
        ZDefaultWidgetManager *defaultManager = findFacet<ZDefaultWidgetManager>();

        if (focus()) {
            baseStyle = {getColor("button.focused.fg"), getColor("button.focused.bg"), getAttributes("button.focused.attrs")};
            painter.writeWithAttributes(0, 0, QStringLiteral("»"), markerStyle.foregroundColor(), markerStyle.backgroundColor(), markerStyle.attributes());
            painter.writeWithAttributes(r.width() - 1, 0, QStringLiteral("«"), markerStyle.foregroundColor(), markerStyle.backgroundColor(), markerStyle.attributes());
        } else if (defaultManager && isDefault() && defaultManager->isDefaultWidgetActive()) {
            baseStyle = {getColor("button.default.fg"), getColor("button.default.bg"), getAttributes("button.default.attrs")};
            painter.writeWithAttributes(0, 0, QStringLiteral("→"), markerStyle.foregroundColor(), markerStyle.backgroundColor(), markerStyle.attributes());
            painter.writeWithAttributes(r.width() - 1, 0, QStringLiteral("←"), markerStyle.foregroundColor(), markerStyle.backgroundColor(), markerStyle.attributes());
        } else {
            baseStyle = {getColor("button.fg"), getColor("button.bg"), getAttributes("button.attrs")};
        }
        shortcut = {getColor("button.shortcut.fg"), getColor("button.shortcut.bg"), getAttributes("button.shortcut.attrs")};
    }

    if (r.width() > 4) {
        p->styledText.setMnemonicStyle(baseStyle, shortcut);
        painter.writeWithAttributes(1, 0, QStringLiteral("[ "), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
        painter.writeWithAttributes(r.width() - 3, 0, QStringLiteral(" ]"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
        if (p->styledText.width(painter.textMetrics()) > r.width() - 5) {
            p->styledText.write(&painter, 2, 0, r.width() - 4);
        } else {
            p->styledText.write(&painter, 3, 0, r.width() - 5);
        }
    } else {
        painter.writeWithAttributes(r.width() >= 3 ? 1 : 0, 0, QStringLiteral("[]"), baseStyle.foregroundColor(), baseStyle.backgroundColor(), baseStyle.attributes());
    }
}

void ZButton::click() {
    if (!isEnabled()) {
        return;
    }

    setFocus();
    clicked();
}

void ZButton::keyEvent(ZKeyEvent *event) {
    if (isEnabled() && (event->key() == Key_Space || event->key() == Key_Enter) && event->modifiers() == 0) {
        setFocus();
        event->accept();
        clicked();
    } else {
        ZWidget::keyEvent(event);
    }
}

bool ZButton::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZButton::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZButton::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZButton::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZButton::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZButton::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZButton::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZButton::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZButton::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZButton::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZButton::pasteEvent(ZPasteEvent *event) {
    return ZWidget::pasteEvent(event);
}

void ZButton::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZButton::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZButton::resizeEvent(ZResizeEvent *event) {
    return ZWidget::resizeEvent(event);
}

void ZButton::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}

ZButtonPrivate::ZButtonPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}

ZButtonPrivate::~ZButtonPrivate() {
}


TUIWIDGETS_NS_END
