/***************************************************************************
                          country.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"

namespace SIM
{

static const ext_info Countries[] =
    {
        {I18N_NOOP("Afghanistan"), 93 },
        {I18N_NOOP("Albania"), 355 },
        {I18N_NOOP("Algeria"), 213 },
        {I18N_NOOP("American Samoa"), 684 },
        {I18N_NOOP("Andorra"), 376 },
        {I18N_NOOP("Angola"), 244 },
        {I18N_NOOP("Anguilla"), 101 },
        {I18N_NOOP("Antigua"), 102 },
        {I18N_NOOP("Argentina"), 54 },
        {I18N_NOOP("Armenia"), 374 },
        {I18N_NOOP("Aruba"), 297 },
        {I18N_NOOP("Ascension Island"), 247 },
        {I18N_NOOP("Australia"), 61 },
        {I18N_NOOP("Australian Antarctic Territory"), 6721 },
        {I18N_NOOP("Austria"), 43 },
        {I18N_NOOP("Azerbaijan"), 994 },
        {I18N_NOOP("Bahamas"), 103 },
        {I18N_NOOP("Bahrain"), 973 },
        {I18N_NOOP("Bangladesh"), 880 },
        {I18N_NOOP("Barbados"), 104 },
        {I18N_NOOP("Barbuda"), 120 },
        {I18N_NOOP("Belarus"), 375 },
        {I18N_NOOP("Belgium"), 32 },
        {I18N_NOOP("Belize"), 501 },
        {I18N_NOOP("Benin"), 229 },
        {I18N_NOOP("Bermuda"), 105 },
        {I18N_NOOP("Bhutan"), 975 },
        {I18N_NOOP("Bolivia"), 591 },
        {I18N_NOOP("Bosnia and Herzegovina"), 387 },
        {I18N_NOOP("Botswana"), 267 },
        {I18N_NOOP("Brazil"), 55 },
        {I18N_NOOP("British Virgin Islands"), 106 },
        {I18N_NOOP("Brunei"), 673 },
        {I18N_NOOP("Bulgaria"), 359 },
        {I18N_NOOP("Burkina Faso"), 226 },
        {I18N_NOOP("Burundi"), 257 },
        {I18N_NOOP("Cambodia"), 855 },
        {I18N_NOOP("Cameroon"), 237 },
        {I18N_NOOP("Canada"), 107 },
        {I18N_NOOP("Cape Verde Islands"), 238 },
        {I18N_NOOP("Cayman Islands"), 108 },
        {I18N_NOOP("Central African Republic"), 236 },
        {I18N_NOOP("Chad"), 235 },
        {I18N_NOOP("Chile"), 56 },
        {I18N_NOOP("China"), 86 },
        {I18N_NOOP("Christmas Island"), 672 },
        {I18N_NOOP("Cocos-Keeling Islands"), 6101 },
        {I18N_NOOP("Colombia"), 57 },
        {I18N_NOOP("Comoros"), 2691 },
        {I18N_NOOP("Congo"), 242 },
        {I18N_NOOP("Cook Islands"), 682 },
        {I18N_NOOP("Costa Rica"), 506 },
        {I18N_NOOP("Croatia"), 385 },
        {I18N_NOOP("Cuba"), 53 },
        {I18N_NOOP("Cyprus"), 357 },
        {I18N_NOOP("Czech Republic"), 420 },
        {I18N_NOOP("Denmark"), 45 },
        {I18N_NOOP("Diego Garcia"), 246 },
        {I18N_NOOP("Djibouti"), 253 },
        {I18N_NOOP("Dominica"), 109 },
        {I18N_NOOP("Dominican Republic"), 110 },
        {I18N_NOOP("Ecuador"), 593 },
        {I18N_NOOP("Egypt"), 20 },
        {I18N_NOOP("El Salvador"), 503 },
        {I18N_NOOP("Equatorial Guinea"), 240 },
        {I18N_NOOP("Eritrea"), 291 },
        {I18N_NOOP("Estonia"), 372 },
        {I18N_NOOP("Ethiopia"), 251 },
        {I18N_NOOP("Faeroe Islands"), 298 },
        {I18N_NOOP("Falkland Islands"), 500 },
        {I18N_NOOP("Fiji Islands"), 679 },
        {I18N_NOOP("Finland"), 358 },
        {I18N_NOOP("France"), 33 },
        {I18N_NOOP("French Antilles"), 5901 },
        {I18N_NOOP("French Guiana"), 594 },
        {I18N_NOOP("French Polynesia"), 689 },
        {I18N_NOOP("Gabon"), 241 },
        {I18N_NOOP("Gambia"), 220 },
        {I18N_NOOP("Georgia"), 995 },
        {I18N_NOOP("Germany"), 49 },
        {I18N_NOOP("Ghana"), 233 },
        {I18N_NOOP("Gibraltar"), 350 },
        {I18N_NOOP("Greece"), 30 },
        {I18N_NOOP("Greenland"), 299 },
        {I18N_NOOP("Grenada"), 111 },
        {I18N_NOOP("Guadeloupe"), 590 },
        {I18N_NOOP("Guam"), 671 },
        {I18N_NOOP("Guantanamo Bay"), 5399 },
        {I18N_NOOP("Guatemala"), 502 },
        {I18N_NOOP("Guinea"), 224 },
        {I18N_NOOP("Guinea-Bissau"), 245 },
        {I18N_NOOP("Guyana"), 592 },
        {I18N_NOOP("Haiti"), 509 },
        {I18N_NOOP("Honduras"), 504 },
        {I18N_NOOP("Hong Kong"), 852 },
        {I18N_NOOP("Hungary"), 36 },
        {I18N_NOOP("INMARSAT (Atlantic-East)"), 871 },
        {I18N_NOOP("INMARSAT (Atlantic-West)"), 874 },
        {I18N_NOOP("INMARSAT (Indian)"), 873 },
        {I18N_NOOP("INMARSAT (Pacific)"), 872 },
        {I18N_NOOP("INMARSAT"), 870 },
        {I18N_NOOP("Iceland"), 354 },
        {I18N_NOOP("India"), 91 },
        {I18N_NOOP("Indonesia"), 62 },
        {I18N_NOOP("International Freephone Service"), 800 },
        {I18N_NOOP("Iran"), 98 },
        {I18N_NOOP("Iraq"), 964 },
        {I18N_NOOP("Ireland"), 353 },
        {I18N_NOOP("Israel"), 972 },
        {I18N_NOOP("Italy"), 39 },
        {I18N_NOOP("Ivory Coast"), 225 },
        {I18N_NOOP("Jamaica"), 112 },
        {I18N_NOOP("Japan"), 81 },
        {I18N_NOOP("Jordan"), 962 },
        {I18N_NOOP("Kazakhstan"), 705 },
        {I18N_NOOP("Kenya"), 254 },
        {I18N_NOOP("Kiribati Republic"), 686 },
        {I18N_NOOP("Korea (North)"), 850 },
        {I18N_NOOP("Korea (Republic of)"), 82 },
        {I18N_NOOP("Kuwait"), 965 },
        {I18N_NOOP("Kyrgyz Republic"), 706 },
        {I18N_NOOP("Laos"), 856 },
        {I18N_NOOP("Latvia"), 371 },
        {I18N_NOOP("Lebanon"), 961 },
        {I18N_NOOP("Lesotho"), 266 },
        {I18N_NOOP("Liberia"), 231 },
        {I18N_NOOP("Libya"), 218 },
        {I18N_NOOP("Liechtenstein"), 4101 },
        {I18N_NOOP("Lithuania"), 370 },
        {I18N_NOOP("Luxembourg"), 352 },
        {I18N_NOOP("Macau"), 853 },
        {I18N_NOOP("Madagascar"), 261 },
        {I18N_NOOP("Malawi"), 265 },
        {I18N_NOOP("Malaysia"), 60 },
        {I18N_NOOP("Maldives"), 960 },
        {I18N_NOOP("Mali"), 223 },
        {I18N_NOOP("Malta"), 356 },
        {I18N_NOOP("Marshall Islands"), 692 },
        {I18N_NOOP("Martinique"), 596 },
        {I18N_NOOP("Mauritania"), 222 },
        {I18N_NOOP("Mauritius"), 230 },
        {I18N_NOOP("Mayotte Island"), 269 },
        {I18N_NOOP("Mexico"), 52 },
        {I18N_NOOP("Micronesia, Federated States of"), 691 },
        {I18N_NOOP("Moldova"), 373 },
        {I18N_NOOP("Monaco"), 377 },
        {I18N_NOOP("Mongolia"), 976 },
        {I18N_NOOP("Montserrat"), 113 },
        {I18N_NOOP("Morocco"), 212 },
        {I18N_NOOP("Mozambique"), 258 },
        {I18N_NOOP("Myanmar"), 95 },
        {I18N_NOOP("Namibia"), 264 },
        {I18N_NOOP("Nauru"), 674 },
        {I18N_NOOP("Nepal"), 977 },
        {I18N_NOOP("Netherlands Antilles"), 599 },
        {I18N_NOOP("Netherlands"), 31 },
        {I18N_NOOP("Nevis"), 114 },
        {I18N_NOOP("New Caledonia"), 687 },
        {I18N_NOOP("New Zealand"), 64 },
        {I18N_NOOP("Nicaragua"), 505 },
        {I18N_NOOP("Niger"), 227 },
        {I18N_NOOP("Nigeria"), 234 },
        {I18N_NOOP("Niue"), 683 },
        {I18N_NOOP("Norfolk Island"), 6722 },
        {I18N_NOOP("Norway"), 47 },
        {I18N_NOOP("Oman"), 968 },
        {I18N_NOOP("Pakistan"), 92 },
        {I18N_NOOP("Palau"), 680 },
        {I18N_NOOP("Panama"), 507 },
        {I18N_NOOP("Papua New Guinea"), 675 },
        {I18N_NOOP("Paraguay"), 595 },
        {I18N_NOOP("Peru"), 51 },
        {I18N_NOOP("Philippines"), 63 },
        {I18N_NOOP("Poland"), 48 },
        {I18N_NOOP("Portugal"), 351 },
        {I18N_NOOP("Puerto Rico"), 121 },
        {I18N_NOOP("Qatar"), 974 },
        {I18N_NOOP("Republic of Macedonia"), 389 },
        {I18N_NOOP("Reunion Island"), 262 },
        {I18N_NOOP("Romania"), 40 },
        {I18N_NOOP("Rota Island"), 6701 },
        {I18N_NOOP("Russia"), 7 },
        {I18N_NOOP("Rwanda"), 250 },
        {I18N_NOOP("Saint Lucia"), 122 },
        {I18N_NOOP("Saipan Island"), 670 },
        {I18N_NOOP("San Marino"), 378 },
        {I18N_NOOP("Sao Tome and Principe"), 239 },
        {I18N_NOOP("Saudi Arabia"), 966 },
        {I18N_NOOP("Senegal Republic"), 221 },
        {I18N_NOOP("Seychelle Islands"), 248 },
        {I18N_NOOP("Sierra Leone"), 232 },
        {I18N_NOOP("Singapore"), 65 },
        {I18N_NOOP("Slovak Republic"), 421 },
        {I18N_NOOP("Slovenia"), 386 },
        {I18N_NOOP("Solomon Islands"), 677 },
        {I18N_NOOP("Somalia"), 252 },
        {I18N_NOOP("South Africa"), 27 },
        {I18N_NOOP("Spain"), 34 },
        {I18N_NOOP("Sri Lanka"), 94 },
        {I18N_NOOP("St. Helena"), 290 },
        {I18N_NOOP("St. Kitts"), 115 },
        {I18N_NOOP("St. Pierre and Miquelon"), 508 },
        {I18N_NOOP("St. Vincent and the Grenadines"), 116 },
        {I18N_NOOP("Sudan"), 249 },
        {I18N_NOOP("Suriname"), 597 },
        {I18N_NOOP("Swaziland"), 268 },
        {I18N_NOOP("Sweden"), 46 },
        {I18N_NOOP("Switzerland"), 41 },
        {I18N_NOOP("Syria"), 963 },
        {I18N_NOOP("Taiwan, Republic of China"), 886 },
        {I18N_NOOP("Tajikistan"), 708 },
        {I18N_NOOP("Tanzania"), 255 },
        {I18N_NOOP("Thailand"), 66 },
        {I18N_NOOP("Tinian Island"), 6702 },
        {I18N_NOOP("Togo"), 228 },
        {I18N_NOOP("Tokelau"), 690 },
        {I18N_NOOP("Tonga"), 676 },
        {I18N_NOOP("Trinidad and Tobago"), 117 },
        {I18N_NOOP("Tunisia"), 216 },
        {I18N_NOOP("Turkey"), 90 },
        {I18N_NOOP("Turkmenistan"), 709 },
        {I18N_NOOP("Turks and Caicos Islands"), 118 },
        {I18N_NOOP("Tuvalu"), 688 },
        {I18N_NOOP("USA"), 1 },
        {I18N_NOOP("Uganda"), 256 },
        {I18N_NOOP("Ukraine"), 380 },
        {I18N_NOOP("United Arab Emirates"), 971 },
        {I18N_NOOP("United Kingdom"), 44 },
        {I18N_NOOP("United States Virgin Islands"), 123 },
        {I18N_NOOP("Uruguay"), 598 },
        {I18N_NOOP("Uzbekistan"), 711 },
        {I18N_NOOP("Vanuatu"), 678 },
        {I18N_NOOP("Vatican City"), 379 },
        {I18N_NOOP("Venezuela"), 58 },
        {I18N_NOOP("Vietnam"), 84 },
        {I18N_NOOP("Wallis and Futuna Islands"), 681 },
        {I18N_NOOP("Western Samoa"), 685 },
        {I18N_NOOP("Yemen"), 967 },
        {I18N_NOOP("Yugoslavia"), 381 },
        {I18N_NOOP("Zaire"), 243 },
        {I18N_NOOP("Zambia"), 260 },
        {I18N_NOOP("Zimbabwe"), 263 },
        {"", 0 },
    };

EXPORT const ext_info *getCountries()
{
    return Countries;
}

static const pager_provider providers[] =
    {
        { "(Brazil) Access", "email.pageacess.com.br" },
        { "(Brazil) E-Trim", "etrim.com.br" },
        { "(Canada) BC Tel Mobility", "message.bctm.com" },
        { "(Canada) Select Communication", "selectcomm.ab.ca" },
        { "(Canada) TeleLink", "ds5k.telelink.com" },
        { "(Canada) Bell Mobility", "pager.mobility.com" },
        { "(India) MessageIndia", "messageindia.com" },
        { "(India) MobiLink", "MobilinkIndia.com" },
        { "(India) PageLink", "pagelink-india.com" },
        { "(Malaysia) EasyCall", "easycall.com.my" },
        { "(Mexico) WebLink Wireless (PageMart) (Traditional)", "pagemart.net" },
        { "(Mexico) WebLink Wireless (PageMart) (Advanced)", "airmessage.net" },
        { "(Russia) F1", "pager.oryol.ru" },
        { "(Russia) FCN (Full Cry Neva)", "pager.fcn.ru" },
        { "(Russia) Multi-Page", "pager.multi-page.ru" },
        { "(Russia) Mobile Telecom", "pager.mtelecom.ru" },
        { "(Russia) Mobile Telecom (subject)", "notify.mtelecom.ru" },
        { "(Singapore) Hutchinson Paging SG", "hutch.com.sg" },
        { "(Singapore) M1", "m1.com.sg" },
        { "(Ukraine) U-Page (Eurotext)", "eurotext.kiev.ua" },
        { "(USA) Ameritech/SBC", "paging.acswireless.com" },
        { "(USA) Arch", "epage.arch.com" },
        { "(USA) AT&T", "mobile.att.net" },
        { "(USA) Infinitel", "alphame.com" },
        { "(USA) Metrocall", "page.metrocall.com" },
        { "(USA) PageNet", "pagenet.net" },
        { "(USA) ProComWireless", "page.procomwireless.com" },
        { "(USA) SkyTel/MCI", "skytel.com" },
        { "(USA) TeleBeep", "page.pnet5.com" },
        { "(USA) TeleTouch", "pageme.teletouch.com" },
        { "(USA) WebLink Wireless (PageMart) (Traditional)", "pagemart.net" },
        { "(USA) WebLink Wireless (PageMart) (Advanced)", "airmessage.net" },
        { "", "" }
    };

EXPORT const pager_provider *getProviders()
{
    return providers;
}


};
