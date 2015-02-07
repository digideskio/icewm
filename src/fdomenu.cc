/*
 *  FDOmenu - Menu code generator for icewm
 *  Copyright (C) 2015 Eduard Bloch
 *
 *  Inspired by icewm-menu-gnome2 and Freedesktop.org specifications
 *  Using pure glib/gio code and a built-in menu structure instead
 *  the XML based external definition (as suggested by FD.o specs)
 *
 *  Release under terms of the GNU Library General Public License
 *  (version 2.0)
 *
 *  2015/02/05: Eduard Bloch <edi@gmx.de>
 *  - initial version
 */

#include "config.h"
#include "base.h"
#include "sysdep.h"
#include "intl.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gio/gdesktopappinfo.h>

template<class T>
struct auto_gfree
{
	T *p;
	auto_gfree(T *xp) : p(xp) {};
	~auto_gfree() { g_free(p); }
};

bool find_in_zArray(const char * const *arrToScan, const char *keyword)
{
	for(const gchar * const * p=arrToScan;*p;++p)
		if(!strcmp(keyword, *p))
			return true;
	return false;
}

typedef GList* pglist;

pglist msettings=0, mscreensavers=0, maccessories=0, mdevelopment=0, meducation=0,
		mgames=0, mgraphics=0, mmultimedia=0, mnetwork=0, moffice=0, msystem=0, mother=0;

//#warning needing a dupe filter for filename, maybe use GHashTable for that

void proc_dir(const char *path, unsigned depth=0)
{
	GDir *pdir = g_dir_open (path, 0, NULL);
	if(!pdir)
		return;

	const gchar *szFilename(NULL);
	while(NULL != (szFilename = g_dir_read_name (pdir)))
	{
		if(!szFilename)
			continue;
		gchar *szFullName = g_strjoin("/", path, szFilename, NULL);
		auto_gfree<gchar> xxfree(szFullName);
		static GStatBuf buf;
		if(g_stat(szFullName, &buf))
			return;
		if(S_ISDIR(buf.st_mode))
		{
			static ino_t reclog[6];
			for(unsigned i=0; i<depth; ++i)
			{
				if(reclog[i] == buf.st_ino)
					goto dir_visited_before;
			}
			if(depth<ACOUNT(reclog))
			{
				reclog[++depth] = buf.st_ino;
				proc_dir(szFullName, depth);
				--depth;
			}
			dir_visited_before:;
		}

		if(!S_ISREG(buf.st_mode))
			return;

		GDesktopAppInfo *pInfo = g_desktop_app_info_new_from_filename (szFullName);
		if(!pInfo)
			continue;

		if(!g_app_info_should_show((GAppInfo*) pInfo))
			continue;

		const char *cmdraw = g_app_info_get_commandline ((GAppInfo*) pInfo);
		if(!cmdraw || !*cmdraw)
			continue;

		// let's whitespace all special fields that we don't support
		gchar * cmd = g_strdup(cmdraw);
		auto_gfree<gchar> cmdfree(cmd);
		bool bDelChars=false;
		for(gchar *p=cmd; *p; ++p)
		{
			if('%' == *p)
				bDelChars=true;
			if(bDelChars)
				*p = ' ';
			if(bDelChars && !isprint((unsigned)*p))
				bDelChars=false;
		}

		const char *pName=g_app_info_get_name( (GAppInfo*) pInfo);
		if(!pName)
			continue;
		const char *pCats=g_desktop_app_info_get_categories(pInfo);
		if(!pCats)
			pCats="Other";

		const char *sicon = "-";
		GIcon *pIcon=g_app_info_get_icon( (GAppInfo*) pInfo);
		if (pIcon)
			sicon = g_icon_to_string(pIcon);
		char *menuLine = g_strdup_printf(
				//"# %s -> %s\n"
				"prog \"%s\" %s %s\n",
				//szFullName, pCats,
				pName, sicon, cmd);

		// Pigeonholing roughly by guessed menu structure
		gchar **ppCats = g_strsplit(pCats, ";", -1);
		if (find_in_zArray(ppCats, "Screensaver"))
			mscreensavers = g_list_append(mscreensavers, menuLine);
		else if (find_in_zArray(ppCats, "Settings"))
			msettings = g_list_append(msettings, menuLine);
		else if (find_in_zArray(ppCats, "Accessories"))
			maccessories = g_list_append(maccessories, menuLine);
		else if (find_in_zArray(ppCats, "Development"))
			mdevelopment = g_list_append(mdevelopment, menuLine);
		else if (find_in_zArray(ppCats, "Education"))
			meducation = g_list_append(meducation, menuLine);
		else if (find_in_zArray(ppCats, "Game"))
			mgames = g_list_append(mgames, menuLine);
		else if (find_in_zArray(ppCats, "Graphics"))
			mgraphics = g_list_append(mgraphics, menuLine);
		else if (find_in_zArray(ppCats, "AudioVideo") || find_in_zArray(ppCats, "Audio")
				|| find_in_zArray(ppCats, "Video"))
		{
			mmultimedia = g_list_append(mmultimedia, menuLine);
		}
		else if (find_in_zArray(ppCats, "Network"))
			mnetwork = g_list_append(mnetwork, menuLine);
		else if (find_in_zArray(ppCats, "Office"))
			moffice = g_list_append(moffice, menuLine);
		else if (find_in_zArray(ppCats, "System") || find_in_zArray(ppCats, "Emulator"))
			msystem = g_list_append(msystem, menuLine);
		else
			mother = g_list_append(mother, menuLine);

	}
	g_dir_close(pdir);
}

struct tMenuHead {
		char *title;
		pglist pEntries;
		tMenuHead(char *xt, pglist p) : title(xt), pEntries(p) {};
	};
gint menu_name_compare(gconstpointer a, gconstpointer b)
{
	tMenuHead *pa=(tMenuHead*)a;
	tMenuHead *pb=(tMenuHead*)b;
	return g_utf8_collate(pa->title, pb->title);
}

void print_submenu(gpointer l)
{
	printf("menu \"%s\" folder {\n",
					((tMenuHead*) l)->title);
			for (pglist m = ((tMenuHead*) l)->pEntries; m != NULL; m = m->next)
			{
				printf("%s", (const char*) m->data);
	// let the OS cleanup...		g_free(m->data);
			}
			printf("}\n");
}

void dump_menu()
{
	pglist xmenu = 0;
#define	addmenu(name, store) \
		xmenu = g_list_insert_sorted(xmenu, new tMenuHead(name, store),\
				(GCompareFunc)menu_name_compare);
	addmenu(_("Settings"), msettings);
	addmenu(_("Screensavers"), mscreensavers);
	addmenu(_("Accessories"), maccessories);
	addmenu(_("Development"), mdevelopment);
	addmenu(_("Education"), meducation);
	addmenu(_("Games"), mgames);
	addmenu(_("Graphics"), mgraphics);
	addmenu(_("Multimedia"), mmultimedia);
	addmenu(_("Network"), mnetwork);
	addmenu(_("Office"), moffice);
	addmenu(_("System"), msystem);

	for (pglist l = xmenu; l != NULL; l = l->next)
		print_submenu(l->data);
	puts("separator");
	print_submenu(new tMenuHead(_("Other"), mother));

}

int main(int argc, char **) {

	setlocale (LC_ALL, "");

#ifdef ENABLE_NLS
    bindtextdomain(PACKAGE, LOCDIR);
    textdomain(PACKAGE);
#endif

	const char * usershare=getenv("XDG_DATA_HOME"),
			*sysshare=getenv("XDG_DATA_DIRS");

	if(!usershare || !*usershare)
		usershare=g_strjoin(NULL, getenv("HOME"), "/.local/share", NULL);

	if(!sysshare || !*sysshare)
		sysshare="/usr/local/share/:/usr/share/";

	if(argc>1)
	{
		g_fprintf(stderr, "This program doesn't use command line options. It only listens to\n"
			"environment variables defined by XDG Base Directory Specification.\n"
			"XDG_DATA_HOME=%s\n"
				"XDG_DATA_DIRS=%s\n"
			,usershare, sysshare);
		return EXIT_FAILURE;
	}

	proc_dir(usershare);
	gchar **ppDirs = g_strsplit (sysshare, ":", -1);
	for(const gchar * const * p=ppDirs;*p;++p)
		proc_dir(g_strjoin(0, *p, "/applications", NULL));

	dump_menu();

	return EXIT_SUCCESS;
}