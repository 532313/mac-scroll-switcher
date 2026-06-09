/*
 * scrollswitch - keep trackpad natural scrolling, invert mouse wheel scrolling.
 *
 * Build:
 *     clang -std=c99 -Wall -Wextra -Werror -pedantic -O2 \
 *         -framework ApplicationServices -framework CoreFoundation \
 *         -o scrollswitch src/scrollswitch.c
 */

#include "scrollswitch.h"

static CFMachPortRef event_tap;
static struct app_config config;

static void usage(FILE *stream, const char *name)
{
	fprintf(stream,
		"usage: %s [options]\n"
		"\n"
		"options:\n"
		"  -h, --help              show this help text\n"
		"  -v, --verbose           log handled scroll events\n"
		"      --invert-continuous invert continuous devices too\n"
		"\n"
		"Enable macOS natural scrolling, then run this tool.\n",
		name);
}

static int parse_args(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage(stdout, argv[0]);
			exit(EXIT_SUCCESS);
		}

		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
			config.verbose = true;
			continue;
		}

		if (!strcmp(argv[i], "--invert-continuous")) {
			config.invert_continuous = true;
			continue;
		}

		fprintf(stderr, "unknown option: %s\n", argv[i]);
		usage(stderr, argv[0]);
		return -1;
	}

	return 0;
}

static void set_int_field(CGEventRef event, CGEventField field, int64_t value)
{
	CGEventSetIntegerValueField(event, field, value);
}

static void set_double_field(CGEventRef event, CGEventField field, double value)
{
	CGEventSetDoubleValueField(event, field, value);
}

static void invert_axis(CGEventRef event,
			CGEventField delta_field,
			CGEventField point_field,
			CGEventField fixed_field)
{
	int64_t delta;
	int64_t point_delta;
	double fixed_delta;

	delta = CGEventGetIntegerValueField(event, delta_field);
	point_delta = CGEventGetIntegerValueField(event, point_field);
	fixed_delta = CGEventGetDoubleValueField(event, fixed_field);

	set_int_field(event, delta_field, -delta);
	set_int_field(event, point_field, -point_delta);
	set_double_field(event, fixed_field, -fixed_delta);
}

static void invert_scroll(CGEventRef event)
{
	invert_axis(event,
		    kCGScrollWheelEventDeltaAxis1,
		    kCGScrollWheelEventPointDeltaAxis1,
		    kCGScrollWheelEventFixedPtDeltaAxis1);

	invert_axis(event,
		    kCGScrollWheelEventDeltaAxis2,
		    kCGScrollWheelEventPointDeltaAxis2,
		    kCGScrollWheelEventFixedPtDeltaAxis2);

	invert_axis(event,
		    kCGScrollWheelEventDeltaAxis3,
		    kCGScrollWheelEventPointDeltaAxis3,
		    kCGScrollWheelEventFixedPtDeltaAxis3);
}

static CGEventRef tap_callback(CGEventTapProxy proxy,
			       CGEventType type,
			       CGEventRef event,
			       void *user_info)
{
	int64_t continuous;

	(void)proxy;
	(void)user_info;

	if (type == kCGEventTapDisabledByTimeout ||
	    type == kCGEventTapDisabledByUserInput) {
		CGEventTapEnable(event_tap, true);
		return event;
	}

	if (type != kCGEventScrollWheel)
		return event;

	continuous = CGEventGetIntegerValueField(event,
						 kCGScrollWheelEventIsContinuous);
	if (continuous && !config.invert_continuous)
		return event;

	invert_scroll(event);

	if (config.verbose) {
		fprintf(stderr,
			"scrollswitch: inverted %s scroll event\n",
			continuous ? "continuous" : "discrete");
	}

	return event;
}

bool accessibility_trusted(void)
{
	const void *keys[] = { kAXTrustedCheckOptionPrompt };
	const void *values[] = { kCFBooleanTrue };
	CFDictionaryRef options;
	Boolean trusted;

	options = CFDictionaryCreate(NULL,
				     keys,
				     values,
				     1,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);
	if (!options)
		return false;

	trusted = AXIsProcessTrustedWithOptions(options);
	CFRelease(options);

	return trusted == true;
}

static void stop_run_loop(int signo)
{
	(void)signo;
	CFRunLoopStop(CFRunLoopGetMain());
}

int install_signal_handlers(void)
{
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	action.sa_handler = stop_run_loop;

	if (sigaction(SIGINT, &action, NULL) < 0)
		return -1;

	if (sigaction(SIGTERM, &action, NULL) < 0)
		return -1;

	return 0;
}

int create_event_tap(void)
{
	CGEventMask mask;
	CFRunLoopSourceRef source;

	mask = CGEventMaskBit(kCGEventScrollWheel);
	event_tap = CGEventTapCreate(kCGSessionEventTap,
				     kCGHeadInsertEventTap,
				     kCGEventTapOptionDefault,
				     mask,
				     tap_callback,
				     NULL);
	if (!event_tap) {
		fprintf(stderr,
			"scrollswitch: failed to create event tap; "
			"check Accessibility permission\n");
		return -1;
	}

	source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, event_tap, 0);
	if (!source) {
		CFRelease(event_tap);
		event_tap = NULL;
		return -1;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	CFRelease(source);

	CGEventTapEnable(event_tap, true);
	return 0;
}

int main(int argc, char **argv)
{
	if (parse_args(argc, argv) < 0)
		return EXIT_FAILURE;

	if (!accessibility_trusted()) {
		fprintf(stderr,
			"scrollswitch: Accessibility permission is required. "
			"Approve the prompt in System Settings, then run again.\n");
	}

	if (install_signal_handlers() < 0) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	if (create_event_tap() < 0)
		return EXIT_FAILURE;

	if (config.verbose)
		fprintf(stderr, "scrollswitch: running\n");

	CFRunLoopRun();

	if (event_tap)
		CFRelease(event_tap);

	return EXIT_SUCCESS;
}
