#pragma once

#include <QtCore/qglobal.h>

#ifndef QTPI4_VERSION
#define QTPI4_VERSION QT_VERSION_CHECK(5, 11, 3)
#endif
#ifndef QT_MIN_TESTED_VERSION
#define QT_MIN_TESTED_VERSION QT_VERSION_CHECK(5, 15, 1)
#endif

#if QT_VERSION > QTPI4_VERSION && QT_VERSION < QT_MIN_TESTED_VERSION
#error "New rasberrypi 4 qt version detected(?)"
#endif
