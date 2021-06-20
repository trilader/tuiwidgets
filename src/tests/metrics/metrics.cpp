#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextMetrics_p.h>

#include <../../third-party/catch.hpp>

#include <termpaint.h>

#include "../termpaint_helpers.h"

namespace {

enum Kind {
    KindQString, KindQChar, KindChar16, KindUtf, KindQStringView, KindU16StringView, KindStringView
};

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#define MAYBEKindQStringView , KindQStringView
#else
#define MAYBEKindQStringView
#endif

#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#define MAYBEKindStringView , KindU16StringView, KindStringView
#else
#define MAYBEKindStringView
#endif

#define ALLKINDS KindQString, KindQChar, KindChar16, KindUtf MAYBEKindQStringView MAYBEKindStringView

Tui::ZTextMetrics::ClusterSize nextClusterWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string) {
    switch (kind) {
        case KindQString:
            {
                QString padding = GENERATE(QString(""), QString("a"), QString("ab"),
                                           QString("はい"), QString("はい"), QString("😇"));
                UNSCOPED_INFO("padding: " << padding.toStdString());
                QString s = padding + string;
                return tm.nextCluster(s, padding.size());
            }
            break;
        case KindQChar:
            return tm.nextCluster(string.data(), string.size());
            break;
        case KindChar16:
            return tm.nextCluster(reinterpret_cast<const char16_t*>(string.data()), string.size());
            break;
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.nextCluster(utf8.data(), utf8.size());
            }
            break;
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.nextCluster(QStringView{string});
#endif
            break;
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.nextCluster(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()));
#endif
            break;
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.nextCluster(std::string_view(utf8.data(), utf8.size()));
            }
#endif
            break;
    }
    FAIL("Unknown kind");
    return {};
}

Tui::ZTextMetrics::ClusterSize splitByColumnsWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string, int maxWidth) {
    switch (kind) {
        case KindQString:
            return tm.splitByColumns(string, maxWidth);
            break;
        case KindQChar:
            return tm.splitByColumns(string.data(), string.size(), maxWidth);
            break;
        case KindChar16:
            return tm.splitByColumns(reinterpret_cast<const char16_t*>(string.data()), string.size(), maxWidth);
            break;
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.splitByColumns(utf8.data(), utf8.size(), maxWidth);
            }
            break;
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.splitByColumns(QStringView{string}, maxWidth);
#endif
            break;
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.splitByColumns(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()), maxWidth);
#endif
            break;
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.splitByColumns(std::string_view(utf8.data(), utf8.size()), maxWidth);
            }
#endif
            break;
    }
    FAIL("Unknown kind");
    return {};
}


int sizeInColumnsWrapper(Kind kind, Tui::ZTextMetrics &tm, const QString &string) {
    switch (kind) {
        case KindQString:
            return tm.sizeInColumns(string);
            break;
        case KindQChar:
            return tm.sizeInColumns(string.data(), string.size());
            break;
        case KindChar16:
            return tm.sizeInColumns(reinterpret_cast<const char16_t*>(string.data()), string.size());
            break;
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                return tm.sizeInColumns(utf8.data(), utf8.size());
            }
            break;
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.sizeInColumns(QStringView{string});
#endif
            break;
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            return tm.sizeInColumns(std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()));
#endif
            break;
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                return tm.sizeInColumns(std::string_view(utf8.data(), utf8.size()));
            }
#endif
            break;
    }
    FAIL("Unknown kind");
    return 0;
}

int nCodeUnits(Kind kind, QString s) {
    switch (kind) {
        case KindQString:
        case KindQChar:
        case KindChar16:
        case KindQStringView:
        case KindU16StringView:
            return s.size();
            break;
        case KindUtf:
        case KindStringView:
            return s.toUtf8().size();
            break;
    }
    FAIL("Unknown kind");
    return 0;
}

int nCodePoints(QString s) {
    return s.toStdU32String().size();
}

}

TEST_CASE("metrics - copy and assigment") {
    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);
    Tui::ZTextMetrics tm2 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);

    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm)->surface == f.surface);
    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm2)->surface == f2.surface);

    Tui::ZTextMetrics tm3{tm};
    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm3)->surface == f.surface);

    tm3 = tm2;
    REQUIRE(Tui::ZTextMetricsPrivate::get(&tm3)->surface == f2.surface);
}


TEST_CASE("metrics - nextCluster") {
    auto kind = GENERATE(ALLKINDS);

    struct TestCase { QString text; int columns; QString cluster; };
    const auto testCase = GENERATE(
                TestCase{ "test", 1, "t" },
                TestCase{ "はa", 2, "は" },
                TestCase{ "はい", 2, "は" },
                TestCase{ "😇bc", 2, "😇" },
                TestCase{"a\xcc\x88\xcc\xa4\x62\x63", 1, "a\xcc\x88\xcc\xa4"}
    );

    CAPTURE(kind);
    CAPTURE(testCase.text.toStdString());

    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);

    Tui::ZTextMetrics::ClusterSize result;
    result = nextClusterWrapper(kind, tm, testCase.text);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.cluster));
    CHECK(result.codePoints == nCodePoints(testCase.cluster));

    // check copy contruction
    Tui::ZTextMetrics tm2 = tm;

    result = nextClusterWrapper(kind, tm, "test");
    CHECK(result.columns == 1);
    CHECK(result.codeUnits == 1);
    CHECK(result.codePoints == 1);

    // check assignment does not break trivially
    Tui::ZTextMetrics tm3 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);
    tm2 = tm3;

    result = nextClusterWrapper(kind, tm, "test");
    CHECK(result.columns == 1);
    CHECK(result.codeUnits == 1);
    CHECK(result.codePoints == 1);
}


TEST_CASE("metrics - splitByColumns") {
    auto kind = GENERATE(ALLKINDS);

    struct TestCase { QString text; int splitAt; int columns; QString left; };
    const auto testCase = GENERATE(
                TestCase{ "test", 2, 2, "te" },
                TestCase{ "test", 1, 1, "t" },
                TestCase{ "test", 4, 4, "test" },
                TestCase{ "はい", 2, 2, "は" },
                TestCase{ "はい", 3, 2, "は" },
                TestCase{ "はい", 4, 4, "はい" },
                TestCase{ "😇bc", 2, 2, "😇" },
                TestCase{"a\xcc\x88\xcc\xa4\x62\x63", 1, 1, "a\xcc\x88\xcc\xa4"}
    );

    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);

    Tui::ZTextMetrics::ClusterSize result;
    result = splitByColumnsWrapper(kind, tm, testCase.text, testCase.splitAt);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.left));
    CHECK(result.codePoints == nCodePoints(testCase.left));

    // check copy contruction
    Tui::ZTextMetrics tm2 = tm;

    result = splitByColumnsWrapper(kind, tm, testCase.text, testCase.splitAt);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.left));
    CHECK(result.codePoints == nCodePoints(testCase.left));

    // check assignment does not break trivially
    Tui::ZTextMetrics tm3 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);
    tm2 = tm3;

    result = splitByColumnsWrapper(kind, tm, testCase.text, testCase.splitAt);
    CHECK(result.columns == testCase.columns);
    CHECK(result.codeUnits == nCodeUnits(kind, testCase.left));
    CHECK(result.codePoints == nCodePoints(testCase.left));
}

TEST_CASE("metrics - sizeInColumns") {
    auto kind = GENERATE(ALLKINDS);

    struct TestCase { QString text; int columns; };
    const auto testCase = GENERATE(
                TestCase{ "test", 4 },
                TestCase{ "はい", 4 },
                TestCase{ "😇bc", 4 },
                TestCase{"a\xcc\x88\xcc\xa4\x62\x63", 3}
    );

    TermpaintFixture f, f2;

    Tui::ZTextMetrics tm = Tui::ZTextMetricsPrivate::createForTesting(f.surface);

    CHECK(sizeInColumnsWrapper(kind, tm, testCase.text) == testCase.columns);

    // check copy contruction
    Tui::ZTextMetrics tm2 = tm;

    CHECK(sizeInColumnsWrapper(kind, tm2, testCase.text) == testCase.columns);

    // check assignment does not break trivially
    Tui::ZTextMetrics tm3 = Tui::ZTextMetricsPrivate::createForTesting(f2.surface);
    tm2 = tm3;

    CHECK(sizeInColumnsWrapper(kind, tm3, testCase.text) == testCase.columns);
}
