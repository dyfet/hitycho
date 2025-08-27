# About HiTycho

HiTycho is my high performance / HPC template library for parallel computing
in C++ using HPX. It implies that Busudo is ``Low Tycho``. Basically, it is a
carefully curated re-thinking of Busudo and ModernCLI headers re-written
specifically for use with the HPX runtime for writing parallel computing and
distributed super-computing applications. HiTycho is purely header-only
specifically to avoid injecting any optional hpx libry components a given
header may use as a mandated link-time dependency.

While both ModernCLI and HiTycho are header-only C++ libraries, there are also
differences in how this package is maintained. For example, all headers use
hitycho/common.hpp and individual header files are not intended to be purely
stand-alone like they are in ModernCLI. This also conforms with Busudo. Some
stylistic aspects and practices are also meant to better fit with the HPX
runtime. C++17 Type traits and other compiler assisted code generation is used
by default and more commonly than were in the ModernCLI codebase.

HiTycho requires cmake and HPX to be installed. It should not itself depend on
Boost, and much of HPX, outside of asio, does not seem to anymore, either. It
is meant to build and use with C++17 or later C++ compiler, so the HPX package
could also be built for C++17. Like in ModernCLI and Busuto, cmake is used to
drive a unit test framework.

From a practical target perspective I assume clang or GCC are the only
compilers explicitly supported. It may be possible to build HPX on Microsoft
Windows, but it does not seem a platform anyone would likely use for practical
HPC work. Not supporting Microsoft Windows has made some HiTycho headers much
simpler than their ModernCLI counterparts. HiTycho will become possible to
usewwith Kakusu for cryptographic support. Kakusu offers Consistent hashing
rings which can be used to help build distributed computing services.

## Dependencies

HiTycho and applications require HPX. The current codebase was developed using
HPX 1.11.0. This gives us very recent HPX support, and that may become the
minimum HPX baseline in the future. I also could build a HPX package for
AlpineLinux.

## Distributions

Distributions of this package are provided as detached source tarballs made
from a tagged release from our public githubrepository or by building the dist
target. These stand-alone detached tarballs can be used to make packages for
many GNU/Linux systems, and for BSD ports. They may also be used to build and
install the software directly on a target platform.

The latest public release source tarball can be produced by an auto-generated
tarball from a tagged release in the projects public git repository at
https://github.com/dyfet/hitycho. HiTycho can also be easily vendored in other
software using git modules from this public repo. I may also package HiTycho
for Alpine Linux. There is no reason this cannot easily be packaged for use on
other distributions where HPX is supported.

## Participation

This project is offered as free (as in freedom) software for public use and has
a public project page at https://www.github.com/dyfet/hitycho which has an
issue tracker where you can submit public bug reports and a public git
repository. Patches and merge requests may be submitted in the issue tracker or
thru email. Other details about participation is in CONTRIBUTING.md.

## Testing

There is a testing program for each header. These run simple tests that will be
expanded to improve code coverage over time. The test programs are the only
build target making this library by itself, and the test programs in this
package work with the cmake ctest framework. They may also be used as simple
examples of how a given header works. There is also a **lint** target that can
be used to verify code changes.

