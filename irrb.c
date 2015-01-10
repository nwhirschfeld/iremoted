/*
 * irrb.c
 * Allows to read events from the Apple Infrared Remote.
 *
 * Build and Install
 *-----------------------------------------------------------------------------
 * $ ruby extconf.rb
 * $ make
 * $ make install
 * 
 *
 * License
 *-----------------------------------------------------------------------------
 * Copyright (c) 2006-2008 Amit Singh. All Rights Reserved.
 *      -> original iremoted.c, see https://github.com/swinton/iremoted
 *         all functionality
 * Copyright (c) 2015 Niclas Hirschfeld.
 *      -> only the wrapper 
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     
 *  THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

 // Include the Ruby headers and goodies
#include "ruby.h"

// Include iremoted headers
#define PROGNAME "iremoted"
#define PROGVERS "2.0"

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <pthread.h>

// Prototype for the initialization method - Ruby calls this, not you
void Init_irrb();

// Prototype for our method 'test1' - methods are prefixed by 'method_' here
VALUE method_test1(VALUE self);
VALUE get_value(VALUE self);
void method_test3(VALUE self);

VALUE Irrb = Qnil;


// iremoted source


IOHIDElementCookie buttonNextID = 0;
IOHIDElementCookie buttonPreviousID = 0;

typedef struct cookie_struct
{
    IOHIDElementCookie gButtonCookie_SystemAppMenu;
    IOHIDElementCookie gButtonCookie_SystemMenuSelect;
    IOHIDElementCookie gButtonCookie_SystemMenuRight;
    IOHIDElementCookie gButtonCookie_SystemMenuLeft;
    IOHIDElementCookie gButtonCookie_SystemMenuUp;
    IOHIDElementCookie gButtonCookie_SystemMenuDown;
} *cookie_struct_t;

static UInt32 return_val = 0;

void            usage(void);
void            QueueCallbackFunction(void *target, IOReturn result,
                                      void *refcon, void *sender);
bool            addQueueCallbacks(IOHIDQueueInterface **hqi);
void            processQueue(IOHIDDeviceInterface **hidDeviceInterface,
                             cookie_struct_t cookies);
void            doRun(IOHIDDeviceInterface **hidDeviceInterface,
                      cookie_struct_t cookies);
cookie_struct_t getHIDCookies(IOHIDDeviceInterface122 **handle);
void            createHIDDeviceInterface(io_object_t hidDevice,
                                         IOHIDDeviceInterface ***hdi);
void            setupAndRun(void);



void
QueueCallbackFunction(void *target, IOReturn result, void *refcon, void *sender)
{
    HRESULT               ret = 0;
    AbsoluteTime          zeroTime = {0,0};
    IOHIDQueueInterface **hqi;
    IOHIDEventStruct      event;

    while (!ret) {
        hqi = (IOHIDQueueInterface **)sender;
        ret = (*hqi)->getNextEvent(hqi, &event, zeroTime, 0);
        if (!ret) {
        	if ( event.value )
        	{
        		return_val = (UInt32)event.elementCookie;

        	}
            fflush(stdout);
        }
    }
}

bool
addQueueCallbacks(IOHIDQueueInterface **hqi)
{
    IOReturn               ret;
    CFRunLoopSourceRef     eventSource;
    IOHIDQueueInterface ***privateData;

    privateData = malloc(sizeof(*privateData));
    *privateData = hqi;

    ret = (*hqi)->createAsyncEventSource(hqi, &eventSource);
    if (ret != kIOReturnSuccess)
        return false;

    ret = (*hqi)->setEventCallout(hqi, QueueCallbackFunction,
                                  NULL, &privateData);
    if (ret != kIOReturnSuccess)
        return false;

    CFRunLoopAddSource(CFRunLoopGetCurrent(), eventSource,
                       kCFRunLoopDefaultMode);
    return true;
}

void
processQueue(IOHIDDeviceInterface **hidDeviceInterface, cookie_struct_t cookies)
{
    HRESULT               result;
    IOHIDQueueInterface **queue;

    queue = (*hidDeviceInterface)->allocQueue(hidDeviceInterface);
    if (!queue) {
        fprintf(stderr, "Failed to allocate event queue.\n");
        return;
    }

    (void)(*queue)->create(queue, 0, 8);

    (void)(*queue)->addElement(queue,
                               cookies->gButtonCookie_SystemAppMenu, 0);

    (void)(*queue)->addElement(queue,
                               cookies->gButtonCookie_SystemMenuSelect, 0);

    (void)(*queue)->addElement(queue,
                               cookies->gButtonCookie_SystemMenuRight, 0);

    (void)(*queue)->addElement(queue,
                               cookies->gButtonCookie_SystemMenuLeft, 0);

    (void)(*queue)->addElement(queue,
                               cookies->gButtonCookie_SystemMenuUp, 0);

    (void)(*queue)->addElement(queue,
                               cookies->gButtonCookie_SystemMenuDown, 0);

    addQueueCallbacks(queue);

    result = (*queue)->start(queue);
    
    CFRunLoopRun();

    result = (*queue)->stop(queue);

    result = (*queue)->dispose(queue);

    (*queue)->Release(queue);
}

void
doRun(IOHIDDeviceInterface **hidDeviceInterface, cookie_struct_t cookies)
{
    IOReturn ioReturnValue;

    ioReturnValue = (*hidDeviceInterface)->open(hidDeviceInterface, 0);

    processQueue(hidDeviceInterface, cookies);

    if (ioReturnValue == KERN_SUCCESS)
        ioReturnValue = (*hidDeviceInterface)->close(hidDeviceInterface);
    (*hidDeviceInterface)->Release(hidDeviceInterface);
}

cookie_struct_t
getHIDCookies(IOHIDDeviceInterface122 **handle)
{
    cookie_struct_t    cookies;
    IOHIDElementCookie cookie;
    CFTypeRef          object;
    long               number;
    long               usage;
    long               usagePage;
    CFArrayRef         elements;
    CFDictionaryRef    element;
    IOReturn           result;

    if ((cookies = (cookie_struct_t)malloc(sizeof(*cookies))) == NULL) {
        fprintf(stderr, "Failed to allocate cookie memory.\n");
        exit(1);
    }

    memset(cookies, 0, sizeof(*cookies));

    if (!handle || !(*handle))
        return cookies;

    result = (*handle)->copyMatchingElements(handle, NULL, &elements);

    if (result != kIOReturnSuccess) {
        fprintf(stderr, "Failed to copy cookies.\n");
        exit(1);
    }

    CFIndex i;
    for (i = 0; i < CFArrayGetCount(elements); i++) {
        element = CFArrayGetValueAtIndex(elements, i);
        object = (CFDictionaryGetValue(element, CFSTR(kIOHIDElementCookieKey)));
        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
            continue;
        if(!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
            continue;
        cookie = (IOHIDElementCookie)number;
        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsageKey));
        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
            continue;
        if (!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number))
            continue;
        usage = number;
        object = CFDictionaryGetValue(element,CFSTR(kIOHIDElementUsagePageKey));
        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
            continue;
        if (!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number))
            continue;
        usagePage = number;

        if (usagePage == kHIDPage_GenericDesktop) {
            switch (usage) {
            case kHIDUsage_GD_SystemAppMenu:
                cookies->gButtonCookie_SystemAppMenu = cookie;
                break;
            case kHIDUsage_GD_SystemMenu:
                cookies->gButtonCookie_SystemMenuSelect = cookie;
                break;
            case kHIDUsage_GD_SystemMenuRight:
                buttonNextID = cookie;
                cookies->gButtonCookie_SystemMenuRight = cookie;
                break;
            case kHIDUsage_GD_SystemMenuLeft:
                buttonPreviousID = cookie;
                cookies->gButtonCookie_SystemMenuLeft = cookie;
                break;
            case kHIDUsage_GD_SystemMenuUp:
                cookies->gButtonCookie_SystemMenuUp = cookie;
                break;
            case kHIDUsage_GD_SystemMenuDown:
                cookies->gButtonCookie_SystemMenuDown = cookie;
                break;
            }
        }
    }

    return cookies;
}

void
createHIDDeviceInterface(io_object_t hidDevice, IOHIDDeviceInterface ***hdi)
{
    io_name_t             className;
    IOCFPlugInInterface **plugInInterface = NULL;
    HRESULT               plugInResult = S_OK;
    SInt32                score = 0;
    IOReturn              ioReturnValue = kIOReturnSuccess;

    ioReturnValue = IOObjectGetClass(hidDevice, className);

    ioReturnValue = IOCreatePlugInInterfaceForService(
                        hidDevice,
                        kIOHIDDeviceUserClientTypeID,
                        kIOCFPlugInInterfaceID,
                        &plugInInterface,
                        &score);

    if (ioReturnValue != kIOReturnSuccess)
        return;

    plugInResult = (*plugInInterface)->QueryInterface(
                        plugInInterface,
                        CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
                        (LPVOID)hdi);

    (*plugInInterface)->Release(plugInInterface);
}

void
setupAndRun(void)
{
    CFMutableDictionaryRef hidMatchDictionary = NULL;
    io_service_t           hidService = (io_service_t)0;
    io_object_t            hidDevice = (io_object_t)0;
    IOHIDDeviceInterface **hidDeviceInterface = NULL;
    IOReturn               ioReturnValue = kIOReturnSuccess;
    cookie_struct_t        cookies;
    
    hidMatchDictionary = IOServiceNameMatching("AppleIRController");
    hidService = IOServiceGetMatchingService(kIOMasterPortDefault,
                                             hidMatchDictionary);



    if (!hidService) {
        fprintf(stderr, "Apple Infrared Remote not found.\n");
        exit(1);
    }

    hidDevice = (io_object_t)hidService;

    createHIDDeviceInterface(hidDevice, &hidDeviceInterface);
    cookies = getHIDCookies((IOHIDDeviceInterface122 **)hidDeviceInterface);
    ioReturnValue = IOObjectRelease(hidDevice);

    if (hidDeviceInterface == NULL) {
        fprintf(stderr, "No HID.\n");
        exit(1);
    }

    ioReturnValue = (*hidDeviceInterface)->open(hidDeviceInterface, 0);

    doRun(hidDeviceInterface, cookies);

    if (ioReturnValue == KERN_SUCCESS)
        ioReturnValue = (*hidDeviceInterface)->close(hidDeviceInterface);

    (*hidDeviceInterface)->Release(hidDeviceInterface);
}


// Get Method
VALUE get_value(VALUE self) {
	UInt32 x = return_val;
	return_val = 0;
	return INT2NUM(x);
}

// The initialization method for this module
void Init_irrb() {
	Irrb = rb_define_module("Irrb");
	rb_define_method(Irrb, "get_value", get_value, 0);
	pthread_t p1;
	pthread_create (&p1, NULL, (void *)&setupAndRun, NULL);
}
