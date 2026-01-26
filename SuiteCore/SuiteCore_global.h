#ifndef SUITECORE_GLOBAL_H
#define SUITECORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SUITECORE_LIBRARY)
#define SUITECORE_EXPORT Q_DECL_EXPORT
#else
#define SUITECORE_EXPORT Q_DECL_IMPORT
#endif

#endif // SUITECORE_GLOBAL_H
