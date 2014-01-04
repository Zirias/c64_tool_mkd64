#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY 0x100
#endif
#else
#include <sys/utsname.h>
#endif

const char *unknown = "<UNKNOWN>";

#ifdef WIN32
const wchar_t *regCurrentVersionName =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

const wchar_t *regEnvironmentName =
    L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";

static HKEY regCurrentVersion;
static HKEY regEnvironment;

static int haveRegCurrentVersion = 0;
static int haveRegEnvironment = 0;

static void
openRegKeys(void)
{
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regCurrentVersionName, 0,
                KEY_READ | KEY_WOW64_64KEY, &regCurrentVersion)
            == ERROR_SUCCESS)
    {
        haveRegCurrentVersion = 1;
    }
    else
    {
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regCurrentVersionName, 0,
                    KEY_READ, &regCurrentVersion)
                == ERROR_SUCCESS)
        {
            haveRegCurrentVersion = 1;
        }
    }
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regEnvironmentName, 0,
                KEY_READ | KEY_WOW64_64KEY, &regEnvironment)
            == ERROR_SUCCESS)
    {
        haveRegEnvironment = 1;
    }
    else
    {
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regEnvironmentName, 0,
                    KEY_READ, &regEnvironment)
                == ERROR_SUCCESS)
        {
            haveRegEnvironment = 1;
        }
    }
}

static void
closeRegKeys(void)
{
    if (haveRegCurrentVersion)
    {
        RegCloseKey(regCurrentVersion);
        haveRegCurrentVersion = 0;
    }
    if (haveRegEnvironment)
    {
        RegCloseKey(regEnvironment);
        haveRegEnvironment = 0;
    }
}

static const char *
getRegValue(HKEY key, const wchar_t *name)
{
    static wchar_t buffer[1024];
    static char value[1024];
    DWORD size = sizeof(buffer) - sizeof(wchar_t);

    memset(&buffer, 0, sizeof(buffer));
    if (RegQueryValueExW(key, name, 0, 0, (LPBYTE)buffer, &size)
            == ERROR_SUCCESS)
    {
        memset(&value, 0, sizeof(value));
        wcstombs(value, buffer, sizeof(value) - 1);
        return value;
    }
    return 0;
}

static const char *
getOsVersion(void)
{
    static char buffer[1024];
    const char *csd;

    if (!haveRegCurrentVersion) return unknown;
    memset(&buffer, 0, sizeof(buffer));
    strncpy(buffer, getRegValue(regCurrentVersion, L"ProductName"),
            sizeof(buffer) - 1);
    strncat(buffer, " [", sizeof(buffer) - 1 - strlen(buffer));
    strncat(buffer, getRegValue(regCurrentVersion, L"CurrentVersion"),
            sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, "]", sizeof(buffer) - 1 - strlen(buffer));
    csd = getRegValue(regCurrentVersion, L"CSDVersion");
    if (csd && strlen(csd))
    {
        strncat(buffer, ", ", sizeof(buffer) - 1 - strlen(buffer));
        strncat(buffer, csd, sizeof(buffer) - 1 - strlen(buffer));
    }
    return buffer;
}

static int
onWow64(void)
{
    HANDLE krnl32;
    FARPROC isWow64Process;
    BOOL result;

    krnl32 = GetModuleHandle("kernel32");
    if (!krnl32) return 0;

    isWow64Process = GetProcAddress(krnl32, "IsWow64Process");
    if (!isWow64Process) return 0;

    ((BOOL (WINAPI *)(HANDLE, PBOOL))isWow64Process)(
        GetCurrentProcess(), &result);
    return (result == TRUE) ? 1 : 0;
}

#else

static struct utsname sysname;
static int haveSysname = 0;

static const char *
getOsVersion(void)
{
    static char buffer[1024];

    if (!haveSysname)
    {
        haveSysname = (uname(&sysname) == 0);
    }
    if (!haveSysname) return unknown;

    snprintf(buffer, sizeof(buffer),"%s %s [%s]",
            sysname.sysname, sysname.release, sysname.version);

    return buffer;
}

#endif

static const char *
getHostArchitecture(void)
{
    const char *procArch;

#ifdef WIN32
    if (!haveRegEnvironment) return unknown;
    procArch = getRegValue(regEnvironment, L"PROCESSOR_ARCHITECTURE");
    if (!procArch || strlen(procArch) == 0) return unknown;
#else
    if (haveSysname)
    {
        procArch = sysname.machine;
    }
    else
    {
        procArch = unknown;
    }
#endif

    return procArch;
}

static const char *
getBuildArchitecture(void)
{
    const char *buildArch;

    buildArch = getHostArchitecture();
#ifdef WIN32
    if (buildArch != unknown && strcmp(buildArch, "x86") != 0)
    {
        if (onWow64()) buildArch = "x86";
    }
#endif

    return buildArch;
}

static const char *
getBuildTime(void)
{
    static char buf[128];
    time_t timestamp;
    struct tm *now;

    time(&timestamp);
    now = gmtime(&timestamp);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S [UTC]", now);

    return buf;
}

int
main(int argc, char **argv)
{
#ifdef WIN32
    openRegKeys();
#endif

    setlocale(LC_ALL, "C");

    puts("#ifndef BUILDID_H\n#define BUILDID_H\n");

    printf("#define BUILDID_SYSTEM \"%s\"\n", getOsVersion());
    printf("#define BUILDID_HOSTARCH \"%s\"\n", getHostArchitecture());
    printf("#define BUILDID_BUILDARCH \"%s\"\n", getBuildArchitecture());
    printf("#define BUILDID_TIME \"%s\"\n", getBuildTime());

    puts("#ifdef __GNUC__\n"
         "#define BUILDID_COMPILER \"gcc \" __VERSION__\n"
         "#else\n"
         "#ifdef _MSC_VER\n"
         "#define BUILDID_COMPILER \"msvc \" _MSC_VER\n"
         "#else\n"
         "#define BUILDID_COMPILER \"<UNKNOWN>\"\n"
         "#endif\n"
         "#endif\n\n"
         "#define BUILDID_ALL \"Built using \" BUILDID_COMPILER \"\\n\" \\\n"
         "                    \"on \" BUILDID_SYSTEM \"\\n\" \\\n"
         "                    \"host architecture \" BUILDID_HOSTARCH \""
         ", target architecture \" BUILDID_BUILDARCH \".\\n\" \\\n"
         "                    \"Build time: \" BUILDID_TIME\n\n"
         "#endif");

#ifdef WIN32
    closeRegKeys();
#endif
    return EXIT_SUCCESS;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
