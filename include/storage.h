#pragma once

#include "constants.h"

#ifdef ENABLE_STORAGE
#include <Preferences.h>

extern Preferences storage;

// getString()/getBytes() log an error on missing keys
inline String getStoredString(Preferences &prefs, const char *key, const String &fallback = String())
{
  return prefs.isKey(key) ? prefs.getString(key, fallback) : fallback;
}

inline size_t getStoredBytes(Preferences &prefs, const char *key, void *buf, size_t len)
{
  return prefs.isKey(key) ? prefs.getBytes(key, buf, len) : 0;
}
#endif
