#ifndef _COUNTRY_H
#define _COUNTRY_H	1

typedef struct ext_info
{
    const char *szName;
    unsigned short nCode;
} ext_info;

typedef struct pager_provider
{
    const char *szName;
    const char *szGate;
} pager_provider;

extern const ext_info *languages;
extern const ext_info *countries;
extern const ext_info *interests;
extern const ext_info *affilations;
extern const ext_info *occupations;
extern const ext_info *pasts;
extern const ext_info *genders;
extern const ext_info *ages;
extern const ext_info *chat_groups;

extern const pager_provider *providers;

#endif

