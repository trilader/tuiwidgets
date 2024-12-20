// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWIDGET_INCLUDED
#define TUIWIDGETS_ZWIDGET_INCLUDED

#include <memory>

#include <QObject>
#include <QMargins>

#include <Tui/ZCommon.h>
#include <Tui/ZEvent.h>
#include <Tui/ZMoFunc_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCommandManager;
class ZColor;
class ZImplicitSymbol;
class ZLayout;
class ZPalette;
class ZTerminal;

enum class FocusContainerMode : int {
    None,
    SubOrdering,
    Cycle
};

enum class SizePolicy : int {
    Fixed,
    Minimum,
    Maximum,
    Preferred,
    Expanding,
};

constexpr int tuiMaxSize = 0xffffff;

class ZWidgetPrivate;

class TUIWIDGETS_EXPORT ZWidget : public QObject {
    Q_OBJECT
public:
    explicit ZWidget(ZWidget *parent = nullptr);
    ~ZWidget() override;

protected:
    explicit ZWidget(ZWidget *parent, std::unique_ptr<ZWidgetPrivate> pimpl);

protected:
    std::unique_ptr<ZWidgetPrivate> tuiwidgets_pimpl_ptr;

public:
    TUIWIDGETS_NODISCARD_GETTER
    ZWidget *parentWidget() const { return static_cast<ZWidget*>(parent()); }
    void setParent(ZWidget *newParent);
    TUIWIDGETS_NODISCARD_GETTER
    QRect geometry() const;
    void setGeometry(const QRect &geometry);
    TUIWIDGETS_NODISCARD_GETTER
    QRect rect() const;
    TUIWIDGETS_NODISCARD_GETTER
    QRect contentsRect() const;
    TUIWIDGETS_NODISCARD_GETTER
    bool isEnabled() const; // includes enabled state of parents
    TUIWIDGETS_NODISCARD_GETTER
    bool isLocallyEnabled() const;
    void setEnabled(bool e);
    TUIWIDGETS_NODISCARD_GETTER
    bool isVisible() const; // includes visible state of parents
    TUIWIDGETS_NODISCARD_GETTER
    bool isLocallyVisible() const;
    void setVisible(bool v);
    void setStackingLayer(int layer);
    TUIWIDGETS_NODISCARD_GETTER
    int stackingLayer() const;
    void raise();
    void lower();
    void stackUnder(ZWidget *w);

    TUIWIDGETS_NODISCARD_GETTER
    QSize minimumSize() const;
    void setMinimumSize(QSize s);
    void setMinimumSize(int w, int h);
    TUIWIDGETS_NODISCARD_GETTER
    QSize maximumSize() const;
    void setMaximumSize(QSize s);
    void setMaximumSize(int w, int h);
    void setFixedSize(QSize s);
    void setFixedSize(int w, int h);
    TUIWIDGETS_NODISCARD_GETTER
    SizePolicy sizePolicyH() const;
    void setSizePolicyH(SizePolicy policy);
    TUIWIDGETS_NODISCARD_GETTER
    SizePolicy sizePolicyV() const;
    void setSizePolicyV(SizePolicy policy);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QSize effectiveSizeHint() const;
    QSize effectiveMinimumSize() const;
    virtual QRect layoutArea() const;
    TUIWIDGETS_NODISCARD_GETTER
    ZLayout *layout() const;
    void setLayout(ZLayout *l);

    void showCursor(QPoint position);
    ZTerminal *terminal() const;

    void update();
    void updateGeometry();

    void setFocusPolicy(FocusPolicy policy);
    TUIWIDGETS_NODISCARD_GETTER
    FocusPolicy focusPolicy() const;
    void setFocusMode(FocusContainerMode mode);
    TUIWIDGETS_NODISCARD_GETTER
    FocusContainerMode focusMode() const;
    void setFocusOrder(int order);
    TUIWIDGETS_NODISCARD_GETTER
    int focusOrder() const;

    TUIWIDGETS_NODISCARD_GETTER
    QMargins contentsMargins() const;
    void setContentsMargins(QMargins m);

    TUIWIDGETS_NODISCARD_GETTER
    const ZPalette &palette() const;
    void setPalette(const ZPalette &pal);
    TUIWIDGETS_NODISCARD_NOSIDEEFFECT
    ZColor getColor(const ZImplicitSymbol &x);
    TUIWIDGETS_NODISCARD_NOSIDEEFFECT
    ZTextAttributes getAttributes(const ZImplicitSymbol &x);

    TUIWIDGETS_NODISCARD_GETTER
    QStringList paletteClass() const;
    void setPaletteClass(QStringList classes);
    void addPaletteClass(const QString &clazz);
    void removePaletteClass(const QString &clazz);

    TUIWIDGETS_NODISCARD_GETTER
    CursorStyle cursorStyle() const;
    void setCursorStyle(CursorStyle style);
    void resetCursorColor();
    void setCursorColor(int r, int b, int g);

    void setFocus(FocusReason reason = OtherFocusReason);
    void grabKeyboard();
    void grabKeyboard(Private::ZMoFunc<void(QEvent*)> handler);
    void releaseKeyboard();

    bool isAncestorOf(const ZWidget *child) const;
    bool isEnabledTo(const ZWidget *ancestor) const;
    bool isVisibleTo(const ZWidget *ancestor) const;
    TUIWIDGETS_NODISCARD_GETTER
    bool focus() const;
    bool isInFocusPath() const;

    TUIWIDGETS_NODISCARD_NOSIDEEFFECT
    QPoint mapFromTerminal(const QPoint &pos);
    TUIWIDGETS_NODISCARD_NOSIDEEFFECT
    QPoint mapToTerminal(const QPoint &pos);

    ZWidget const *prevFocusable() const;
    ZWidget *prevFocusable();
    ZWidget const *nextFocusable() const;
    ZWidget *nextFocusable();
    const ZWidget *placeFocus(bool last = false) const;
    ZWidget *placeFocus(bool last = false);

    virtual ZWidget *resolveSizeHintChain();

    TUIWIDGETS_NODISCARD_GETTER
    ZCommandManager *commandManager() const;
    ZCommandManager *ensureCommandManager();
    void setCommandManager(ZCommandManager *cmd);

    template<typename T>
    T *findFacet() const {
        const ZWidget *w = this;
        while (w) {
            T *t = static_cast<T*>(w->facet(T::staticMetaObject));
            if (t) {
                return t;
            }
            w = w->parentWidget();
        }
        return nullptr;
    }

    virtual QObject *facet(const QMetaObject &metaObject) const;

    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual void paintEvent(ZPaintEvent *event);
    virtual void keyEvent(ZKeyEvent *event);
    virtual void pasteEvent(ZPasteEvent *event);
    virtual void focusInEvent(ZFocusEvent *event);
    virtual void focusOutEvent(ZFocusEvent *event);
    virtual void resizeEvent(ZResizeEvent *event);
    virtual void moveEvent(ZMoveEvent *event);

    void childEvent(QChildEvent *event) override;

    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    Q_DISABLE_COPY(ZWidget)
    TUIWIDGETS_DECLARE_PRIVATE(ZWidget)
};

TUIWIDGETS_NS_END
#endif // TUIWIDGETS_ZWIDGET_INCLUDED
