AC_INIT(wsd-disambiguation,1.0,sschaffert@apache.org)
AM_INIT_AUTOMAKE

LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_CC_C99
AC_PROG_CXX

AM_PATH_PYTHON

AC_CHECK_HEADERS([stdio.h stdarg.h time.h])

AC_CHECK_LIB(igraph, igraph_tree, , AC_MSG_ERROR("required igraph library is missing"))
AC_CHECK_HEADER(igraph/igraph.h, , AC_MSG_ERROR("required igraph headers are missing"))

AC_CHECK_LIB(raptor2, raptor_new_parser,rdflibs="-lraptor2", AC_MSG_ERROR("required raptor2 library is missing"))
AC_CHECK_HEADER(raptor2/raptor2.h, , AC_MSG_ERROR("required raptor2 headers are missing"))
AC_SUBST(rdflibs)

AC_CHECK_LIB(m,log, , AC_MSG_ERROR("required math library is missing"))
AC_CHECK_HEADER(math.h, , AC_MSG_ERROR("required math headers are missing"))

AC_CHECK_LIB(protobuf,_ZN6google8protobuf14MessageFactory17generated_factoryEv, protobuf_libs="-lprotobuf", AC_MSG_ERROR("protobuf library is missing"))
AC_SUBST(protobuf_libs)

AC_PATH_PROG(PROTOC,protoc)
AC_SUBST(PROTOC)

AC_ARG_ENABLE(profiling,AS_HELP_STRING(--enable-profiling,enable GNU gprof profiling support),[
CFLAGS="$CFLAGS -g -pg"
CXXFLAGS="$CXXFLAGS -g -pg"
AC_DEFINE(PROFILING,1,[profiling enabled])
])

AC_ARG_ENABLE(profiling,AS_HELP_STRING(--enable-warnings,enable compile-time warnings),[
CFLAGS="$CFLAGS -Wall -Wextra"
CXXFLAGS="$CXXFLAGS -Wall -Wextra"
])


threads=
AC_ARG_ENABLE(threads, AC_HELP_STRING([--enable-threads],
                      [enable multi-threaded computation (default is no)]),
                    ac_enable_threads=$enableval,
                    ac_enable_threads=no)
if test "$ac_enable_threads" = yes; then
  AC_DEFINE(USE_THREADS, 1, [use threads])
  CFLAGS="$CFLAGS -pthread"
  CXXFLAGS="$CXXFLAGS -pthread"
  AC_CHECK_LIB(pthread,pthread_create,threads=-lpthread, [AC_CHECK_LIB(c_r,pthread_create,threads=-lc_r)])
fi
AC_SUBST(threads)





AC_OUTPUT([Makefile relatedness/Makefile communication/Makefile graph/Makefile create/Makefile python/Makefile src/Makefile])
