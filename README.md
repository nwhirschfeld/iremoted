Ruby Extention the Apple Infrared Remote - based on iremoted
=============================================================

![ARemote](https://raw.githubusercontent.com/nwhirschfeld/irrd/master/logo/remote.svg)

Getting started
---------------
Compile irrb like so:

    $ ruby extconf.rb
    $ make
    $ make install

Usage
-----

Sample Script:

    $ ruby testit.rb
    24
    25
    31
    32
    ...

In your Ruby-Project:

    require 'irrb'
    include Irrb # here starts a backgrounded thread, which stores the last recived value

    get_value # last stored value, if 0 it's empty
              # the last value is stored until someone calls 'get_value' or a new value is recived

For more info, see <http://www.osxbook.com/software/iremoted/>.

Credits
-------
This extention is just a wrapper for iremoted originally copied from <http://www.osxbook.com/software/iremoted/>.

Copyright
---------
Copyright (c) 2006-2008 Amit Singh. All Rights Reserved.

 - original iremoted.c, see https://github.com/swinton/iremoted
 - all functionality

Copyright (c) 2015 Niclas Hirschfeld.

 - only the ruby wrapper

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
