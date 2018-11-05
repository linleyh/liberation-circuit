deps=$2.deps
deps_ne=$2.deps_ne
cflags="-O3 -fwrapv -MD -MF $deps"

if [ "$1" != "precompile.o" ]; then
 redo-ifchange precompile.h.gch
 cflags="$cflags -include precompile.h"
fi

if [ -e "${1%.o}.c" ]; then
 src="${1%.o}.c"
elif [ -e "${1%.o}.h" ]; then
 src="${1%.o}.h"
else
 echo "$1: no source file found" >&2
 exit 99
fi

if command -v strace >/dev/null; then
 # Record non-existence header dependencies.
 # If headers GCC does not find are produced
 # in the future, the target is built again.
 strace -e stat,stat64,fstat,fstat64,lstat,lstat64 -o "$deps_ne.in" -f \
  gcc $cflags -o $3 -c $src
 grep '1 ENOENT' <$deps_ne.in\
  |grep '\.h'\
  |cut -d'"' -f2\
  >$deps_ne
 rm -f "$deps_ne.in"

 xargs redo-ifcreate <$deps_ne
else
 # Record non-existence strace dependency.
 # When strace is installed in the future,
 # the target is built again, with missing
 # headers recorded as non-existence deps.
 (
  IFS=:
  for folder in $PATH; do
   echo "$folder/strace"
  done | xargs redo-ifcreate
 )
 gcc $cflags -o $3 -c $src
fi

read DEPS <$deps
redo-ifchange ${DEPS#*:}
