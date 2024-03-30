/*
 *  CoreCLR 
 *  Copyright (c) 2024 Bifrost Inc.
 *  Author: Nathan Martell
 *  Forked from: .NET Foundation
 *  MIT LICENSE
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef __CORECLR_DELEGATES_H__
#define __CORECLR_DELEGATES_H__

#include <stdint.h>

#if defined(_WIN32)
    #define CORECLR_DELEGATE_CALLTYPE __stdcall
    #ifdef _WCHAR_T_DEFINED
        typedef wchar_t char_t;
    #else
        typedef unsigned short char_t;
    #endif
#else
    #define CORECLR_DELEGATE_CALLTYPE
    typedef char char_t;
#endif

#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)

// Signature of delegate returned by coreclr_delegate_type::load_assembly_and_get_function_pointer
typedef int (CORECLR_DELEGATE_CALLTYPE *load_assembly_and_get_function_pointer_fn)(
    const char_t *assembly_path      /* Fully qualified path to assembly */,
    const char_t *type_name          /* Assembly qualified type name */,
    const char_t *method_name        /* Public static method name compatible with delegateType */,
    const char_t *delegate_type_name /* Assembly qualified delegate type name or null
                                        or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
                                        the UnmanagedCallersOnlyAttribute. */,
    void         *reserved           /* Extensibility parameter (currently unused and must be 0) */,
    /*out*/ void **delegate          /* Pointer where to store the function pointer result */);

// Signature of delegate returned by load_assembly_and_get_function_pointer_fn when delegate_type_name == null (default)
typedef int (CORECLR_DELEGATE_CALLTYPE *component_entry_point_fn)(void *arg, int32_t arg_size_in_bytes);

typedef int (CORECLR_DELEGATE_CALLTYPE *get_function_pointer_fn)(
    const char_t *type_name          /* Assembly qualified type name */,
    const char_t *method_name        /* Public static method name compatible with delegateType */,
    const char_t *delegate_type_name /* Assembly qualified delegate type name or null,
                                        or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
                                        the UnmanagedCallersOnlyAttribute. */,
    void         *load_context       /* Extensibility parameter (currently unused and must be 0) */,
    void         *reserved           /* Extensibility parameter (currently unused and must be 0) */,
    /*out*/ void **delegate          /* Pointer where to store the function pointer result */);

#endif // __CORECLR_DELEGATES_H__
