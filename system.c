#include "dbg.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <inttypes.h>
#include "system.h"
#include "options.h"
#include "math.h"
#include <fcntl.h>
#include <sys/wait.h>

int reboot() {
  debug("Rebooting system");
  int i = system("reboot");
  return i < 0 ? i : WEXITSTATUS(i);
}

void machine_type(char *type, size_t len)
{
  if (strlen(options.machine) > 0) {
    strcpy(type, options.machine);
  } else {
    FILE* fp;
    size_t bytes_read = 0;
    char *match;
    char buffer[1024];
    char fmt[64];
#ifdef __OPENWRT__
    fp = fopen ("/tmp/sysinfo/model", "r");
#elif __linux
    fp = fopen ("/etc/os-release", "r");
#endif
    if (fp) {
      bytes_read = fread (buffer, 1, sizeof (buffer), fp);
      fclose (fp);
    }

    if (bytes_read == 0 || bytes_read == sizeof (buffer))
      return;

    buffer[bytes_read] = '\0';
#ifdef __OPENWRT__
    strncpy(type, buffer, strlen(buffer)+1);
#elif __linux
    match = strstr(buffer, "NAME");

    if (match == NULL)
      return;
    snprintf(fmt, sizeof(fmt), "NAME=\"%%%zu[^\t\n]\"", len - 1);
    sscanf(match, fmt, type);
#endif
  }
}

struct SystemInfo system_info() {
  struct sysinfo info;
  struct SystemInfo s = { 0 };
  if ( sysinfo (&info) != -1 ){

    double pf = (double)info.freeram / (double)info.totalram;

    // Not functioning
    float t_ram = floorf(info.totalram * 100) / 100;
    float f_ram = floorf(info.freeram * 100) / 100;
    // Not functioning

    s.uptime = info.uptime;
    s.totalram = t_ram;
    s.freeram = f_ram;
    s.percent_used = pf;
    s.procs = info.procs;
    s.load_1 = info.loads[0] / LINUX_SYSINFO_LOADS_SCALE;
    s.load_5 = info.loads[1] / LINUX_SYSINFO_LOADS_SCALE;
    s.load_15 = info.loads[2] / LINUX_SYSINFO_LOADS_SCALE;
  }
  return s;
}
