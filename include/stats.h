#ifndef STATS_H
#define STATS_H

/* Structure used by 'get_stats' function */
struct stats
{
	unsigned int tics;
	unsigned int cs; /* Number of times the process has got the CPU: READY->RUN transitions */
    unsigned int remaining_quantum;
};
#endif /* !STATS_H */
