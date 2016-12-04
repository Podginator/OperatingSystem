#ifndef __CTYPE_H
#define	__CTYPE_H


/* Constants */

#define CT_UP	0x01	/* upper case */
#define CT_LOW	0x02	/* lower case */
#define CT_DIG	0x04	/* digit */
#define CT_CTL	0x08	/* control */
#define CT_PUN	0x10	/* punctuation */
#define CT_WHT	0x20	/* white space (space/cr/lf/tab) */
#define CT_HEX	0x40	/* hex digit */
#define CT_SP	0x80	/* hard space (0x20) */

/* Basic macros */

#define isascii(c)	((unsigned)(c) <= 0x7F)
#define toascii(c)	((unsigned)(c) & 0x7F)
#define isupper(c)	(c >= 'A' && c <= 'Z')
#define islower(c)	(c >= 'a' && c <= 'z')
#define tolower(c)	(isupper(c) ? c + 'a' - 'A' : c)
#define toupper(c)	(islower(c) ? c + 'A' - 'a' : c)


#endif
