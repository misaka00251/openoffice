#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=.

PRJNAME=epm
TARGET=epm

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(BUILD_EPM)" == "NO"

@all:
    @echo "epm disabled or system epm used ...."

.ELSE

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="UNX"

# Download from https://github.com/misaka00251/epm/archive/refs/tags/v5.0.0.1.tar.gz
TARFILE_NAME=v5.0.0.1
TARFILE_MD5=3dc5cd542a0f6aeffc3d604165839f65

CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--disable-fltk
.IF "$(OS)"=="MACOSX"
.IF "$(EXTRA_CFLAGS)"!=""
CONFIGURE_FLAGS+=CFLAGS="$(EXTRA_CFLAGS)" LDFLAGS="$(EXTRA_LINKFLAGS)" CPP="gcc -E $(EXTRA_CFLAGS)"
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF
BUILD_ACTION=make
OUT2BIN=epm epminstall mkepmlist

.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

