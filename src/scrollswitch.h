#ifndef SCROLLSWITCH_H
#define SCROLLSWITCH_H

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct app_config {
	bool invert_continuous;
	bool verbose;
};

bool accessibility_trusted(void);
int install_signal_handlers(void);
int create_event_tap(void);

#endif /* SCROLLSWITCH_H */
