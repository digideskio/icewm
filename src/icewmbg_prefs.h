#include "yconfig.h"

XSV(const char *, DesktopBackgroundColor, "rgb:00/20/40")
XSV(const char *, DesktopBackgroundPixmap, 0)
XSV(const char *, DesktopTransparencyColor, 0)
XSV(const char *, DesktopTransparencyPixmap, 0)
XIV(bool, centerBackground, false)
XIV(bool, supportSemitransparency, true)

void addBgImage(const char *name, const char *value);

cfoption icewmbg_prefs[] = {
    OBV("DesktopBackgroundCenter",              &centerBackground,              "Display desktop background centered and not tiled"),
    OBV("SupportSemitransparency",              &supportSemitransparency,       "Support for semitransparent terminals like Eterm or gnome-terminal"),
    OSV("DesktopBackgroundColor",               &DesktopBackgroundColor,        "Desktop background color"),
    OSV("DesktopBackgroundImage",               &DesktopBackgroundPixmap,       "Desktop background image"),
    OSV("DesktopTransparencyColor",             &DesktopTransparencyColor,      "Color to announce for semi-transparent windows"),
    OSV("DesktopTransparencyImage",             &DesktopTransparencyPixmap,     "Image to announce for semi-transparent windows"),
    OKF("DesktopBackgroundImage", addBgImage, ""),
    //{ false, { 0, 0, 0 }, { 0, false }, { 0 } }, &addBgImage },
    OK0()
    //{ cfoption::CF_NONE, 0, { false, { 0, 0, 0 }, { 0, false }, { 0 } }, 0 }
};
