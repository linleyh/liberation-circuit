deps=$2.deps
deps_ne=$2.deps_ne
cflags="-O3 -fwrapv -MD -MF $deps"

if (command -v strace >/dev/null); then
 # Record non-existence header dependencies.
 # If headers GCC does not find are produced
 # in the future, the target is built again.
 strace -e stat,stat64,fstat,fstat64,lstat,lstat64 -o "$deps_ne.in" -f \
  gcc $cflags -o $3 -c ${1%.o}.c
 grep '1 ENOENT' <$deps_ne.in\
  |grep '\.h'\
  |cut -d'"' -f2\
  >$deps_ne
 rm -f "$deps_ne.in"

 while read -r DEP_NE; do
  redo-ifcreate ${DEP_NE}
 done <$deps_ne
else
 # Record non-existence strace dependency.
 # When strace is installed in the future,
 # the target is built again, with missing
 # headers recorded as non-existence deps.
 (
  IFS=:
  for folder in $PATH; do
   redo-ifcreate $folder/strace
  done
 )
 gcc $cflags -o $3 -c ${1%.o}.c
fi

read DEPS <$deps
redo-ifchange ${DEPS#*:}
