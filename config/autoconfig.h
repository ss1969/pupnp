/* whether the system defaults to 32bit off_t but can do 64bit when requested */
#define UPNP_LARGEFILE_SENSITIVE 1

/* see upnpconfig.h */
#define UPNP_VERSION_STRING "17.2.5"

/* see upnpconfig.h */
#define UPNP_VERSION_MAJOR 17

/* see upnpconfig.h */
#define UPNP_VERSION_MINOR 2

#ifndef UPNP_VERSION_MINOR
#       define UPNP_VERSION_MINOR 0
#endif

/* see upnpconfig.h */
#define UPNP_VERSION_PATCH 5

#ifndef UPNP_VERSION_PATCH
#       define UPNP_VERSION_PATCH 0
#endif

/* see upnpconfig.h */
/* #undef UPNP_HAVE_DEBUG */

/* see upnpconfig.h */
#define UPNP_HAVE_CLIENT 1

/* see upnpconfig.h */
#define UPNP_HAVE_DEVICE 1

/* see upnpconfig.h */
#define UPNP_HAVE_WEBSERVER 1

/* see upnpconfig.h */
#define UPNP_HAVE_SSDP 1

/* see upnpconfig.h */
#define UPNP_HAVE_OPTSSDP 1

/* see upnpconfig.h */
#define UPNP_HAVE_SOAP 1

/* see upnpconfig.h */
#define UPNP_HAVE_GENA 1

/* see upnpconfig.h */
#define UPNP_HAVE_TOOLS 1

/* see upnpconfig.h */
#define UPNP_ENABLE_IPV6 1

/* see upnpconfig.h */
/* #undef UPNP_ENABLE_UNSPECIFIED_SERVER */

/* see upnpconfig.h */
/* #undef UPNP_ENABLE_OPEN_SSL */

/* see upnpconfig.h */
/* #undef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS */

/* see upnpconfig.h */
#define IXML_HAVE_SCRIPTSUPPORT 1

/* see upnpconfig.h */
/* #undef UPNP_ENABLE_POST_WRITE */

/* see upnpconfig.h */
#define UPNP_MINISERVER_REUSEADDR 1

/* Type for storing the length of struct sockaddr */
/* #undef socklen_t */

/* Defines if strnlen is available on your system */
#define HAVE_STRNLEN 1

/* Defines if strndup is available on your system */
/* #undef HAVE_STRNDUP */

/* Use pthread_rwlock_t */
/* #undef UPNP_USE_RWLOCK */

