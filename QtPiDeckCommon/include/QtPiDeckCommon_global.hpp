#ifndef QTPIDECKCOMMON_GLOBAL_H
#define QTPIDECKCOMMON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QTPIDECKCOMMON_LIBRARY)
#  define QTPIDECKCOMMON_EXPORT Q_DECL_EXPORT
#else
#  define QTPIDECKCOMMON_EXPORT Q_DECL_IMPORT
#endif

#endif // QTPIDECKCOMMON_GLOBAL_H
