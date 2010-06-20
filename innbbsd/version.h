/*  Version information */
/*
$Id: version.h 4237 2003-09-05 14:35:03Z yuhuan $
*/

#ifndef _VERSION_H_
#define _VERSION_H_

#ifndef INNBBSDVERSION
#  define INNBBSDVERSION	"0.50beta-5F"
#endif

#ifndef NCMVERSION
#  define NCMVERSION    	"NoCeM_0.63"
#endif
#if 0
#ifdef USE_NCM_PATCH
#  define VERSION       	INNBBSDVERSION"_"NCMVERSION
#else
#  define VERSION       	INNBBSDVERSION
#endif
#endif
#endif
