/*

    This file is a part of the JThread package, which contains some object-
    oriented thread wrappers for different thread implementations.

    Copyright (c) 2000-2011  Jori Liesenborgs (jori.liesenborgs@gmail.com)

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*/

#ifndef JTHREADCONFIG_H

#define JTHREADCONFIG_H

#ifdef WIN32
    #define JTHREAD_IMPORT __declspec(dllimport)
    #define JTHREAD_EXPORT __declspec(dllexport)
#else
    #define JTHREAD_IMPORT 
    #define JTHREAD_EXPORT 
#endif

#ifdef JTHREAD_COMPILING
	#define JTHREAD_IMPORTEXPORT JTHREAD_EXPORT
#else
	#define JTHREAD_IMPORTEXPORT JTHREAD_IMPORT
#endif // JTHREAD_COMPILING

#ifdef WIN32
    #define JTHREAD_CONFIG_WIN32THREADS 1
#endif

#ifdef WIN32
    #define JTHREAD_CONFIG_JMUTEXCRITICALSECTION 1
#endif

#endif // JTHREADCONFIG_H

