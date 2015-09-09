#if defined (WIN32)
#if defined (_MSC_VER)
#pragma warning(disable: 4251)
#endif
   #if defined(tympan_lib_EXPORTS)
     #define TYMPAN_EXPORT __declspec(dllexport)
   #else
     #define TYMPAN_EXPORT __declspec(dllimport)
   #endif
#else
  #define TYMPAN_EXPORT
#endif
