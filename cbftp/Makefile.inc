# uncomment and edit this line to link with a static openssl
#STATIC_SSL_PATH := /path/to/openssl

# relative path to the ui to use
UI_PATH := src/ui

UNAME_S := $(shell uname -s)
ifneq ($(STATIC_SSL_PATH),)
STATIC_SSL_INCLUDE := -I$(STATIC_SSL_PATH)/include
STATIC_SSL_LINK := $(STATIC_SSL_PATH)/libssl.a $(STATIC_SSL_PATH)/libcrypto.a
SSL_LINKFLAGS := $(STATIC_SSL_LINK) -ldl
else
SSL_LINKFLAGS := -lssl -lcrypto
endif

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

HAS_SVN := $(shell svn info $(ROOT_DIR) > /dev/null 2>&1; echo $$?)
REDIST_VERSION := $(shell cat $(ROOT_DIR)/.version 2>/dev/null)

ifeq ($(HAS_SVN), 0)
VERSION := "\"svn:r$(shell svn info $(ROOT_DIR)|grep Revision|awk '{ print $$2 }')\""
SVN_ROOT_DIR := $(shell svn info|grep 'Working Copy Root'|awk '{print $$5 }')
else ifneq ($(REDIST_VERSION),)
VERSION := "\"redist:r$(REDIST_VERSION)\""
else
VERSION := "\"unknown\""
endif

DATE := "\"$(shell date)\""

BOXTAG := "\"cbftp\""

CXX=g++
OPTFLAGS=-g -O0
CPPFLAGS=$(OPTFLAGS) -Wall -MMD -MP $(STATIC_SSL_INCLUDE)
LINKFLAGS=-lpthread $(SSL_LINKFLAGS)
ifeq ($(UNAME_S),Darwin)
LINKFLAGS+=-lncurses
else
LINKFLAGS+=-lncursesw
endif
