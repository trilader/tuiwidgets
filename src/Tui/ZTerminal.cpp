#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QMetaMethod>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QVector>

#include <Tui/ZEvent.h>
#include <Tui/ZPainter_p.h>
#include <Tui/ZShortcutManager_p.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextMetrics_p.h>
#include <Tui/ZImage_p.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWidget_p.h>

TUIWIDGETS_NS_START

ZTerminalPrivate::ZTerminalPrivate(ZTerminal *pub, ZTerminal::Options options)
    : options(options)
{
    pub_ptr = pub;
    QObject::connect(QThread::currentThread()->eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock,
                     pub, &ZTerminal::dispatcherIsAboutToBlock);
}

ZTerminalPrivate::~ZTerminalPrivate() {
    deinitTerminal();
}

ZTerminalPrivate *ZTerminalPrivate::get(ZTerminal *terminal) {
    return terminal->tuiwidgets_impl();
}

const ZTerminalPrivate *ZTerminalPrivate::get(const ZTerminal *terminal) {
    return terminal->tuiwidgets_impl();
}

void ZTerminalPrivate::setFocus(ZWidget *w) {
    if (!w) {
        focusWidget = nullptr;
    } else {
        focusWidget = ZWidgetPrivate::get(w);
        focusHistory.appendOrMoveToLast(focusWidget);
    }
}

ZWidget *ZTerminalPrivate::focus() {
    return focusWidget ? focusWidget->pub() : nullptr;
}

void ZTerminalPrivate::setKeyboardGrab(ZWidget *w) {
    keyboardGrabWidget = w;
}

ZWidget *ZTerminalPrivate::keyboardGrab() {
    return keyboardGrabWidget;
}

ZShortcutManager *ZTerminalPrivate::ensureShortcutManager() {
    if (!shortcutManager) {
        shortcutManager = std::make_unique<ZShortcutManager>(pub());
    }
    return shortcutManager.get();
}

void ZTerminal::setCursorStyle(CursorStyle style) {
    auto *const p = tuiwidgets_impl();
    switch (style) {
        case CursorStyle::Unset:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_TERM_DEFAULT, true);
            break;
        case CursorStyle::Bar:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_BAR, true);
            break;
        case CursorStyle::Block:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_BLOCK, true);
            break;
        case CursorStyle::Underline:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_UNDERLINE, true);
            break;
    }
}

void ZTerminal::setCursorPosition(QPoint cursorPosition) {
    auto *const p = tuiwidgets_impl();
    termpaint_terminal_set_cursor_position(p->terminal, cursorPosition.x(), cursorPosition.y());
    const bool cursorVisible = cursorPosition != QPoint{-1, -1};
    termpaint_terminal_set_cursor_visible(p->terminal, cursorVisible);
}

void ZTerminal::setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB) {
    auto *const p = tuiwidgets_impl();
    if (cursorColorR != -1) {
        termpaint_terminal_set_color(p->terminal, TERMPAINT_COLOR_SLOT_CURSOR,
                                     cursorColorR,
                                     cursorColorG,
                                     cursorColorB);
    } else {
        termpaint_terminal_reset_color(p->terminal, TERMPAINT_COLOR_SLOT_CURSOR);
    }
}

void ZTerminalPrivate::processPaintingAndUpdateOutput(bool fullRepaint) {
    if (mainWidget.get()) {
        cursorPosition = QPoint{-1, -1};
        const QSize minSize = mainWidget->minimumSize();
        std::unique_ptr<ZPainter> paint;
        std::unique_ptr<ZImage> img;
        if (minSize.width() > termpaint_surface_width(surface) || minSize.height() > termpaint_surface_height(surface)) {
            viewportActive = true;
            viewportRange.setX(std::min(0, termpaint_surface_width(surface) - minSize.width()));
            viewportRange.setY(std::min(0, termpaint_surface_height(surface) - minSize.height() - 1));
            adjustViewportOffset();
            img = std::make_unique<ZImage>(pub(), std::max(minSize.width(), termpaint_surface_width(surface)),
                                           std::max(minSize.height(), termpaint_surface_height(surface)));
            paint = std::make_unique<ZPainter>(img->painter());
        } else {
            viewportActive = false;
            viewportUI = false;
            viewportRange.setX(0);
            viewportRange.setY(0);
            viewportOffset.setX(0);
            viewportOffset.setY(0);
            paint = std::make_unique<ZPainter>(pub()->painter());
        }
        ZPaintEvent event(ZPaintEvent::update, paint.get());
        QCoreApplication::sendEvent(mainWidget.get(), &event);
        if (initState == ZTerminalPrivate::InitState::Ready) {
            QPoint realCursorPosition = cursorPosition + viewportOffset;
            const bool cursorVisible = !(realCursorPosition.x() < 0
                                   || realCursorPosition.y() < 0
                                   || realCursorPosition.x() >= termpaint_surface_width(surface)
                                   || realCursorPosition.y() >= termpaint_surface_height(surface));
            if (cursorVisible) {
                pub()->setCursorPosition(realCursorPosition);
                CursorStyle style = CursorStyle::Unset;
                if (focusWidget) {
                    style = focusWidget->cursorStyle;
                    pub()->setCursorColor(focusWidget->cursorColorR,
                                          focusWidget->cursorColorG,
                                          focusWidget->cursorColorB);
                }
                pub()->setCursorStyle(style);
            } else {
                pub()->setCursorPosition({-1, -1});
            }
        }
        Q_EMIT pub()->afterRendering();
        if (viewportActive) {
            ZPainter terminalPainter = pub()->painter();
            terminalPainter.clear(ZColor::defaultColor(), ZColor::defaultColor());
            terminalPainter.drawImage(viewportOffset.x(), viewportOffset.y(), *img);
            if(viewportUI) {
                terminalPainter.writeWithColors(0, termpaint_surface_height(surface) - 1, QStringLiteral("←↑→↓ ESC"),
                                                ZColor::defaultColor(), ZColor::defaultColor());
            } else {
                terminalPainter.writeWithColors(0, termpaint_surface_height(surface) - 1, QStringLiteral("F6 Scroll"),
                                                ZColor::defaultColor(), ZColor::defaultColor());
            }
        }
        if (fullRepaint) {
            pub()->updateOutputForceFullRepaint();
        } else {
            pub()->updateOutput();
        }
    }
}

void ZTerminal::dispatcherIsAboutToBlock() {
    auto *const p = tuiwidgets_impl();
    if (!p->focusWidget || !p->focusWidget->enabled || !p->focusWidget->pub()->isVisibleTo(p->mainWidget.get())) {
        bool focusWasSet = false;
        ZWidgetPrivate *w = p->focusHistory.last;
        while (w) {
            if (w->enabled && w->pub()->isVisibleTo(p->mainWidget.get())) {
                w->pub()->setFocus();
                focusWasSet = true;
                break;
            }
            w = w->focusHistory.prev;
        }
        if (!focusWasSet) {
            p->setFocus(p->mainWidget.get());
        }
    }
}

ZTerminal::ZTerminal(QObject *parent)
    : ZTerminal(0, parent)
{
}

ZTerminal::ZTerminal(Options options, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this, options))
{
    tuiwidgets_impl()->initTerminal(options);
}

ZTerminal::ZTerminal(const ZTerminal::OffScreen &offscreen, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this, Options()))
{
    tuiwidgets_impl()->initOffscreen(offscreen);
}


bool ZTerminalPrivate::initTerminal(ZTerminal::Options options) {
    return setup(options);
}

void ZTerminalPrivate::initOffscreen(const ZTerminal::OffScreen &offscreen) {
    auto free = [] (termpaint_integration* ptr) {
        termpaint_integration_deinit(ptr);
    };
    auto write = [] (termpaint_integration* ptr, const char *data, int length) {
        Q_UNUSED(ptr);
        Q_UNUSED(data);
        Q_UNUSED(length);
    };
    auto flush = [] (termpaint_integration* ptr) {
        Q_UNUSED(ptr);
    };
    termpaint_integration_init(&integration, free, write, flush);

    callbackRequested = false;
    terminal = termpaint_terminal_new(&integration);
    surface = termpaint_terminal_get_surface(terminal);
    auto * const offscreenData = ZTerminal::OffScreenData::get(&offscreen);
    termpaint_surface_resize(surface, offscreenData->width, offscreenData->height);
    termpaint_terminal_set_event_cb(terminal, [](void *, termpaint_event *) {}, nullptr);
}

ZTerminal::~ZTerminal() {
    // widget destructors might depend on our services.
    // so ensure the widget hierarchie is torn down before the terminal.
    tuiwidgets_impl()->mainWidget.reset();
}

ZPainter ZTerminal::painter() {
    auto *surface = tuiwidgets_impl()->surface;
    return ZPainter(std::make_unique<ZPainterPrivate>(surface,
                                                      termpaint_surface_width(surface),
                                                      termpaint_surface_height(surface)));
}

ZTextMetrics ZTerminal::textMetrics() const {
    auto *surface = tuiwidgets_impl()->surface;
    return ZTextMetrics(std::make_shared<ZTextMetricsPrivate>(surface));
}

ZWidget *ZTerminal::mainWidget() {
    return tuiwidgets_impl()->mainWidget.get();
}

void ZTerminal::setMainWidget(ZWidget *w) {
    if (tuiwidgets_impl()->mainWidget) {
        ZWidgetPrivate::get(tuiwidgets_impl()->mainWidget.get())->unsetTerminal();
        tuiwidgets_impl()->mainWidget.release();
    }
    tuiwidgets_impl()->mainWidget.reset(w);
    ZWidgetPrivate::get(w)->setManagingTerminal(this);

    tuiwidgets_impl()->sendOtherChangeEvent(ZOtherChangeEvent::all().subtract({TUISYM_LITERAL("terminal")}));
    // TODO respect minimal widget size and add system managed scrolling if terminal is too small
    auto *surface = tuiwidgets_impl()->surface;
    w->setGeometry({0, 0, termpaint_surface_width(surface), termpaint_surface_height(surface)});

    update();
}

void ZTerminalPrivate::sendOtherChangeEvent(QSet<ZSymbol> unchanged) {
    ZOtherChangeEvent change(unchanged);
    QVector<QPointer<QObject>> todo;
    todo.append(mainWidget.get());
    while (todo.size()) {
        QObject *o = todo.takeLast();
        if (!o) continue;
        for (QObject* x : o->children()) todo.append(x);
        QCoreApplication::sendEvent(o, &change);
        change.setAccepted(true);
    }
}

ZWidget *ZTerminal::focusWidget() {
    if (tuiwidgets_impl()->focusWidget) {
        return tuiwidgets_impl()->focusWidget->pub();
    } else {
        return nullptr;
    }
}

void ZTerminal::update() {
    if (tuiwidgets_impl()->updateRequested) {
        return;
    }
    tuiwidgets_impl()->updateRequested = true;
    // XXX ZTerminal uses updateRequest with null painter internally
    QCoreApplication::postEvent(this, new ZPaintEvent(ZPaintEvent::update, nullptr), Qt::LowEventPriority);
}

void ZTerminal::forceRepaint() {
    auto *const p = tuiwidgets_impl();
    p->processPaintingAndUpdateOutput(true);
}

ZImage ZTerminal::grabCurrentImage() const {
    auto * const surface = tuiwidgets_impl()->surface;
    ZImage img = ZImage(this, width(), height());
    termpaint_surface_copy_rect(surface, 0, 0, width(), height(),
                                ZImageData::get(&img)->surface, 0, 0,
                                TERMPAINT_COPY_NO_TILE, TERMPAINT_COPY_NO_TILE);
    return img;
}

int ZTerminal::width() const {
    const auto * const surface = tuiwidgets_impl()->surface;
    return termpaint_surface_width(surface);

}

int ZTerminal::height() const {
    const auto * const surface = tuiwidgets_impl()->surface;
    return termpaint_surface_height(surface);
}

void ZTerminal::resize(int width, int height) {
    auto *const p = tuiwidgets_impl();
    termpaint_surface_resize(p->surface, width, height);
    if (p->mainWidget) {
        const QSize minSize = p->mainWidget->minimumSize();
        p->mainWidget->setGeometry({0, 0, std::max(minSize.width(), termpaint_surface_width(p->surface)),
                                    std::max(minSize.height(), termpaint_surface_height(p->surface))});
    }
    forceRepaint();
}

void ZTerminalPrivate::updateNativeTerminalState() {
    if (titleNeedsUpdate) {
        termpaint_terminal_set_title(terminal, title.toUtf8().data(), TERMPAINT_TITLE_MODE_ENSURE_RESTORE);
        titleNeedsUpdate = false;
    }
    if (iconTitleNeedsUpdate) {
        termpaint_terminal_set_icon_title(terminal, title.toUtf8().data(), TERMPAINT_TITLE_MODE_ENSURE_RESTORE);
        iconTitleNeedsUpdate = true;
    }
}

void ZTerminal::updateOutput() {
    auto *const p = tuiwidgets_impl();
    if (p->initState == ZTerminalPrivate::InitState::InInitWithoutPendingPaintRequest) {
        p->initState = ZTerminalPrivate::InitState::InInitWithPendingPaintRequest;
    } else if (p->initState == ZTerminalPrivate::InitState::InInitWithPendingPaintRequest) {
        // already requested
    } else if (p->initState == ZTerminalPrivate::InitState::Ready) {
        p->updateNativeTerminalState();
        termpaint_terminal_flush(p->terminal, false);
    }
}

void ZTerminal::updateOutputForceFullRepaint() {
    auto *const p = tuiwidgets_impl();
    if (p->initState == ZTerminalPrivate::InitState::InInitWithoutPendingPaintRequest) {
        p->initState = ZTerminalPrivate::InitState::InInitWithPendingPaintRequest;
    } else if (p->initState == ZTerminalPrivate::InitState::InInitWithPendingPaintRequest) {
        // already requested
    } else if (p->initState == ZTerminalPrivate::InitState::Ready) {
        p->updateNativeTerminalState();
        termpaint_terminal_flush(tuiwidgets_impl()->terminal, true);
    }
}

void ZTerminal::setTitle(QString title) {
    auto *const p = tuiwidgets_impl();
    p->title = title;
    p->titleNeedsUpdate = true;
}

void ZTerminal::setIconTitle(QString title) {
    auto *const p = tuiwidgets_impl();
    p->iconTitle = title;
    p->iconTitleNeedsUpdate = true;
}

void ZTerminal::setAutoDetectTimeoutMessage(const QString &message) {
    auto *const p = tuiwidgets_impl();
    p->autoDetectTimeoutMessage = message;
}

QString ZTerminal::autoDetectTimeoutMessage() const {
    auto *const p = tuiwidgets_impl();
    return p->autoDetectTimeoutMessage;
}

static ZSymbol extendedCharset = TUISYM_LITERAL("extendedCharset");
bool ZTerminal::hasCapability(ZSymbol cap) {
    auto *const p = tuiwidgets_impl();
    if (cap == extendedCharset) {
        return termpaint_terminal_capable(p->terminal, TERMPAINT_CAPABILITY_EXTENDED_CHARSET);
    }
    return false;
}

void ZTerminal::pauseOperation() {
    auto *const p = tuiwidgets_impl();
    if (p->initState != ZTerminalPrivate::InitState::Ready) return;
    p->pauseTerminal();
    p->initState = ZTerminalPrivate::InitState::Paused;
}

void ZTerminal::unpauseOperation() {
    auto *const p = tuiwidgets_impl();
    if (p->initState != ZTerminalPrivate::InitState::Paused) return;
    p->unpauseTerminal();
    p->initState = ZTerminalPrivate::InitState::Ready;
    updateOutputForceFullRepaint();
}

bool ZTerminal::isPaused() {
    auto *const p = tuiwidgets_impl();
    return p->initState == ZTerminalPrivate::InitState::Paused;
}

std::unique_ptr<ZKeyEvent> ZTerminal::translateKeyEvent(const ZTerminalNativeEvent &nativeEvent) {
    termpaint_event* native = static_cast<termpaint_event*>(nativeEvent.nativeEventPointer());

    Qt::KeyboardModifiers modifiers = 0;
    unsigned nativeModifier = 0;
    if (native->type == TERMPAINT_EV_KEY) {
        nativeModifier = native->key.modifier;
    } else if (native->type == TERMPAINT_EV_CHAR) {
        nativeModifier = native->c.modifier;
    }

    if (nativeModifier & TERMPAINT_MOD_SHIFT) {
        modifiers |= Qt::ShiftModifier;
    }
    if (nativeModifier & TERMPAINT_MOD_CTRL) {
        modifiers |= Qt::ControlModifier;
    }
    if (nativeModifier & TERMPAINT_MOD_ALT) {
        modifiers |= Qt::AltModifier;
    }

    if (native->type == TERMPAINT_EV_KEY) {
        int key = Qt::Key_unknown;
        if (native->key.atom == termpaint_input_page_up()) {
            key = Qt::Key_PageUp;
        } else if (native->key.atom == termpaint_input_page_down()) {
            key = Qt::Key_PageDown;
        } else if (native->key.atom == termpaint_input_arrow_right()) {
            key = Qt::Key_Right;
        } else if (native->key.atom == termpaint_input_arrow_left()) {
            key = Qt::Key_Left;
        } else if (native->key.atom == termpaint_input_arrow_down()) {
            key = Qt::Key_Down;
        } else if (native->key.atom == termpaint_input_arrow_up()) {
            key = Qt::Key_Up;
        } else if (native->key.atom == termpaint_input_tab()) {
            key = Qt::Key_Tab;
        } else if  (native->key.atom == termpaint_input_enter()) {
            key = Qt::Key_Enter;
        } else if  (native->key.atom == termpaint_input_backspace()) {
            key = Qt::Key_Backspace;
        } else if  (native->key.atom == termpaint_input_context_menu()) {
            key = Qt::Key_Menu;
        } else if  (native->key.atom == termpaint_input_delete()) {
            key = Qt::Key_Delete;
        } else if  (native->key.atom == termpaint_input_home()) {
            key = Qt::Key_Home;
        } else if  (native->key.atom == termpaint_input_insert()) {
            key = Qt::Key_Insert;
        } else if  (native->key.atom == termpaint_input_end()) {
            key = Qt::Key_End;
        } else if  (native->key.atom == termpaint_input_space()) {
            key = Qt::Key_Space;
        } else if  (native->key.atom == termpaint_input_escape()) {
            key = Qt::Key_Escape;
        } else if  (native->key.atom == termpaint_input_f1()) {
            key = Qt::Key_F1;
        } else if  (native->key.atom == termpaint_input_f2()) {
            key = Qt::Key_F2;
        } else if  (native->key.atom == termpaint_input_f3()) {
            key = Qt::Key_F3;
        } else if  (native->key.atom == termpaint_input_f4()) {
            key = Qt::Key_F4;
        } else if  (native->key.atom == termpaint_input_f5()) {
            key = Qt::Key_F5;
        } else if  (native->key.atom == termpaint_input_f6()) {
            key = Qt::Key_F6;
        } else if  (native->key.atom == termpaint_input_f7()) {
            key = Qt::Key_F7;
        } else if  (native->key.atom == termpaint_input_f8()) {
            key = Qt::Key_F8;
        } else if  (native->key.atom == termpaint_input_f9()) {
            key = Qt::Key_F9;
        } else if  (native->key.atom == termpaint_input_f10()) {
            key = Qt::Key_F10;
        } else if  (native->key.atom == termpaint_input_f11()) {
            key = Qt::Key_F11;
        } else if  (native->key.atom == termpaint_input_f12()) {
            key = Qt::Key_F12;
        } else if  (native->key.atom == termpaint_input_numpad_divide()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_division;
        } else if  (native->key.atom == termpaint_input_numpad_multiply()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_multiply;
        } else if  (native->key.atom == termpaint_input_numpad_subtract()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_Minus;
        } else if  (native->key.atom == termpaint_input_numpad_add()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_Plus;
        } else if  (native->key.atom == termpaint_input_numpad_enter()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_Enter;
        } else if  (native->key.atom == termpaint_input_numpad_decimal()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_Period;
        } else if  (native->key.atom == termpaint_input_numpad0()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_0;
        } else if  (native->key.atom == termpaint_input_numpad1()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_1;
        } else if  (native->key.atom == termpaint_input_numpad2()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_2;
        } else if  (native->key.atom == termpaint_input_numpad3()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_3;
        } else if  (native->key.atom == termpaint_input_numpad4()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_4;
        } else if  (native->key.atom == termpaint_input_numpad5()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_5;
        } else if  (native->key.atom == termpaint_input_numpad6()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_6;
        } else if  (native->key.atom == termpaint_input_numpad7()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_7;
        } else if  (native->key.atom == termpaint_input_numpad8()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_8;
        } else if  (native->key.atom == termpaint_input_numpad9()) {
            modifiers |= Qt::KeypadModifier;
            key = Qt::Key_9;
        }
        return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(key, modifiers, QString()) };
    } else if (native->type == TERMPAINT_EV_CHAR) {
        int key = Qt::Key_unknown;
        return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(key, modifiers, QString::fromUtf8(native->c.string, native->c.length)) };
    }

    return nullptr;
}

void ZTerminal::dispatchKeyboardEvent(ZKeyEvent &translated) {
    auto *const p = tuiwidgets_impl();
    if (p->keyboardGrabWidget) {
        QCoreApplication::sendEvent(p->keyboardGrabWidget, &translated);
    } else if (!p->shortcutManager || !p->shortcutManager->process(&translated)) {
        translated.accept();
        QPointer<ZWidget> w = tuiwidgets_impl()->focus();
        while (w) {
            bool processed = QCoreApplication::sendEvent(w, &translated);
            if ((processed && translated.isAccepted()) || !w) {
                break;
            }
            w = w->parentWidget();
        }
    }
}

void ZTerminal::dispatchPasteEvent(ZPasteEvent &translated) {
    auto *const p = tuiwidgets_impl();
    if (p->keyboardGrabWidget) {
        QCoreApplication::sendEvent(p->keyboardGrabWidget, &translated);
    } else {
        QPointer<ZWidget> w = tuiwidgets_impl()->focus();
        while (w) {
            bool processed = QCoreApplication::sendEvent(w, &translated);
            if ((processed && translated.isAccepted()) || !w) {
                break;
            }
            w = w->parentWidget();
        }
    }
}

void ZTerminalPrivate::adjustViewportOffset() {
    viewportOffset.setX(std::min(0, std::max(viewportRange.x(), viewportOffset.x())));
    viewportOffset.setY(std::min(0, std::max(viewportRange.y(), viewportOffset.y())));
}

bool ZTerminalPrivate::viewportKeyEvent(ZKeyEvent *translated) {
    if (viewportUI) {
        if (translated->key() == Qt::Key_F6 && translated->modifiers() == 0) {
            viewportUI = false;
            return false;
        } else if (translated->key() == Qt::Key_Escape) {
            viewportUI = false;
        } else if(translated->key() == Qt::Key_Left) {
            viewportOffset.setX(viewportOffset.x() + 1);
        } else if(translated->key() == Qt::Key_Right) {
            viewportOffset.setX(viewportOffset.x() - 1);
        } else if(translated->key() == Qt::Key_Down) {
            viewportOffset.setY(viewportOffset.y() - 1);
        } else if(translated->key() == Qt::Key_Up) {
            viewportOffset.setY(viewportOffset.y() + 1);
        }
        adjustViewportOffset();
        pub()->update();
    } else if (viewportActive && translated->key() == Qt::Key_F6 && translated->modifiers() == 0) {
        viewportUI = true;
        pub()->update();
    } else {
        return false;
    }
    return true;
}

bool ZTerminal::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->type() == ZEventType::rawSequence()) {
        return false;
    }
    if (event->type() == ZEventType::terminalNativeEvent()) {
        termpaint_event* native = static_cast<termpaint_event*>(static_cast<Tui::ZTerminalNativeEvent*>(event)->nativeEventPointer());
        if (native->type == TERMPAINT_EV_CHAR || native->type == TERMPAINT_EV_KEY) {
            std::unique_ptr<ZKeyEvent> translated = translateKeyEvent(*static_cast<Tui::ZTerminalNativeEvent*>(event));
            if (translated) {
                if (!p->viewportKeyEvent(translated.get())) {
                    dispatchKeyboardEvent(*translated);
                }
                if (!translated->isAccepted()) {
                    if (translated->modifiers() == Qt::ControlModifier
                        && translated->text() == QStringLiteral("l")) {
                        forceRepaint();
                    }
                }
            }
        } else if (native->type == TERMPAINT_EV_PASTE) {
            if (native->paste.initial) {
                p->pasteTemp = QString::fromUtf8(native->paste.string, native->paste.length);
            } else {
                p->pasteTemp += QString::fromUtf8(native->paste.string, native->paste.length);
            }
            if (native->paste.final) {
                std::unique_ptr<ZPasteEvent> translated = std::make_unique<ZPasteEvent>(p->pasteTemp);
                dispatchPasteEvent(*translated);
            }
        } else if (native->type == TERMPAINT_EV_REPAINT_REQUESTED) {
            update();
        } else if (native->type == TERMPAINT_EV_AUTO_DETECT_FINISHED) {
            termpaint_terminal_auto_detect_apply_input_quirks(p->terminal, p->backspaceIsX08);
            if (termpaint_terminal_might_be_supported(p->terminal)
                    || (p->options & ZTerminal::ForceIncompatibleTerminals)) {
                p->autoDetectTimeoutTimer = nullptr;
                QByteArray nativeOptions;
                if ((p->options & (ZTerminal::AllowInterrupt | ZTerminal::AllowQuit | ZTerminal::AllowSuspend)) == 0) {
                    nativeOptions.append(" +kbdsig ");
                }
                if (p->options & DisableAlternativeScreen) {
                    nativeOptions.append(" -altscreen ");
                }
                termpaint_terminal_setup_fullscreen(p->terminal,
                                                    termpaint_surface_width(p->surface),
                                                    termpaint_surface_height(p->surface),
                                                    nativeOptions.data());

                if (p->initState == ZTerminalPrivate::InitState::InInitWithPendingPaintRequest) {
                    update();
                }
                p->initState = ZTerminalPrivate::InitState::Ready;

                if (!termpaint_terminal_might_be_supported(p->terminal)) {
                    incompatibleTerminalDetected();
                }

                if (!(p->options & DisableTaggedPaste)) {
                    termpaint_terminal_request_tagged_paste(p->terminal, true);
                }
            } else {
                if (isSignalConnected(QMetaMethod::fromSignal(&ZTerminal::incompatibleTerminalDetected))) {
                    QPointer<ZTerminal> weak = this;
                    QTimer::singleShot(0, [weak] {
                        if (!weak.isNull()) {
                            weak->tuiwidgets_impl()->deinitTerminal();
                            weak->incompatibleTerminalDetected();
                        }
                    });
                } else {
                    QByteArray utf8 = QStringLiteral("Terminal auto detection failed. If this repeats the terminal might be incompatible.\r\n").toUtf8();
                    p->integration_write(utf8.data(), utf8.size());
                    p->integration_flush();
                    QPointer<ZTerminal> weak = this;
                    QTimer::singleShot(0, [weak] {
                        if (!weak.isNull()) {
                            weak->tuiwidgets_impl()->deinitTerminal();
                            QCoreApplication::quit();
                        }
                    });
                }
            }
        }

        return true; // ???
    }
    if (event->type() == ZEventType::updateRequest()) {
        // XXX ZTerminal uses updateRequest with null painter internally
        p->updateRequested = false;
        p->processPaintingAndUpdateOutput(false);
    }

    return QObject::event(event);
}

bool ZTerminal::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZTerminal::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZTerminal::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZTerminal::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZTerminal::connectNotify(const QMetaMethod &signal) {
    Q_UNUSED(signal);
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZTerminal::disconnectNotify(const QMetaMethod &signal) {
    Q_UNUSED(signal);
    // XXX needs to be thread-safe

}

ZTerminal::OffScreen::OffScreen(int width, int height) : tuiwidgets_pimpl_ptr(width, height) {
}

ZTerminal::OffScreen::~OffScreen() {
}

ZTerminal::OffScreenData::OffScreenData(int width, int height) : width(width), height(height) {
}


TUIWIDGETS_NS_END
