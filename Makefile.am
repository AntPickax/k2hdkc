#
# K2HDKC
# 
# Copyright 2016 Yahoo Japan Corporation.
# 
# K2HDKC is k2hash based distributed KVS cluster.
# K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports
# distributed KVS cluster server program and client libraries.
# 
# For the full copyright and license information, please view
# the license file that was distributed with this source code.
#
# AUTHOR:   Takeshi Nakatani
# CREATE:   Thu Mar 31 2016
# REVISION:
#

SUBDIRS=docs lib src tests buildutils

EXTRA_DIST=RELEASE_VERSION @CONFIGURECUSTOM@

.PHONY: cppcheck shellcheck

#
# CppCheck
#
# [NOTE]
# Versions of cppcheck below 1.8 perform poorly and the test is
# skipped if those versions are installed.
#
CPPCHECK_NG_VER			= -e \^0\\. -e \^1\\.[0-7]
CPPCHECK_CMD			= cppcheck
CPPCHECK_TARGET			= $(SUBDIRS)
CPPCHECK_BUILD_DIR		= /tmp/cppcheck
CPPCHECK_BASE_OPT		= --quiet \
						  --error-exitcode=1 \
						  --inline-suppr \
						  -j 16 \
						  --std=c++03 \
						  --xml
CPPCHECK_ENABLE_OPT		= --enable=warning,style,information,missingInclude
CPPCHECK_IGNORE_OPT		= --suppress=unmatchedSuppression
CPPCHECK_INCDIR_OPT		= -I . -I lib
CPPCHECK_DEFINE_OPT		=

# [NOTE] Switch options
# The options available vary depending on the version.
# There are cases where you use options that do not exist in older
# versions.
#
CPPCHECK_GT207_VER		= 2007
CPPCHECK_GT207_ADD_OPT	= --suppress=ctuOneDefinitionRuleViolation
CPPCHECK_GE211_VER		= 2011
CPPCHECK_GE211_ADD_OPT	= --check-level=exhaustive \
						  --suppress=missingIncludeSystem

cppcheck:
	@set -e; \
	if command -v $(CPPCHECK_CMD) >/dev/null 2>&1; then \
		if ($(CPPCHECK_CMD) --version | sed -e 's|Cppcheck[[:space:]]*||gi' | grep -q $(CPPCHECK_NG_VER)); then \
			echo "*** [INFO] cppcheck version below 1.8, so skip to run cppcheck."; \
		else \
			echo "*** Check all files with CppCheck"; \
			if [ -d $(CPPCHECK_BUILD_DIR) ]; then \
				rm -rf $(CPPCHECK_BUILD_DIR); \
			fi; \
			mkdir -p $(CPPCHECK_BUILD_DIR); \
			if test `$(CPPCHECK_CMD) --version | sed -e 's/\./ /g' | awk '{print ($$2 * 1000 + $$3)}'` -le $(CPPCHECK_GT207_VER); then \
				$(CPPCHECK_CMD) $(CPPCHECK_BASE_OPT) $(CPPCHECK_DEFINE_OPT) $(CPPCHECK_INCDIR_OPT) $(CPPCHECK_ENABLE_OPT) $(CPPCHECK_IGNORE_OPT) --cppcheck-build-dir=$(CPPCHECK_BUILD_DIR) $(CPPCHECK_TARGET); \
			elif test `$(CPPCHECK_CMD) --version | sed -e 's/\./ /g' | awk '{print ($$2 * 1000 + $$3)}'` -lt $(CPPCHECK_GE211_VER); then \
				$(CPPCHECK_CMD) $(CPPCHECK_BASE_OPT) $(CPPCHECK_DEFINE_OPT) $(CPPCHECK_INCDIR_OPT) $(CPPCHECK_ENABLE_OPT) $(CPPCHECK_IGNORE_OPT) $(CPPCHECK_GT207_ADD_OPT) --cppcheck-build-dir=$(CPPCHECK_BUILD_DIR) $(CPPCHECK_TARGET); \
			else \
				$(CPPCHECK_CMD) $(CPPCHECK_BASE_OPT) $(CPPCHECK_DEFINE_OPT) $(CPPCHECK_INCDIR_OPT) $(CPPCHECK_ENABLE_OPT) $(CPPCHECK_IGNORE_OPT) $(CPPCHECK_GT207_ADD_OPT) $(CPPCHECK_GE211_ADD_OPT) --cppcheck-build-dir=$(CPPCHECK_BUILD_DIR) $(CPPCHECK_TARGET); \
			fi; \
			rm -rf $(CPPCHECK_BUILD_DIR); \
		fi; \
	else \
		echo "*** [WARNING] cppcheck is not installed, so skip to run cppcheck."; \
	fi

#
# ShellCheck
#
# This make target executes shellchek on text files with the
# extension '*.sh' under the current directory(other than the
# '.git' directory) and executable files(text files other than
# the extension '*.sh').
#
SHELLCHECK_CMD				= shellcheck
SHELLCHECK_BASE_OPT			= --shell=sh
SHELLCHECK_IGN_OPT			= --exclude=SC1117,SC1090,SC1091
SHELLCHECK_INCLUDE_IGN_OPT	= --exclude=SC1117,SC1090,SC1091,SC2034,SC2148
SHELLCHECK_FILES_NO_SH		= `grep -ril '^\#!/bin/sh' . | grep -v '\.sh' | grep -v '\.log' | grep -v '/\.git/' | grep -v '/rpmbuild/' | grep -v '/debian_build/' | grep -v '/autom4te.cache/' | grep -v '/m4/' | grep -v '/install-sh' | grep -v '/ltmain.sh' | tr '\n' ' '`
SHELLCHECK_FILES_SH			= `grep -ril '^\#!/bin/sh' . | grep '\.sh'    | grep -v '\.log' | grep -v '/\.git/' | grep -v '/rpmbuild/' | grep -v '/debian_build/' | grep -v '/autom4te.cache/' | grep -v '/m4/' | grep -v '/install-sh' | grep -v '/ltmain.sh' | tr '\n' ' '`
SHELLCHECK_FILES_INCLUDE_SH	= `grep -Lir '^\#!/bin/sh' . | grep '\.sh'    | grep -v '\.log' | grep -v '/\.git/' | grep -v '/rpmbuild/' | grep -v '/debian_build/' | grep -v '/autom4te.cache/' | grep -v '/m4/' | grep -v '/install-sh' | grep -v '/ltmain.sh' | tr '\n' ' '`

shellcheck:
	@set -e; \
	if type shellcheck > /dev/null 2>&1; then \
		echo "*** Check all files with ShellCheck"; \
		if [ -n "$(SHELLCHECK_FILES_NO_SH)" ]; then \
			LC_ALL=C.UTF-8 $(SHELLCHECK_CMD) $(SHELLCHECK_BASE_OPT) $(SHELLCHECK_IGN_OPT) $(SHELLCHECK_FILES_NO_SH); \
		fi; \
		if [ -n "$(SHELLCHECK_FILES_SH)" ]; then \
			LC_ALL=C.UTF-8 $(SHELLCHECK_CMD) $(SHELLCHECK_BASE_OPT) $(SHELLCHECK_IGN_OPT) $(SHELLCHECK_FILES_SH); \
		fi; \
		if [ -n "$(SHELLCHECK_FILES_INCLUDE_SH)" ]; then \
			LC_ALL=C.UTF-8 $(SHELLCHECK_CMD) $(SHELLCHECK_BASE_OPT) $(SHELLCHECK_INCLUDE_IGN_OPT) $(SHELLCHECK_FILES_INCLUDE_SH); \
		fi; \
		echo "   -> No error was detected."; \
		echo ""; \
	else \
		echo "ShellCheck is not installed, so skip check by it."; \
		echo ""; \
	fi

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
