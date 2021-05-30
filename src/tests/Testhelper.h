#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <memory>

#include <QCoreApplication>

#include <Tui/ZColor.h>
#include <Tui/ZImage.h>
#include <Tui/ZLayout.h>
#include <Tui/ZPainter.h>
#include <Tui/ZRoot.h>

#include "../third-party/catch.hpp"


std::vector<std::string> getCurrentTestNames();


class Testhelper : public QObject {
public:
    enum Option {
        ReducedCharset = (1 << 0),
    };
    Q_DECLARE_FLAGS(Options, Option)

public:
    Testhelper(QString dir, QString namePrefix, int width, int height, Options options = {});

    void sendChar(QString ch, Qt::KeyboardModifiers modifiers = {});
    void sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers = {});
    void sendKeyToWidget(Tui::ZWidget *w, Qt::Key key, Qt::KeyboardModifiers modifiers = {});
    void sendKeyToZTerminal(QString key);

    void compare();
    void compare(QString name);
    void crossCheckWithMask(std::vector<std::string> overrideNames, std::vector<QPoint> ignore);
    void render();

    QString namePrefix;

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<Tui::ZTerminal> terminal;
    Tui::ZRoot *root;

    std::unique_ptr<const Tui::ZImage> lastCapture;
    QString basePath();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Testhelper::Options)


class StubLayout : public Tui::ZLayout {
public:
    void setGeometry(QRect) override {};
    QSize sizeHint() const override { return stubSizeHint; };
    Tui::SizePolicy sizePolicyH() const override { return stubSizePolicyH; };
    Tui::SizePolicy sizePolicyV() const override { return stubSizePolicyV; };
    bool isVisible() const override { return stubIsVisible; };
    bool isSpacer() const override { return stubIsSpacer; };

    QSize stubSizeHint;
    Tui::SizePolicy stubSizePolicyH = Tui::SizePolicy::Expanding;
    Tui::SizePolicy stubSizePolicyV = Tui::SizePolicy::Expanding;
    bool stubIsVisible = true;
    bool stubIsSpacer = false;
};


class StubWidget : public Tui::ZWidget {
public:
    explicit StubWidget(ZWidget *parent = 0) : Tui::ZWidget(parent) {};

public:
    QSize sizeHint() const override {
        return stubSizeHint;
    }

    void paintEvent(Tui::ZPaintEvent *event) override {
        auto painter = event->painter();
        painter->clear(fg, bg);
    }

    QSize stubSizeHint;
    Tui::ZColor fg;
    Tui::ZColor bg;
};


#endif // TESTHELPER_H