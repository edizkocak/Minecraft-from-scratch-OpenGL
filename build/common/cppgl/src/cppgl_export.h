
#ifndef CPPGL_EXPORT_H
#define CPPGL_EXPORT_H

#ifdef CPPGL_STATIC_DEFINE
#  define CPPGL_EXPORT
#  define CPPGL_NO_EXPORT
#else
#  ifndef CPPGL_EXPORT
#    ifdef cppgl_EXPORTS
        /* We are building this library */
#      define CPPGL_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define CPPGL_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef CPPGL_NO_EXPORT
#    define CPPGL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef CPPGL_DEPRECATED
#  define CPPGL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef CPPGL_DEPRECATED_EXPORT
#  define CPPGL_DEPRECATED_EXPORT CPPGL_EXPORT CPPGL_DEPRECATED
#endif

#ifndef CPPGL_DEPRECATED_NO_EXPORT
#  define CPPGL_DEPRECATED_NO_EXPORT CPPGL_NO_EXPORT CPPGL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CPPGL_NO_DEPRECATED
#    define CPPGL_NO_DEPRECATED
#  endif
#endif

#endif /* CPPGL_EXPORT_H */
