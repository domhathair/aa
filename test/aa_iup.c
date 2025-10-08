#include "alloc.h"
#include <assert.h>
#include <iup/iup.h>
#include <stdlib.h>

#ifdef TEST_AA_IUP

#define AA_KEY char *
#define AA_VALUE void *
#define AA_IMPLEMENTATION
#include "aa.h"

#ifndef countof
#define countof(array) (sizeof(array) / sizeof(*array))
#endif

static struct aa *a;

struct locale {
    const char *language;
    struct pairs {
        const char *key, *value;
    } entry[7];
} locale[] = {

    {"English",
     {
         {"Open", "Open"},
         {"Save As", "Save As"},
         {"Exit", "Exit"},
         {"Locale", "Locale"},
         {"File", "File"},
         {"Edit", "Edit"},
         {"Notepad", "Notepad"},
     }},
    {"Русский",
     {
         {"Open", "Открыть"},
         {"Save As", "Сохранить как"},
         {"Exit", "Выход"},
         {"Locale", "Язык"},
         {"File", "Файл"},
         {"Edit", "Редактировать"},
         {"Notepad", "Блокнот"},
     }},
    {"Deutsch",
     {
         {"Open", "Öffnen"},
         {"Save As", "Speichern unter"},
         {"Exit", "Beenden"},
         {"Locale", "Gebietsschema"},
         {"File", "Datei"},
         {"Edit", "Bearbeiten"},
         {"Notepad", "Notizblock"},
     }},
};

static struct aa *pair = NULL;
static bool redraw;
static Ihandle *dlg;

static int exit_cb(Ihandle *) { return IUP_CLOSE; }

static int locale_cb(Ihandle *locale) {
    struct aa *o = pair;

    if (aa_get(a, IupGetAttribute(locale, "TITLE"), &pair) == 0 && pair != o) {
        redraw = true;
        IupHide(dlg);
    }

    return IUP_CONTINUE;
}

static const char *_(const char *$) {
    char *out = NULL;

    if (pair)
        if (aa_get(pair, $, &out) == 0)
            return out;

    return $;
}

int main(int argc, char **argv) {
    a = aa_new();

    for (size_t i = 0; i < countof(locale); i++) {
        struct aa *pair = aa_new();
        aa_set(a, locale[i].language, pair);
        for (size_t j = 0; j < countof(locale->entry); j++)
            aa_set(pair, locale[i].entry[j].key, locale[i].entry[j].value);
    }

    do {
        redraw = false;

        IupOpen(&argc, &argv);

        Ihandle *multitext = IupText(NULL);
        IupSetAttribute(multitext, "MULTILINE", "YES");
        IupSetAttribute(multitext, "EXPAND", "YES");

        /* File */
        Ihandle *item_open = IupItem(_("Open"), NULL);
        Ihandle *item_saveas = IupItem(_("Save As"), NULL);
        Ihandle *item_exit = IupItem(_("Exit"), NULL);
        IupSetCallback(item_exit, "ACTION", (Icallback)exit_cb);

        /* Edit */
        Ihandle *locale_menu = IupMenu(NULL);
        for (struct aa_node *node = NULL; (node = aa_next(a));) {
            /* Add locales from table above */
            Ihandle *item_locale = IupItem(node->key, NULL);
            IupSetCallback(item_locale, "ACTION", (Icallback)locale_cb);
            IupAppend(locale_menu, item_locale);
        }

        /* Menu */
        Ihandle *file_menu = IupMenu(item_open, item_saveas, IupSeparator(), item_exit, NULL);
        Ihandle *edit_menu = IupMenu(IupSubmenu(_("Locale"), locale_menu), NULL);

        /* clang-format off */
        Ihandle *menu = IupMenu(
            IupSubmenu(_("File"), file_menu),
            IupSubmenu(_("Edit"), edit_menu),
            NULL
        );
        /* clang-format on */

        Ihandle *vbox = IupVbox(multitext, NULL);
        IupSetAttribute(vbox, "MINSIZE", "640x480");

        dlg = IupDialog(vbox);
        IupSetAttributeHandle(dlg, "MENU", menu);
        IupSetAttribute(dlg, "TITLE", _("Notepad"));

        IupPopup(dlg, IUP_CENTER, IUP_CENTER);
        IupDestroy(dlg);

        IupClose();
    } while (redraw);

    for (struct aa_node *node = NULL; (node = aa_next(a));)
        aa_delete(node->value);
    aa_delete(a);

    assert(_Allocated_memory == 0);

    return EXIT_SUCCESS;
}

#endif /* TEST_AA_IUP */
