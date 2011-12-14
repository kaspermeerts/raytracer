#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "timer.h"

struct Timer {
	const char *name;
	bool running;
	struct timeval start;
	struct timeval stop;
};

Timer *timer_start(const char *activity)
{
	Timer *ret;

	ret = malloc(sizeof(Timer));
	ret->name = activity;
	ret->running = true;
	gettimeofday(&ret->start, NULL);

	return ret;
}

void timer_stop(Timer *timer)
{
	timer->running = false;
	gettimeofday(&timer->stop, NULL);
}

double timer_diff(Timer *timer)
{
	struct timeval begin;
	struct timeval end;

	begin = timer->start;

	if (timer->running)
		gettimeofday(&end, NULL);
	else
		end = timer->stop;

	return (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1.0e6;
}

void timer_diff_print(Timer *timer)
{
	double diff = timer_diff(timer);
	int sec, msec;

	sec = diff;
	msec = (diff - sec) * 1000;

	printf("%s took %d s %03d msec\n", timer->name, sec, msec);
}
