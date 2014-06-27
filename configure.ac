AC_INIT(wsd-disambiguation,1.0,sschaffert@apache.org)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIRS([m4])

LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_CC_C99
AC_PROG_CXX


# C system headers and libraries
AC_CHECK_HEADERS([stdio.h stdarg.h time.h])

AC_CHECK_LIB(m,log, , AC_MSG_ERROR("required math library is missing"))
AC_CHECK_HEADER(math.h, , AC_MSG_ERROR("required math headers are missing"))


AC_CHECK_LIB(pthread,pthread_create, , [AC_CHECK_LIB(c_r,pthread_create, , AC_MSG_ERROR("threading not supported on platform"))])
AC_CHECK_HEADER(pthread.h, , AC_MSG_ERROR("required pthread headers are missing"))
AC_DEFINE(USE_THREADS, 1, [use threads])
CFLAGS="$CFLAGS -pthread"
CXXFLAGS="$CXXFLAGS -pthread"


# C user headers and libraries
AC_MSG_NOTICE([checking if the igraph graph algorithm is available])
AC_CHECK_LIB(igraph, igraph_tree, , AC_MSG_ERROR("required igraph library is missing"))
AC_CHECK_HEADER(igraph/igraph.h, , AC_MSG_ERROR("required igraph headers are missing"))

AC_MSG_NOTICE([checking if the Raptor RDF library is available])
AC_CHECK_LIB(raptor2, raptor_new_parser,rdflibs="-lraptor2", AC_MSG_ERROR("required raptor2 library is missing"))
AC_CHECK_HEADER(raptor2/raptor2.h, , AC_MSG_ERROR("required raptor2 headers are missing"))
AC_SUBST(rdflibs)


AC_MSG_NOTICE([checking if the METIS graph partitioning library is available])
AC_CHECK_LIB(metis, METIS_PartGraphRecursive, metislibs="-lmetis", AC_MSG_ERROR("required METIS partitioning library is missing"))
AC_CHECK_HEADER(metis.h, , AC_MSG_ERROR("required METIS partitioning library headers are missing"))
AC_SUBST(metislibs)

# c++ headers and libraries
AC_LANG_PUSH([C++])

# check for C++11
AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS="$CXXFLAGS -std=c++11"],
  AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"], AC_MSG_ERROR("C++11 not supported by compiler"))
)

#AC_CHECK_HEADER(boost/heap/fibonacci_heap.hpp, , AC_MSG_ERROR("required boost fibonacci heap missing (install boost library!)"))

#AC_CHECK_HEADER(boost/timer/timer.hpp, AC_DEFINE(TIMING, 1, [timing enabled]))

AC_MSG_NOTICE([checking if Google Protobuf is available])
AC_CHECK_HEADERS([google/protobuf/stubs/common.h google/protobuf/generated_message_util.h google/protobuf/message.h google/protobuf/repeated_field.h], protobuf_libs="-lprotobuf", AC_MSG_ERROR("protobuf library is missing"))
AC_SUBST(protobuf_libs)
AC_PATH_PROG(PROTOC,protoc)
AC_SUBST(PROTOC)

AC_LANG_POP([C++])





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
