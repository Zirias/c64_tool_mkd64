# Created by: Felix Palmen <felix@palmen-it.de>
# $FreeBSD$

.ifdef (GIT_SNAPSHOT)
GIT_VERSIONADD!=date +%Y%m%d
GIT_VERSIONADD:=+git${GIT_VERSIONADD}
GH_COMMIT!=git rev-parse --short HEAD
GH_TAGNAME=${GH_COMMIT}
.else
GIT_VERSIONADD=
GH_COMMIT=03f7977
GH_TAGNAME=v1.4b
.endif

PORTNAME=	mkd64
PORTVERSION=	1.4b${GIT_VERSIONADD}
CATEGORIES=	devel
USE_GITHUB=	yes
GH_ACCOUNT=	Zirias
GH_PROJECT=	c64_tool_mkd64

MAINTAINER=	felix@palmen-it.de
COMMENT=	Tool for creating C64 disk files from the commandline

USES= gmake

MAKE_ARGS=CC=cc USELTO=0 prefix=/usr/local

.include <bsd.port.mk>

