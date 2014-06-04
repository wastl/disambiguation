AC_INIT(wsd-disambiguation,1.0,sschaffert@apache.org)
AM_INIT_AUTOMAKE

LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_CC_C99

AC_CHECK_HEADERS([stdio.h stdarg.h time.h])


AC_CHECK_LIB(igraph, igraph_tree)
AC_CHECK_HEADER(igraph/igraph.h)

AC_CHECK_LIB(raptor2,raptor_new_parser)
AC_CHECK_HEADER(raptor2/raptor2.h)

AC_OUTPUT([Makefile src/Makefile])
