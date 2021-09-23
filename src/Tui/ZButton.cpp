#include "ZButton.h"
#include "ZButton_p.h"

#include <Tui/ZDefaultWidgetManager.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZButton::ZButton(Tui::ZWidget *parent) : Tui::ZWidget(parent, std::make_unique<ZButtonPrivate>(this)) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicyV(Tui::SizePolicy::Fixed);
    setSizePolicyH(Tui::SizePolicy::Minimum);
}

Tui::ZButton::ZButton(const QString &text, Tui::ZWidget *parent) : ZButton(parent) {
    setText(text);
}

ZButton::ZButton(Tui::WithMarkupTag, const QString &markup, Tui::ZWidget *parent) : ZButton(parent) {
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

void ZButton::setMarkup(QString m) {
    auto *p = tuiwidgets_impl();
    p->styledText.setMarkup(m);
    if (p->styledText.mnemonic().size()) {
        setShortcut(Tui::ZKeySequence::forMnemonic(p->styledText.mnemonic()));
    } else {
        removeShortcut();
    }
    update();
}

void ZButton::removeShortcut() {
    for(Tui::ZShortcut *s : findChildren<Tui::ZShortcut*>(QString(), Qt::FindDirectChildrenOnly)) {
        delete s;
    }
}

void ZButton::setShortcut(const Tui::ZKeySequence &key) {
    removeShortcut();
    Tui::ZShortcut *s = new Tui::ZShortcut(key, this);
    QObject::connect(s, &Tui::ZShortcut::activated, this, &ZButton::click);
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

bool ZButton::isDefault() {
    ZDefaultWidgetManager *defaultManager = findFacet<ZDefaultWidgetManager>();
    if (defaultManager == nullptr) {
        return false;
    } else {
        return defaultManager->defaultWidget() == this;
    }
}

bool ZButton::event(QEvent *event) {
    if (event->type() == Tui::ZEventType::queryAcceptsEnter()) {
        if (isEnabled()) {
            event->accept();
        }
        return true;
    } else {
        return Tui::ZWidget::event(event);
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

void ZButton::paintEvent(Tui::ZPaintEvent *event) {
    auto *p = tuiwidgets_impl();
    Tui::ZTextStyle baseStyle;
    Tui::ZTextStyle shortcut;
    Tui::ZTextStyle markerStyle = {getColor("control.fg"), getColor("control.bg")};
    QRect r = contentsRect();
    Tui::ZPainter painter = event->painter()->translateAndClip(r.left(), r.top(), r.width(), r.height());

    if (!isEnabled()) {
        baseStyle = {getColor("button.disabled.fg"), getColor("button.disabled.bg")};
        shortcut = baseStyle;
    } else {
        ZDefaultWidgetManager *defaultManager = findFacet<ZDefaultWidgetManager>();

        if (focus()) {
            baseStyle = {getColor("button.focused.fg"), getColor("button.focused.bg")};
            painter.writeWithColors(0, 0, QStringLiteral("»"), markerStyle.foregroundColor(), markerStyle.backgroundColor());
            painter.writeWithColors(r.width() - 1, 0, QStringLiteral("«"), markerStyle.foregroundColor(), markerStyle.backgroundColor());
        } else if (defaultManager && isDefault() && defaultManager->isDefaultWidgetActive()) {
            baseStyle = {getColor("button.default.fg"), getColor("button.default.bg")};
            painter.writeWithColors(0, 0, QStringLiteral("→"), markerStyle.foregroundColor(), markerStyle.backgroundColor());
            painter.writeWithColors(r.width() - 1, 0, QStringLiteral("←"), markerStyle.foregroundColor(), markerStyle.backgroundColor());
        } else {
            baseStyle = {getColor("button.fg"), getColor("button.bg")};
        }
        shortcut = {getColor("button.shortcut.fg"), getColor("button.shortcut.bg")};
    }

    if (r.width() > 4) {
        p->styledText.setMnemonicStyle(baseStyle, shortcut);
        painter.writeWithColors(1, 0, QStringLiteral("[ "), baseStyle.foregroundColor(), baseStyle.backgroundColor());
        painter.writeWithColors(r.width() - 3, 0, QStringLiteral(" ]"), baseStyle.foregroundColor(), baseStyle.backgroundColor());
        if (p->styledText.width(painter.textMetrics()) > r.width()-5) {
            p->styledText.write(&painter, 2, 0, r.width()-4);
        } else {
            p->styledText.write(&painter, 3, 0, r.width()-5);
        }
    } else {
        painter.writeWithColors(r.width() >= 3 ? 1 : 0, 0, QStringLiteral("[]"), baseStyle.foregroundColor(), baseStyle.backgroundColor());
    }
}

void ZButton::click() {
    if (!isEnabled()) {
        return;
    }

    setFocus();
    clicked();
}

void ZButton::keyEvent(Tui::ZKeyEvent *event) {
    if(isEnabled() && (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) && event->modifiers() == 0) {
        setFocus();
        event->accept();
        clicked();
    } else {
        Tui::ZWidget::keyEvent(event);
    }
}

ZButtonPrivate::ZButtonPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}


TUIWIDGETS_NS_END