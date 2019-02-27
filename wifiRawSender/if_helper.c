#include "if_helper.h"

// Either I'm blind, or libiw API documentation is very hard to find,
// so for now system() calls will have to do for configuring the
// wireless interface.

int if_up(char *name) {
    char buf[128];
    snprintf(buf, sizeof(buf), "ifconfig %s up", name);
    int ret = system(buf);
    if (ret != 0) {
        printf("Could not bring up interface %s\n", name);
    }
    return ret;
}

int if_down(char *name) {
    char buf[128];
    snprintf(buf, sizeof(buf), "ifconfig %s down", name);
    int ret = system(buf);
    if (ret != 0) {
        printf("Could not bring down interface %s\n", name);
    }
    return ret;
}

int if_mtu(char *name, int mtu) {
    char buf[128];
    snprintf(buf, sizeof(buf), "ifconfig %s mtu %d", name, mtu);
    int ret = system(buf);
    if (ret != 0) {
        printf("Could not set MTU for interface %s\n", name);
    }
    return ret;
}

int if_enable_ibss(char *name) {
    char buf[128];
    snprintf(buf, sizeof(buf), "iw %s set type ibss", name);
    int ret = system(buf);
    if (ret != 0) {
        printf("Could not enable IBSS on %s\n", name);
    }
    return ret;
}

int if_join_ibss(char *name, char *essid, int frequency) {
    char buf[128];
    snprintf(buf, sizeof(buf), "iw %s ibss join %s %d", name, essid, frequency);
    int ret = system(buf);
    if (ret != 0) {
        printf("Interface %s failed enabling ESSID \"%s\" on frequency %d \n", name, essid, frequency);
    }
    return ret;
}

int if_promisc(char *name) {
    char buf[128];
    snprintf(buf, sizeof(buf), "ifconfig %s promisc", name);
    int ret = system(buf);
    if (ret != 0) {
        printf("Could not put interface %s into promiscous mode\n", name);
    }
    return ret;
}