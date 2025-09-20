# Features

HiTycho consists of a series of individual C++ 20 header files that typically
are installed under and included from /usr/include/hitycho for use in HPX
applications. Some of these are template headers, but all are meant to be used
directly inline. These currently include:

## atomics.hpp

Atomic types and lockfree data structures. This includes lockfree stack,
buffer, and unordered dictionary implimentations which are something like C#
ConcurrentStack, ConcurrentDictionary, and ConcurrentQueue. It also includes
an implimentation of atomic\_ref that should be similar to the C++20 one.

## binary.hpp

This is a generic portable convertable flexible binary data array object class
with supporting utility conversions for/to B64 and hex strings that I have
always wanted ever since using Qt QByteArray.

## biffer.hpp

Memory based stream buffering. This lets one parse memory buffers or address
spaces with stream operators. The format\_buffer provides a healpless and fast
alternative to std::strstream.

## common.hpp

Some very generic, universal, miscellaneous templates and functions. This also
is used to introduce new language-like "features", and is included in every
other header.

## expected.hpp

A C++17 simplified emulation of C++23 std::expected using std::variant.

## finalize.hpp

This offers the ability to finalize scopes thru a compiler optimized template
function. While scope\_defer is somewhat analogous to finalize in C# and golang
defer there is also a "scope\_detach" which then executes the finalize in a
separate HPX thread, thereby not blocking the function return itself. This
version of defer feels truer to the ``spirit'' of HPX though obviously you will
want to consider the scope and lifetimes of any captures or arguments used.

## fsys.hpp

Some useful and functional extensions to filesystem and file handling
operations.

## locking.hpp

This offers a small but interesting subset of ModernCLI classes that focus on
the idea that locking is access. This embodies combining a unique or shared
mutex with a private object that can only be accessed thru scope guards when a
lock is acquired. This offers consistent access behavior for lock protected
objects.

Note that for locking semantics to work with condition variables there is an
public accessible unlock method associated with exclusive_ptr and reader_ptr.
It's direct manual use probably would create race windows if the data ptr is
also accessed and relied on. In the future I may add additional owning check
along with existing ptr_ check for accessing data from these scoped pointers.

## networks.hpp

Animates the network interfaces list into a stl compatible container and
provides helper functions for finding what network interface an address
belongs to or to find interfaces for binding to subnets.

## pipeline.hpp

This is used to move objects between producer and consumer threads much like go
channels might work. Pipeline is optimized to use move semantics when possible
and to clear objects from the pipeline when they are removed. The pipeline is
constructed in a sized template so that it can be in static or stack space
without any heap allocations.

When pipelines are made for pointers, pipeline assumes the objects in the
pipeline are made from "new". If objects have to be dropped, they may be
deleted. The notify\_pipeline subclass behaves more like a true golang channel
in that you can have a poll or select wait on an event notification handle.

## print.hpp

Format and produce application output thru streams.

## resolver.hpp

A pure hpx native async resolver that does not rely on Boost, asio, or any
external packages. Mostly, like timers, it takes advantage of the fact that it
is very cheap to spin up lots of parallel threads in hpx if you need to.

## scan.hpp

Common functions to parse and extract fields like numbers and quoted strings
from a string view. As a scan function extracts, it also updates the view. The
low level scan functions will eventually be driven from a scan template class
that has a format string much like format. Other upper level utility functions
will also be provided.

## socket.hpp

Generic basic header to wrap access to address storage for low level BSD
sockets api. This makes it easier to manage, manipulate, and convert socket
addresses to strings.

## strings.hpp

Various string utilities. This includes a mix of the HPX string utils and some
advanced templated string utils from moderncli.

## sync.hpp

This introduces scoped guards for common C++17 HPX task synchronization classes
and adds some special wrapper versions for sempahores. The golang-like
ModernCLI wait group is also provided for HPX threads. The use cases for golang
waitgroups and the specific race conditions they help to resolve apply equally
well to detached HPX threads.

## system.hpp

Just some convenient C++ wrappers around system handles (file descriptors). It
may add some process level functionality eventually, too. It is also meant to
include the hpx init functions and be a basic application main include.

## threads.hpp

Common threading and simple support for parallel hpx function dispatch.

## timer.hpp

This is a timer system that takes advantage of the ability of HPX to spawn off
thousands of threads cheaply without needing thread pools or asio. Rather than
using an execute queue of lambdas on a single OS timer thread each timer has
it's own detached HPX thread instance for both one shot and periodic tasks.
These are executed as functional expressions thru templating so the compiler
can also optimize the call site.

## linting

Since this is common code extensive support exists for linting and static
analysis.
