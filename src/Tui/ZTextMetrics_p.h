#ifndef TUIWIDGETS_ZTEXTMETRICS_P_INCLUDED
#define TUIWIDGETS_ZTEXTMETRICS_P_INCLUDED

#include <termpaint.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextMetricsPrivate {
public:
    ZTextMetricsPrivate(termpaint_surface *surface);

    termpaint_surface *surface;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTMETRICS_P_INCLUDED