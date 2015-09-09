#if defined (WIN32)
#if defined (_MSC_VER)
#pragma warning(disable: 4251)
#endif
   #if defined(tympan_common_EXPORTS)
     #define COMMON_EXPORT __declspec(dllexport)
   #else
     #define COMMON_EXPORT __declspec(dllimport)
   #endif
#else
  #define COMMON_EXPORT
#endif
