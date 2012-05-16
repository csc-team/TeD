#include <android/log.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "NativeExample native",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "NativeExample native",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "native code",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "NativeExample native",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "NativeExample native",__VA_ARGS__)
