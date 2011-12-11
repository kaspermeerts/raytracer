#ifndef CG_TIMER
#define CG_TIMER

typedef struct Timer Timer;

Timer *timer_start(const char *activity);
void timer_stop(Timer *timer);
double timer_diff(Timer *timer);
void timer_diff_print(Timer *timer);

#endif
