## -*- mode: m4 -*-

dnl Check if the type socklen_t is defined anywhere
AC_DEFUN(AC_C_SOCKLEN_T,
[AC_CACHE_CHECK(for socklen_t, ac_cv_c_socklen_t,
[
  AC_TRY_COMPILE([
    #include <sys/types.h>
    #include <sys/socket.h>
  ],[
    socklen_t foo;
  ],[
    ac_cv_c_socklen_t=yes
  ],[
    ac_cv_c_socklen_t=no
  ])
])

if test $ac_cv_c_socklen_t = no; then
  AC_DEFINE(socklen_t, int, [socklen_t type])
fi

])

AC_DEFUN(AC_CHECK_COMPILER_FLAG,
[
dnl AC_REQUIRE([AC_CHECK_COMPILERS]) <- breaks with autoconf 2.50
AC_MSG_CHECKING(whether $CXX supports -$1)
kde_cache=`echo $1 | sed 'y%.=/+-%___p_%'`
AC_CACHE_VAL(kde_cv_prog_cxx_$kde_cache,
[
echo 'int main() { return 0; }' >conftest.cc
eval "kde_cv_prog_cxx_$kde_cache=no"
if test -z "`$CXX -$1 -c conftest.cc 2>&1`"; then
  if test -z "`$CXX -$1 -o conftest conftest.o 2>&1`"; then
    eval "kde_cv_prog_cxx_$kde_cache=yes"
  fi
fi
rm -f conftest*
])
if eval "test \"`echo '$kde_cv_prog_cxx_'$kde_cache`\" = yes"; then
 CXXFLAGS="$CXXFLAGS -$1" 
 AC_MSG_RESULT(yes)
 :
 $2
else
 AC_MSG_RESULT(no)
 :
 $3
fi
])

AC_DEFUN(AC_CHECK_COMPILER,
[
AC_ARG_ENABLE(debug,
    [  --enable-debug          creates debugging code],
    [ use_debug="$enableval"], [use_debug="no"])
if test "$use_debug" = yes ; then
	AC_CHECK_COMPILER_FLAG(Wall)
	AC_CHECK_COMPILER_FLAG(g)
        AC_DEFINE(DEBUG,1,[Use debug code])
else
	AC_CHECK_COMPILER_FLAG(O2)
fi
AC_CHECK_COMPILER_FLAG(fno-exceptions)
AC_CHECK_COMPILER_FLAG(fno-check-new)
AC_CHECK_COMPILER_FLAG(fno-rtti)
])

