#include "cache.h"
#include "preloadworker.h"


QCache<int, ImageData> cache;
QReadWriteLock cache_lock;
bool g_is_page_current_changed;
bool g_is_preload_run;
int currentPage;