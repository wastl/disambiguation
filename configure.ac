AC_INIT(wsd-disambiguation,1.0,sschaffert@apache.org)
AM_INIT_AUTOMAKE

LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_CC_C99
AC_PROG_CXX


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


AC_CHECK_LIB(pthread,pthread_create, , [AC_CHECK_LIB(c_r,pthread_create, , AC_MSG_ERROR("threading not supported on platform"))])
AC_CHECK_HEADER(pthread.h, , AC_MSG_ERROR("required pthread headers are missing"))
AC_DEFINE(USE_THREADS, 1, [use threads])
CFLAGS="$CFLAGS -pthread"
CXXFLAGS="$CXXFLAGS -pthread"


AC_ARG_ENABLE(profiling,AS_HELP_STRING(--enable-profiling,enable GNU gprof profiling support),[
CFLAGS="$CFLAGS -g -pg"
CXXFLAGS="$CXXFLAGS -g -pg"
AC_DEFINE(PROFILING,1,[profiling enabled])
])

AC_ARG_ENABLE(profiling,AS_HELP_STRING(--enable-warnings,enable compile-time warnings),[
CFLAGS="$CFLAGS -Wall -Wextra"
CXXFLAGS="$CXXFLAGS -Wall -Wextra"
])


# for python client library; TODO: should be enabled/disabled
AM_PATH_PYTHON

# for Java client library; TODO: should be enabled/disabled
AC_PATH_PROG(MVN,mvn)
AC_SUBST(MVN)

AC_OUTPUT([Makefile relatedness/Makefile disambiguation/Makefile communication/Makefile graph/Makefile create/Makefile threading/Makefile python/Makefile java/Makefile src/Makefile])
