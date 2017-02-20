# New ports collection makefile for:	ici-cgi
# Date created:				Tue Jun 19 22:59:35 EST 2001
# Whom:					Andy Newman <atrn@zeta.org.au>
#
# $FreeBSD$
#

PORTNAME=	cgi
PORTVERSION=	0.1
CATEGORIES=	lang
MASTER_SITES=	http://www.zeta.org.au/~atrn/ici/modules/distfiles/
DIST_SUBDIR=	ici
MAINTAINER=	atrn@zeta.org.au

PKGNAMEPREFIX=	ici-
DISTNAME=	${PKGNAMEPREFIX}${PORTNAME}-${PORTVERSION}
BUILD_DEPENDS=	ici3:${PORTSDIR}/lang/ici

NO_BUILD=	yes
MAKE_ENV=	ICIV=ici3

do-install:
	@[ -d ${PREFIX}/lib/ici3 ] || mkdir -p ${PREFIX}/lib/ici3
	@( cd ${WRKSRC} && ${MAKE} ICIV=ici3 install )

.include <bsd.port.mk>
