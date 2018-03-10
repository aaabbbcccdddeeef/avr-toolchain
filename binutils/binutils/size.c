/* size.c -- report size of various sections of an executable file.
   Copyright (C) 1991-2014 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Extensions/incompatibilities:
   o - BSD output has filenames at the end.
   o - BSD output can appear in different radicies.
   o - SysV output has less redundant whitespace.  Filename comes at end.
   o - SysV output doesn't show VMA which is always the same as the PMA.
   o - We also handle core files.
   o - We also handle archives.
   If you write shell scripts which manipulate this info then you may be
   out of luck; there's no --compatibility or --pedantic option.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "getopt.h"
#include "bucomm.h"

typedef enum
{
    format_sysv = 0,
    format_bsd = 1,
    format_avr = 2,
} format_type_t;


/* Set the default format. */
#define FORMAT_DEFAULT_SYSV 0
#define FORMAT_DEFAULT_BSD 1
#define FORMAT_DEFAULT_AVR 0

#if FORMAT_DEFAULT_SYSV
    #define FORMAT_DEFAULT format_sysv
    #define FORMAT_NAME "sysv"
#elif FORMAT_DEFAULT_BSD
    #define FORMAT_DEFAULT format_bsd
    #define FORMAT_NAME "berkeley"
#elif FORMAT_DEFAULT_AVR
    #define FORMAT_DEFAULT format_avr
    #define FORMAT_NAME "avr"
#endif


/* Program options.  */

static enum
  {
    decimal, octal, hex
  }
radix = decimal;


format_type_t format = FORMAT_DEFAULT;
static int show_version = 0;
static int show_help = 0;
static int show_totals = 0;
static int show_common = 0;

static bfd_size_type common_size;
static bfd_size_type total_bsssize;
static bfd_size_type total_datasize;
static bfd_size_type total_textsize;

/* Program exit status.  */
static int return_code = 0;


/* AVR Size specific stuff */

#define AVR64 64UL
#define AVR128 128UL
#define AVR256 256UL
#define AVR512 512UL
#define AVR2112 2112UL
#define AVR1K 1024UL
#define AVR2K 2048UL
#define AVR4K 4096UL
#define AVR8K 8192UL
#define AVR10K 10240UL
#define AVR16K 16384UL
#define AVR20K 20480UL
#define AVR24K 24576UL
#define AVR32K 32768UL
#define AVR36K 36864UL
#define AVR40K 40960UL
#define AVR64K 65536UL
#define AVR68K 69632UL
#define AVR128K 131072UL
#define AVR136K 139264UL
#define AVR200K 204800UL
#define AVR256K 262144UL
#define AVR264K 270336UL
#define AVR384K 393216UL
#define AVR392K 401408UL

typedef struct
{
    char *name;
	long flash;
	long ram;
	long eeprom;
} avr_device_t;

avr_device_t avr[] =
{
  	{"atxmega384c3",  AVR392K, AVR32K, AVR4K},
 
 	{"atxmega384d3",  AVR384K, AVR16K, AVR4K},
  
 	{"atmega256rfr2", AVR256K, AVR32K, AVR8K},
 	{"atmega2564rfr2",AVR256K, AVR32K, AVR8K},
 
  	{"atxmega256a3",  AVR264K, AVR16K, AVR4K},
 	{"atxmega256a3u", AVR264K, AVR16K, AVR4K},
  	{"atxmega256a3b", AVR264K, AVR16K, AVR4K},
 	{"atxmega256a3bu",AVR264K, AVR16K, AVR4K},
 	{"atxmega256c3",  AVR264K, AVR16K, AVR4K},
  	{"atxmega256d3",  AVR264K, AVR16K, AVR4K},
  
  	{"atmega2560",    AVR256K, AVR8K,  AVR4K},
  	{"atmega2561",    AVR256K, AVR8K,  AVR4K},
  
  	{"atxmega192a3",  AVR200K, AVR16K, AVR2K},
 	{"atxmega192a3u", AVR200K, AVR16K, AVR2K},
 	{"atxmega192c3",  AVR200K, AVR16K, AVR2K},
  	{"atxmega192d3",  AVR200K, AVR16K, AVR2K},
  
 	{"atmega128rfr2", AVR128K, AVR16K, AVR4K},
 	{"atmega1284rfr2",AVR128K, AVR16K, AVR4K},
 
  	{"atxmega128a1",  AVR136K, AVR8K,  AVR2K},
  	{"atxmega128a1u", AVR136K, AVR8K,  AVR2K},
 	{"atxmega128a4u", AVR136K, AVR8K,  AVR2K},
  	{"atxmega128a3",  AVR136K, AVR8K,  AVR2K},
 	{"atxmega128a3u", AVR136K, AVR8K,  AVR2K},
 	{"atxmega128b1",  AVR136K, AVR8K,  AVR2K},
 	{"atxmega128b3",  AVR136K, AVR8K,  AVR2K},
 	{"atxmega128c3",  AVR136K, AVR8K,  AVR2K},
  	{"atxmega128d3",  AVR136K, AVR8K,  AVR2K},
 	{"atxmega128d4",  AVR136K, AVR8K,  AVR2K},
  
  	{"at43usb320",    AVR128K, 608UL,  0UL},
  	{"at90can128",    AVR128K, AVR4K,  AVR4K},
  	{"at90usb1286",   AVR128K, AVR8K,  AVR4K},
  	{"at90usb1287",   AVR128K, AVR8K,  AVR4K},
  	{"atmega128",     AVR128K, AVR4K,  AVR4K},
 	{"atmega128a",    AVR128K, AVR4K,  AVR4K},
  	{"atmega1280",    AVR128K, AVR8K,  AVR4K},
  	{"atmega1281",    AVR128K, AVR8K,  AVR4K},
 	{"atmega1284",    AVR128K, AVR16K, AVR4K},
  	{"atmega1284p",   AVR128K, AVR16K, AVR4K},
  	{"atmega128rfa1", AVR128K, AVR16K, AVR4K},
  	{"atmega103",     AVR128K, 4000UL, AVR4K},

	{"atxmega64a1",   AVR68K,  AVR4K,  AVR2K},
	{"atxmega64a1u",  AVR68K,  AVR4K,  AVR2K},
	{"atxmega64a3",   AVR68K,  AVR4K,  AVR2K},
	{"atxmega64a3u",  AVR68K,  AVR4K,  AVR2K},
	{"atxmega64a4u",  AVR68K,  AVR4K,  AVR2K},
	{"atxmega64b1",   AVR68K,  AVR4K,  AVR2K},
	{"atxmega64b3",   AVR68K,  AVR4K,  AVR2K},
	{"atxmega64c3",   AVR68K,  AVR4K,  AVR2K},
	{"atxmega64d3",   AVR68K,  AVR4K,  AVR2K},
	{"atxmega64d4",   AVR68K,  AVR4K,  AVR2K},

	{"atmega64rfr2",  AVR64K,  AVR8K,  AVR2K},
	{"atmega644rfr2", AVR64K,  AVR8K,  AVR2K},

	{"at90can64",     AVR64K,  AVR4K,  AVR2K},
	{"at90scr100",    AVR64K,  AVR4K,  AVR2K},
	{"at90usb646",    AVR64K,  AVR4K,  AVR2K},
	{"at90usb647",    AVR64K,  AVR4K,  AVR2K},
	{"ata5505",       AVR16K,  AVR512, AVR512},
	{"ata5790",       AVR16K,  AVR512, AVR2112},
	{"ata5795",       AVR8K,   AVR512, AVR2112},
	{"ata5272",       AVR8K,   AVR512, AVR512},
	{"atmega64",      AVR64K,  AVR4K,  AVR2K},
	{"atmega64a",     AVR64K,  AVR4K,  AVR2K},
	{"atmega640",     AVR64K,  AVR8K,  AVR4K},
	{"atmega644",     AVR64K,  AVR4K,  AVR2K},
	{"atmega644a",    AVR64K,  AVR4K,  AVR2K},
	{"atmega644p",    AVR64K,  AVR4K,  AVR2K},
	{"atmega644pa",   AVR64K,  AVR4K,  AVR2K},
	{"atmega645",     AVR64K,  AVR4K,  AVR2K},
	{"atmega645a",    AVR64K,  AVR4K,  AVR2K},
	{"atmega645p",    AVR64K,  AVR4K,  AVR2K},
	{"atmega6450",    AVR64K,  AVR4K,  AVR2K},
	{"atmega6450a",   AVR64K,  AVR4K,  AVR2K},
	{"atmega6450p",   AVR64K,  AVR4K,  AVR2K},
	{"atmega649",     AVR64K,  AVR4K,  AVR2K},
	{"atmega649a",    AVR64K,  AVR4K,  AVR2K},
	{"atmega649p",    AVR64K,  AVR4K,  AVR2K},
	{"atmega6490",    AVR64K,  AVR4K,  AVR2K},
	{"atmega6490a",   AVR64K,  AVR4K,  AVR2K},
	{"atmega6490p",   AVR64K,  AVR4K,  AVR2K},
	{"atmega64c1",    AVR64K,  AVR4K,  AVR2K},
	{"atmega64hve",   AVR64K,  AVR4K,  AVR1K},
	{"atmega64m1",    AVR64K,  AVR4K,  AVR2K},
	{"m3000",         AVR64K,  AVR4K,  0UL},

	{"atmega406",     AVR40K,  AVR2K,  AVR512},

	{"atxmega32a4",   AVR36K,  AVR4K,  AVR1K},
	{"atxmega32a4u",  AVR36K,  AVR4K,  AVR1K},
	{"atxmega32c4",   AVR36K,  AVR4K,  AVR1K},
	{"atxmega32d4",   AVR36K,  AVR4K,  AVR1K},
	{"atxmega32e5",   AVR36K,  AVR4K,  AVR1K},
	{"atxmega16e5",   AVR20K,  AVR2K,  AVR512},
	{"atxmega8e5",	  AVR10K,  AVR1K,  AVR512},

	{"at90can32",     AVR32K,  AVR2K,  AVR1K},
	{"at94k",         AVR32K,  AVR4K,  0UL},
	{"atmega32",      AVR32K,  AVR2K,  AVR1K},
	{"atmega32a",     AVR32K,  AVR2K,  AVR1K},
	{"atmega323",     AVR32K,  AVR2K,  AVR1K},
	{"atmega324a",    AVR32K,  AVR2K,  AVR1K},
	{"atmega324p",    AVR32K,  AVR2K,  AVR1K},
	{"atmega324pa",   AVR32K,  AVR2K,  AVR1K},
	{"atmega325",     AVR32K,  AVR2K,  AVR1K},
	{"atmega325a",    AVR32K,  AVR2K,  AVR1K},
	{"atmega325p",    AVR32K,  AVR2K,  AVR1K},
	{"atmega325pa",   AVR32K,  AVR2K,  AVR1K},
	{"atmega3250",    AVR32K,  AVR2K,  AVR1K},
	{"atmega3250a",   AVR32K,  AVR2K,  AVR1K},
	{"atmega3250p",   AVR32K,  AVR2K,  AVR1K},
	{"atmega3250pa",  AVR32K,  AVR2K,  AVR1K},
	{"atmega328",     AVR32K,  AVR2K,  AVR1K},
	{"atmega328p",    AVR32K,  AVR2K,  AVR1K},
	{"atmega329",     AVR32K,  AVR2K,  AVR1K},
	{"atmega329a",    AVR32K,  AVR2K,  AVR1K},
	{"atmega329p",    AVR32K,  AVR2K,  AVR1K},
	{"atmega329pa",   AVR32K,  AVR2K,  AVR1K},
	{"atmega3290",    AVR32K,  AVR2K,  AVR1K},
	{"atmega3290a",   AVR32K,  AVR2K,  AVR1K},
	{"atmega3290p",   AVR32K,  AVR2K,  AVR1K},
	{"atmega3290pa",  AVR32K,  AVR2K,  AVR1K},
	{"atmega32hvb",   AVR32K,  AVR2K,  AVR1K},
	{"atmega32hvbrevb",AVR32K, AVR2K,  AVR1K},
	{"atmega32c1",    AVR32K,  AVR2K,  AVR1K},
	{"atmega32m1",    AVR32K,  AVR2K,  AVR1K},
	{"atmega32u2",    AVR32K,  AVR1K,  AVR1K},
	{"atmega32u4",    AVR32K,  2560UL, AVR1K},
	{"atmega32u6",    AVR32K,  2560UL, AVR1K},

	{"at43usb355",    AVR24K,  1120UL,   0UL},

	{"atxmega16a4",   AVR20K,  AVR2K,  AVR1K},
        {"atxmega16a4u",  AVR20K,  AVR2K,  AVR1K},
	{"atxmega16c4",   AVR20K,  AVR2K,  AVR1K},
	{"atxmega16d4",   AVR20K,  AVR2K,  AVR1K},

	{"at76c711",      AVR16K,  AVR2K,  0UL},
	{"at90pwm161",    AVR16K,  AVR1K,  AVR512},
	{"at90pwm216",    AVR16K,  AVR1K,  AVR512},
	{"at90pwm316",    AVR16K,  AVR1K,  AVR512},
	{"at90usb162",    AVR16K,  AVR512, AVR512},
	{"atmega16",      AVR16K,  AVR1K,  AVR512},
	{"atmega16a",     AVR16K,  AVR1K,  AVR512},
	{"atmega161",     AVR16K,  AVR1K,  AVR512},
	{"atmega162",     AVR16K,  AVR1K,  AVR512},
	{"atmega163",     AVR16K,  AVR1K,  AVR512},
	{"atmega164",     AVR16K,  AVR1K,  AVR512},
	{"atmega164a",    AVR16K,  AVR1K,  AVR512},
	{"atmega164p",    AVR16K,  AVR1K,  AVR512},
	{"atmega164pa",   AVR16K,  AVR1K,  AVR512},
	{"atmega165a",    AVR16K,  AVR1K,  AVR512},
	{"atmega165",     AVR16K,  AVR1K,  AVR512},
	{"atmega165p",    AVR16K,  AVR1K,  AVR512},
	{"atmega165pa",   AVR16K,  AVR1K,  AVR512},
	{"atmega168",     AVR16K,  AVR1K,  AVR512},
	{"atmega168a",    AVR16K,  AVR1K,  AVR512},
	{"atmega168p",    AVR16K,  AVR1K,  AVR512},
	{"atmega168pa",   AVR16K,  AVR1K,  AVR512},
	{"atmega169",     AVR16K,  AVR1K,  AVR512},
	{"atmega169a",    AVR16K,  AVR1K,  AVR512},
	{"atmega169p",    AVR16K,  AVR1K,  AVR512},
	{"atmega169pa",   AVR16K,  AVR1K,  AVR512},
	{"atmega16hva",   AVR16K,  768UL,  AVR256},
	{"atmega16hva2",  AVR16K,  AVR1K,  AVR256},
        {"atmega16hvb",   AVR16K,  AVR1K,  AVR512},
        {"atmega16hvbrevb",AVR16K, AVR1K,  AVR512},
	{"atmega16m1",    AVR16K,  AVR1K,  AVR512},
	{"attiny1634",    AVR16K,  AVR1K,  AVR256},
	{"atmega16u2",    AVR16K,  AVR512, AVR512},
	{"atmega16u4",    AVR16K,  1280UL, AVR512},
	{"attiny167",     AVR16K,  AVR512, AVR512},

	{"at90c8534",     AVR8K,   352UL,  AVR512},
	{"at90pwm1",      AVR8K,   AVR512, AVR512},
	{"at90pwm2",      AVR8K,   AVR512, AVR512},
	{"at90pwm2b",     AVR8K,   AVR512, AVR512},
	{"at90pwm3",      AVR8K,   AVR512, AVR512},
	{"at90pwm3b",     AVR8K,   AVR512, AVR512},
	{"at90pwm81",     AVR8K,   AVR256, AVR512},
	{"at90s8515",     AVR8K,   AVR512, AVR512},
	{"at90s8535",     AVR8K,   AVR512, AVR512},
	{"at90usb82",     AVR8K,   AVR512, AVR512},
	{"ata6285",       AVR8K,   AVR512, 320UL},
	{"ata6286",       AVR8K,   AVR512, 320UL},
	{"ata6289",       AVR8K,   AVR512, 320UL},
	{"atmega8",       AVR8K,   AVR1K,  AVR512},
	{"atmega8a",      AVR8K,   AVR1K,  AVR512},
	{"atmega8515",    AVR8K,   AVR512, AVR512},
	{"atmega8535",    AVR8K,   AVR512, AVR512},
	{"atmega88",      AVR8K,   AVR1K,  AVR512},
	{"atmega88a",     AVR8K,   AVR1K,  AVR512},
	{"atmega88p",     AVR8K,   AVR1K,  AVR512},
	{"atmega88pa",    AVR8K,   AVR1K,  AVR512},
	{"atmega8hva",    AVR8K,   768UL,  AVR256},
	{"atmega8u2",     AVR8K,   AVR512, AVR512},
	{"attiny84",      AVR8K,   AVR512, AVR512},
	{"attiny84a",     AVR8K,   AVR512, AVR512},
	{"attiny85",      AVR8K,   AVR512, AVR512},
	{"attiny861",     AVR8K,   AVR512, AVR512},
	{"attiny861a",    AVR8K,   AVR512, AVR512},
	{"attiny87",      AVR8K,   AVR512, AVR512},
	{"attiny88",      AVR8K,   AVR512, AVR64},

	{"at90s4414",     AVR4K,   352UL,  AVR256},
	{"at90s4433",     AVR4K,   AVR128, AVR256},
	{"at90s4434",     AVR4K,   352UL,  AVR256},
	{"atmega48",      AVR4K,   AVR512, AVR256},
	{"atmega48a",     AVR4K,   AVR512, AVR256},
	{"atmega48pa",    AVR4K,   AVR512, AVR256},
	{"atmega48p",     AVR4K,   AVR512, AVR256},
	{"attiny4313",    AVR4K,   AVR256, AVR256},
	{"attiny43u",     AVR4K,   AVR256, AVR64},
	{"attiny44",      AVR4K,   AVR256, AVR256},
	{"attiny44a",     AVR4K,   AVR256, AVR256},
	{"attiny45",      AVR4K,   AVR256, AVR256},
	{"attiny461",     AVR4K,   AVR256, AVR256},
	{"attiny461a",    AVR4K,   AVR256, AVR256},
	{"attiny48",      AVR4K,   AVR256, AVR64},
	{"attiny828",     AVR8K,   AVR512, AVR256},

	{"at86rf401",     AVR2K,   224UL,  AVR128},
	{"at90s2313",     AVR2K,   AVR128, AVR128},
	{"at90s2323",     AVR2K,   AVR128, AVR128},
	{"at90s2333",     AVR2K,   224UL,  AVR128},
	{"at90s2343",     AVR2K,   AVR128, AVR128},
	{"attiny20",      AVR2K,   AVR128, 0UL},
	{"attiny22",      AVR2K,   224UL,  AVR128},
	{"attiny2313",    AVR2K,   AVR128, AVR128},
	{"attiny2313a",   AVR2K,   AVR128, AVR128},
	{"attiny24",      AVR2K,   AVR128, AVR128},
	{"attiny24a",     AVR2K,   AVR128, AVR128},
	{"attiny25",      AVR2K,   AVR128, AVR128},
	{"attiny26",      AVR2K,   AVR128, AVR128},
	{"attiny261",     AVR2K,   AVR128, AVR128},
	{"attiny261a",    AVR2K,   AVR128, AVR128},
	{"attiny28",      AVR2K,   0UL,    0UL},
	{"attiny40",      AVR2K,   AVR256, 0UL},

	{"at90s1200",     AVR1K,   0UL,    AVR64},
	{"attiny9",       AVR1K,   32UL,   0UL},
	{"attiny10",      AVR1K,   32UL,   0UL},
	{"attiny11",      AVR1K,   0UL,    AVR64},
	{"attiny12",      AVR1K,   0UL,    AVR64},
	{"attiny13",      AVR1K,   AVR64,  AVR64},
	{"attiny13a",     AVR1K,   AVR64,  AVR64},
	{"attiny15",      AVR1K,   0UL,    AVR64},

	{"attiny4",       AVR512,  32UL,   0UL},
	{"attiny5",       AVR512,  32UL,   0UL},
};

static char *avrmcu = NULL;


static char *target = NULL;

/* Forward declarations.  */

static void display_file (char *);
static void rprint_number (int, bfd_size_type);
static void print_sizes (bfd * file);

static void
usage (FILE *stream, int status)
{
  fprintf (stream, _("Usage: %s [option(s)] [file(s)]\n"), program_name);
  fprintf (stream, _(" Displays the sizes of sections inside binary files\n"));
  fprintf (stream, _(" If no input file(s) are specified, a.out is assumed\n"));
  fprintf (stream, _(" The options are:\n\
  -A|-B|-C  --format={sysv|berkeley|avr}  Select output style (default is %s)\n\
            --mcu=<avrmcu>            MCU name for AVR format only\n\
            --mlist-devices           List all supported MCUs\n\
  -o|-d|-x  --radix={8|10|16}         Display numbers in octal, decimal or hex\n\
  -t        --totals                  Display the total sizes (Berkeley only)\n\
            --common                  Display total size for *COM* syms\n\
            --target=<bfdname>        Set the binary file format\n\
            @<file>                   Read options from <file>\n\
  -h        --help                    Display this information\n\
  -v        --version                 Display the program's version\n\
\n"),
FORMAT_NAME
);
  list_supported_targets (program_name, stream);
  if (REPORT_BUGS_TO[0] && status == 0)
    fprintf (stream, _("Report bugs to %s\n"), REPORT_BUGS_TO);
  exit (status);
}

#define OPTION_FORMAT (200)
#define OPTION_RADIX (OPTION_FORMAT + 1)
#define OPTION_TARGET (OPTION_RADIX + 1)
#define OPTION_MCU (OPTION_TARGET + 1) 
#define OPTION_MLIST_DEVICES (OPTION_MCU + 1)

static struct option long_options[] =
{
  {"common", no_argument, &show_common, 1},
  {"format", required_argument, 0, OPTION_FORMAT},
  {"radix", required_argument, 0, OPTION_RADIX},
  {"target", required_argument, 0, OPTION_TARGET},
  {"mcu", required_argument, 0, 203},
  {"mlist-devices", no_argument, 0, OPTION_MLIST_DEVICES},
  {"totals", no_argument, &show_totals, 1},
  {"version", no_argument, &show_version, 1},
  {"help", no_argument, &show_help, 1},
  {0, no_argument, 0, 0}
};

static void list_supported_devices (FILE *stream, int status)
{
  unsigned int i ;
  int llen = 0;
  fprintf (stream, _("List of supported devices:\n"));
  for (i = 0; i < sizeof(avr) / sizeof(avr[0]); i++)
  {
    int slen = strlen(avr[i].name);
    llen += slen + 1 ;
    if (llen < 75)
    {
      fprintf (stream, _(" %s"), avr[i].name);
    }
    else
    {
      fprintf (stream, _("\n %s"), avr[i].name);
      llen = slen + 2 ;
    }
  }
  fprintf (stream, "\n");

  exit (status);
}

int main (int, char **);

int
main (int argc, char **argv)
{
  int temp;
  int c;

#if defined (HAVE_SETLOCALE) && defined (HAVE_LC_MESSAGES)
  setlocale (LC_MESSAGES, "");
#endif
#if defined (HAVE_SETLOCALE)
  setlocale (LC_CTYPE, "");
#endif
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  program_name = *argv;
  xmalloc_set_program_name (program_name);

  expandargv (&argc, &argv);

  bfd_init ();
  set_default_bfd_target ();

  while ((c = getopt_long (argc, argv, "ABCHhVvdfotx", long_options,
			   (int *) 0)) != EOF)
    switch (c)
      {
      case OPTION_FORMAT:
	switch (*optarg)
	  {
	  case 'B':
	  case 'b':
	    format = format_bsd;
	    break;
	  case 'S':
	  case 's':
	    format = format_sysv;
	    break;
	  case 'A':
	  case 'a':
	    format = format_avr;
	    break;
	  default:
	    non_fatal (_("invalid argument to --format: %s"), optarg);
	    usage (stderr, 1);
	  }
	break;

      case OPTION_MCU:
	avrmcu = optarg;
	break;

      case OPTION_MLIST_DEVICES:
	list_supported_devices (stdout, 1);
	break;

      case OPTION_TARGET:
	target = optarg;
	break;

      case OPTION_RADIX:
#ifdef ANSI_LIBRARIES
	temp = strtol (optarg, NULL, 10);
#else
	temp = atol (optarg);
#endif
	switch (temp)
	  {
	  case 10:
	    radix = decimal;
	    break;
	  case 8:
	    radix = octal;
	    break;
	  case 16:
	    radix = hex;
	    break;
	  default:
	    non_fatal (_("Invalid radix: %s\n"), optarg);
	    usage (stderr, 1);
	  }
	break;

      case 'A':
	format = format_sysv;
	break;
      case 'B':
	format = format_bsd;
	break;
      case 'C':
    format = format_avr;
    break;
      case 'v':
      case 'V':
	show_version = 1;
	break;
      case 'd':
	radix = decimal;
	break;
      case 'x':
	radix = hex;
	break;
      case 'o':
	radix = octal;
	break;
      case 't':
	show_totals = 1;
	break;
      case 'f': /* FIXME : For sysv68, `-f' means `full format', i.e.
		   `[fname:] M(.text) + N(.data) + O(.bss) + P(.comment) = Q'
		   where `fname: ' appears only if there are >= 2 input files,
		   and M, N, O, P, Q are expressed in decimal by default,
		   hexa or octal if requested by `-x' or `-o'.
		   Just to make things interesting, Solaris also accepts -f,
		   which prints out the size of each allocatable section, the
		   name of the section, and the total of the section sizes.  */
		/* For the moment, accept `-f' silently, and ignore it.  */
	break;
      case 0:
	break;
      case 'h':
      case 'H':
      case '?':
	usage (stderr, 1);
      }

  if (show_version)
    print_version ("size");
  if (show_help)
    usage (stdout, 0);

  if (optind == argc)
    display_file ("a.out");
  else
    for (; optind < argc;)
      display_file (argv[optind++]);

  if (show_totals && format == format_bsd)
    {
      bfd_size_type total = total_textsize + total_datasize + total_bsssize;

      rprint_number (7, total_textsize);
      putchar('\t');
      rprint_number (7, total_datasize);
      putchar('\t');
      rprint_number (7, total_bsssize);
      printf (((radix == octal) ? "\t%7lo\t%7lx\t" : "\t%7lu\t%7lx\t"),
	      (unsigned long) total, (unsigned long) total);
      fputs ("(TOTALS)\n", stdout);
    }

  return return_code;
}

/* Total size required for common symbols in ABFD.  */

static void
calculate_common_size (bfd *abfd)
{
  asymbol **syms = NULL;
  long storage, symcount;

  common_size = 0;
  if ((bfd_get_file_flags (abfd) & (EXEC_P | DYNAMIC | HAS_SYMS)) != HAS_SYMS)
    return;

  storage = bfd_get_symtab_upper_bound (abfd);
  if (storage < 0)
    bfd_fatal (bfd_get_filename (abfd));
  if (storage)
    syms = (asymbol **) xmalloc (storage);

  symcount = bfd_canonicalize_symtab (abfd, syms);
  if (symcount < 0)
    bfd_fatal (bfd_get_filename (abfd));

  while (--symcount >= 0)
    {
      asymbol *sym = syms[symcount];

      if (bfd_is_com_section (sym->section)
	  && (sym->flags & BSF_SECTION_SYM) == 0)
	common_size += sym->value;
    }
  free (syms);
}

/* Display stats on file or archive member ABFD.  */

static void
display_bfd (bfd *abfd)
{
  char **matching;

  if (bfd_check_format (abfd, bfd_archive))
    /* An archive within an archive.  */
    return;

  if (bfd_check_format_matches (abfd, bfd_object, &matching))
    {
      print_sizes (abfd);
      printf ("\n");
      return;
    }

  if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
    {
      bfd_nonfatal (bfd_get_filename (abfd));
      list_matching_formats (matching);
      free (matching);
      return_code = 3;
      return;
    }

  if (bfd_check_format_matches (abfd, bfd_core, &matching))
    {
      const char *core_cmd;

      print_sizes (abfd);
      fputs (" (core file", stdout);

      core_cmd = bfd_core_file_failing_command (abfd);
      if (core_cmd)
	printf (" invoked as %s", core_cmd);

      puts (")\n");
      return;
    }

  bfd_nonfatal (bfd_get_filename (abfd));

  if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
    {
      list_matching_formats (matching);
      free (matching);
    }

  return_code = 3;
}

static void
display_archive (bfd *file)
{
  bfd *arfile = (bfd *) NULL;
  bfd *last_arfile = (bfd *) NULL;

  for (;;)
    {
      bfd_set_error (bfd_error_no_error);

      arfile = bfd_openr_next_archived_file (file, arfile);
      if (arfile == NULL)
	{
	  if (bfd_get_error () != bfd_error_no_more_archived_files)
	    {
	      bfd_nonfatal (bfd_get_filename (file));
	      return_code = 2;
	    }
	  break;
	}

      display_bfd (arfile);

      if (last_arfile != NULL)
	bfd_close (last_arfile);
      last_arfile = arfile;
    }

  if (last_arfile != NULL)
    bfd_close (last_arfile);
}

static void
display_file (char *filename)
{
  bfd *file;

  if (get_file_size (filename) < 1)
    {
      return_code = 1;
      return;
    }

  file = bfd_openr (filename, target);
  if (file == NULL)
    {
      bfd_nonfatal (filename);
      return_code = 1;
      return;
    }

  if (bfd_check_format (file, bfd_archive))
    display_archive (file);
  else
    display_bfd (file);

  if (!bfd_close (file))
    {
      bfd_nonfatal (filename);
      return_code = 1;
      return;
    }
}

static int
size_number (bfd_size_type num)
{
  char buffer[40];

  sprintf (buffer,
	   (radix == decimal ? "%" BFD_VMA_FMT "u" :
	   ((radix == octal) ? "0%" BFD_VMA_FMT "o" : "0x%" BFD_VMA_FMT "x")),
	   num);

  return strlen (buffer);
}

static void
rprint_number (int width, bfd_size_type num)
{
  char buffer[40];

  sprintf (buffer,
	   (radix == decimal ? "%" BFD_VMA_FMT "u" :
	   ((radix == octal) ? "0%" BFD_VMA_FMT "o" : "0x%" BFD_VMA_FMT "x")),
	   num);

  printf ("%*s", width, buffer);
}

static bfd_size_type bsssize;
static bfd_size_type datasize;
static bfd_size_type textsize;

static void
berkeley_sum (bfd *abfd ATTRIBUTE_UNUSED, sec_ptr sec,
	      void *ignore ATTRIBUTE_UNUSED)
{
  flagword flags;
  bfd_size_type size;

  flags = bfd_get_section_flags (abfd, sec);
  if ((flags & SEC_ALLOC) == 0)
    return;

  size = bfd_get_section_size (sec);
  if ((flags & SEC_CODE) != 0 || (flags & SEC_READONLY) != 0)
    textsize += size;
  else if ((flags & SEC_HAS_CONTENTS) != 0)
    datasize += size;
  else
    bsssize += size;
}

static void
print_berkeley_format (bfd *abfd)
{
  static int files_seen = 0;
  bfd_size_type total;

  bsssize = 0;
  datasize = 0;
  textsize = 0;

  bfd_map_over_sections (abfd, berkeley_sum, NULL);

  bsssize += common_size;
  if (files_seen++ == 0)
    puts ((radix == octal) ? "   text\t   data\t    bss\t    oct\t    hex\tfilename" :
	  "   text\t   data\t    bss\t    dec\t    hex\tfilename");

  total = textsize + datasize + bsssize;

  if (show_totals)
    {
      total_textsize += textsize;
      total_datasize += datasize;
      total_bsssize  += bsssize;
    }

  rprint_number (7, textsize);
  putchar ('\t');
  rprint_number (7, datasize);
  putchar ('\t');
  rprint_number (7, bsssize);
  printf (((radix == octal) ? "\t%7lo\t%7lx\t" : "\t%7lu\t%7lx\t"),
	  (unsigned long) total, (unsigned long) total);

  fputs (bfd_get_filename (abfd), stdout);

  if (bfd_my_archive (abfd))
    printf (" (ex %s)", bfd_get_filename (bfd_my_archive (abfd)));
}

/* I REALLY miss lexical functions! */
bfd_size_type svi_total = 0;
bfd_vma svi_maxvma = 0;
int svi_namelen = 0;
int svi_vmalen = 0;
int svi_sizelen = 0;

static void
sysv_internal_sizer (bfd *file ATTRIBUTE_UNUSED, sec_ptr sec,
		     void *ignore ATTRIBUTE_UNUSED)
{
  bfd_size_type size = bfd_section_size (file, sec);

  if (   ! bfd_is_abs_section (sec)
      && ! bfd_is_com_section (sec)
      && ! bfd_is_und_section (sec))
    {
      int namelen = strlen (bfd_section_name (file, sec));

      if (namelen > svi_namelen)
	svi_namelen = namelen;

      svi_total += size;

      if (bfd_section_vma (file, sec) > svi_maxvma)
	svi_maxvma = bfd_section_vma (file, sec);
    }
}

static void
sysv_one_line (const char *name, bfd_size_type size, bfd_vma vma)
{
  printf ("%-*s   ", svi_namelen, name);
  rprint_number (svi_sizelen, size);
  printf ("   ");
  rprint_number (svi_vmalen, vma);
  printf ("\n");
}

static void
sysv_internal_printer (bfd *file ATTRIBUTE_UNUSED, sec_ptr sec,
		       void *ignore ATTRIBUTE_UNUSED)
{
  bfd_size_type size = bfd_section_size (file, sec);

  if (   ! bfd_is_abs_section (sec)
      && ! bfd_is_com_section (sec)
      && ! bfd_is_und_section (sec))
    {
      svi_total += size;

      sysv_one_line (bfd_section_name (file, sec),
		     size,
		     bfd_section_vma (file, sec));
    }
}

static void
print_sysv_format (bfd *file)
{
  /* Size all of the columns.  */
  svi_total = 0;
  svi_maxvma = 0;
  svi_namelen = 0;
  bfd_map_over_sections (file, sysv_internal_sizer, NULL);
  if (show_common)
    {
      if (svi_namelen < (int) sizeof ("*COM*") - 1)
	svi_namelen = sizeof ("*COM*") - 1;
      svi_total += common_size;
    }

  svi_vmalen = size_number ((bfd_size_type)svi_maxvma);

  if ((size_t) svi_vmalen < sizeof ("addr") - 1)
    svi_vmalen = sizeof ("addr")-1;

  svi_sizelen = size_number (svi_total);
  if ((size_t) svi_sizelen < sizeof ("size") - 1)
    svi_sizelen = sizeof ("size")-1;

  svi_total = 0;
  printf ("%s  ", bfd_get_filename (file));

  if (bfd_my_archive (file))
    printf (" (ex %s)", bfd_get_filename (bfd_my_archive (file)));

  printf (":\n%-*s   %*s   %*s\n", svi_namelen, "section",
	  svi_sizelen, "size", svi_vmalen, "addr");

  bfd_map_over_sections (file, sysv_internal_printer, NULL);
  if (show_common)
    {
      svi_total += common_size;
      sysv_one_line ("*COM*", common_size, 0);
    }

  printf ("%-*s   ", svi_namelen, "Total");
  rprint_number (svi_sizelen, svi_total);
  printf ("\n\n");
}


static avr_device_t *
avr_find_device (void)
{
  unsigned int i;
  if (avrmcu != NULL)
  {
    for (i = 0; i < sizeof(avr) / sizeof(avr[0]); i++)
    {
      if (strcmp(avr[i].name, avrmcu) == 0)
      {
        /* Match found */
        return (&avr[i]);
      }
    }
  }
  return (NULL);
}



static void
print_avr_format (bfd *file)
{
  char *avr_name = "Unknown";
  int flashmax = 0;
  int rammax = 0;
  int eeprommax = 0;
  asection *section; 
  bfd_size_type avr_datasize = 0;
  bfd_size_type avr_textsize = 0;
  bfd_size_type avr_bsssize = 0;
  bfd_size_type bootloadersize = 0;
  bfd_size_type noinitsize = 0;
  bfd_size_type eepromsize = 0;

  avr_device_t *avrdevice = avr_find_device();
  if (avrdevice != NULL)
  {
    avr_name = avrdevice->name;
    flashmax = avrdevice->flash;
    rammax = avrdevice->ram;
    eeprommax = avrdevice->eeprom;
  }

  if ((section = bfd_get_section_by_name (file, ".data")) != NULL)
    avr_datasize = bfd_section_size (file, section);
  if ((section = bfd_get_section_by_name (file, ".text")) != NULL)
    avr_textsize = bfd_section_size (file, section);
  if ((section = bfd_get_section_by_name (file, ".bss")) != NULL)
    avr_bsssize = bfd_section_size (file, section);
  if ((section = bfd_get_section_by_name (file, ".bootloader")) != NULL)
    bootloadersize = bfd_section_size (file, section);
  if ((section = bfd_get_section_by_name (file, ".noinit")) != NULL)
    noinitsize = bfd_section_size (file, section);
  if ((section = bfd_get_section_by_name (file, ".eeprom")) != NULL)
    eepromsize = bfd_section_size (file, section);
  
  bfd_size_type text = avr_textsize + avr_datasize + bootloadersize;
  bfd_size_type data = avr_datasize + avr_bsssize + noinitsize;
  bfd_size_type eeprom = eepromsize;
  
  printf ("AVR Memory Usage\n"
          "----------------\n"
          "Device: %s\n\n", avr_name);
  
  /* Text size */
  printf ("Program:%8ld bytes", text);
  if (flashmax > 0)
  {
    printf (" (%2.1f%% Full)", ((float)text / flashmax) * 100);
  }
  printf ("\n(.text + .data + .bootloader)\n\n");
  
  /* Data size */
  printf ("Data:   %8ld bytes", data);
  if (rammax > 0)
  {
    printf (" (%2.1f%% Full)", ((float)data / rammax) * 100);
  }
  printf ("\n(.data + .bss + .noinit)\n\n");
  
  /* EEPROM size */
  if (eeprom > 0) 
  { 
    printf ("EEPROM: %8ld bytes", eeprom);
    if (eeprommax > 0)
    {
      printf (" (%2.1f%% Full)", ((float)eeprom / eeprommax) * 100);
    }
    printf ("\n(.eeprom)\n\n");
  }
}


static void
print_sizes (bfd *file)
{
  if (show_common)
    calculate_common_size (file);
  switch (format)
  {
    case format_sysv:
      print_sysv_format (file);
      break;
    case format_bsd:
      print_berkeley_format (file);
      break;
    case format_avr:
    default:
      print_avr_format (file);
      break;
  }
}
