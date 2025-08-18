#ifndef UPNP_CONFIG_H
#define UPNP_CONFIG_H

/* Remove any existing definitions */
#ifdef UPNP_INLINE
#undef UPNP_INLINE
#endif

/* Define to 1 if the system supports 64-bit off_t */
#define UPNP_LARGEFILE_SENSITIVE 1

/* Version number of package */
#define UPNP_VERSION_STRING "1.14.25"
#define UPNP_VERSION_MAJOR 1
#define UPNP_VERSION_MINOR 14
#define UPNP_VERSION_PATCH 25

/* Debug Options */
#define UPNP_HAVE_DEBUG 1

/* Library Options */
#define UPNP_HAVE_CLIENT 1
#define UPNP_HAVE_DEVICE 1
#define UPNP_HAVE_WEBSERVER 1
#define UPNP_HAVE_SSDP 1
#define UPNP_HAVE_SOAP 1
#define UPNP_HAVE_GENA 1
#define UPNP_HAVE_TOOLS 1

/* Network specific defines */
#define HAVE_SOCKLEN_T 1
#define HAVE_IN6_ADDR 1
#define UPNP_ENABLE_IPV6 1
#define UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS 1

/* Other features */
#define UPNP_ENABLE_UNSPECIFIED_SERVER 0
#define UPNP_MINISERVER_REUSEADDR 1

/* Platform-specific headers */
#define UPNP_USE_MSVCPP 1


#endif /* UPNP_CONFIG_H */
