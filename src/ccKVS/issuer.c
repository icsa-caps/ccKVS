#include "util.h"
#include "issuer.h"
#include <sys/time.h>
#include <gsl/gsl_randist.h>
//#include <time.h>
#include <sys/sysinfo.h>

#define MILLION 1000000
#define BILLION 1000000000


static inline u_int64_t getMicrotime(void){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * MILLION + currentTime.tv_usec;
}

static inline u_int64_t getNanotime_deprecated(void){
	struct timespec currentTime;
	gettimeofday((struct timeval*) &currentTime, NULL);
	return currentTime.tv_sec * BILLION + currentTime.tv_nsec;
}

static inline u_int64_t getNanotime(void){
	/* get monotonic clock time */
	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);
	return monotime.tv_sec * BILLION + monotime.tv_nsec;
}

static inline u_int64_t ceil_to_second(u_int64_t timestamp){
	return timestamp / 1000000;
}

int bench() {
	struct timespec tv_start, tv_end;
	struct timespec tv_tmp;
	int i = 0, count = 1 * 1000 * 1000 * 50;
	clockid_t clockid;
	int rv = clock_getcpuclockid(0, &clockid);

	if (rv) {
		perror("clock_getcpuclockid");
		return 1;
	}

	clock_gettime(clockid, &tv_start);
	for (i = 0; i < count; i++)
		clock_gettime(CLOCK_MONOTONIC, &tv_tmp);
		//gettimeofday(&tv_tmp, NULL);
	clock_gettime(clockid, &tv_end);

	long long diff = (long long)(tv_end.tv_sec - tv_start.tv_sec)*(1*1000*1000*1000);
	diff += (tv_end.tv_nsec - tv_start.tv_nsec);

	//printf("getting the clock avg overhead = %f ns/cycle\n", count, diff, (double)diff / (double)count);
    printf("getting the clock avg overhead = %f ns/cycle\n", (double)diff / (double)count);
	return 0;
}

void *run_issuer(void *arg) {
    bench();
	struct issuer_params params = *(struct issuer_params *) arg;
	int issuer_id = params.id;
	int trace_iter = 0;
	struct trace_command *trace;
	trace_init(&trace, issuer_id);
	struct cache_op op;

	/* create a generator chosen by the environment variable GSL_RNG_TYPE */
	gsl_rng_env_setup();
	const gsl_rng_type * T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc(T);
	u_int64_t next_req_dispatch = 0;
	int messages_per_sec = 0, max_messages_per_sec = 0;
	u_int64_t xput_timestamp = 0, prev_prev_timestamp = 0, prev_timestamp = 0, timestamp = 0, error = 0, max_error = 0;
	int loopcounter = 0, high_error_counter = 0, low_error_counter = 0;
    next_req_dispatch = (u_int64_t) gsl_ran_poisson(r, POISSON_AVG_REQ_ARRIVAL);
	//timestamp = getMicrotime();
	timestamp = getNanotime();
    prev_timestamp = timestamp;
	prev_prev_timestamp = timestamp;
	xput_timestamp = timestamp;
    while(1){
		timestamp = getNanotime();
		//timestamp = getMicrotime();
		loopcounter++;
		if(loopcounter % (10 * MILLION) == 0){
			/*printf("Max msg/sec: %llu\n" "xPut_ts: %llu\n"
						   "Prev_ts: %llu\n" "Curr_ts: %llu\n"
						   "Next_REQ: %llu\n" "Max error: %llu\n"
						   "error: %llu\n" "\n\n\n", max_messages_per_sec,
				   xput_timestamp, prev_timestamp, timestamp,
				   next_req_dispatch, max_error, error );
				   */
			printf("Total xput: %d\n",max_messages_per_sec);
			printf("High error: %d\n" "Low error: %d\n", high_error_counter, low_error_counter);
			//printf("Dispatch error: %2.f % (higher 1us)\n",100.0 * high_error_counter/((low_error_counter + high_error_counter)));
			max_error = 0;
			low_error_counter = 0;
			high_error_counter = 0;
			max_messages_per_sec = 0;
		}

		if(prev_timestamp + next_req_dispatch <= timestamp){
            assert(prev_timestamp + next_req_dispatch <= timestamp);
			///Send req
			error = timestamp - (prev_timestamp + next_req_dispatch);
			if(error > 1000)
                high_error_counter ++;
			else
				low_error_counter ++;
				//printf("error: %llu \n", error);
            if(max_error < error){
				max_error = error;
				//if(error > 2000)
				//	printf("error: %llu \n", error);
                /*if(error > 1000){
					printf("Prev Prev_ts %llu\n" "Prev_ts: %llu\n" "Next_REQ: %llu\n" "SUM: %llu\n"
                           "Curr_ts: %llu\n", prev_prev_timestamp, prev_timestamp, next_req_dispatch,
						   prev_timestamp + next_req_dispatch, timestamp);
				}*/
			}
			prev_prev_timestamp = prev_timestamp;
			prev_timestamp = timestamp;//prev_timestamp + next_req_dispatch;// or = timestamp

            //xPut stats
			if(timestamp - xput_timestamp < BILLION)
				messages_per_sec++;
			else{
				xput_timestamp = timestamp;
				if(max_messages_per_sec < messages_per_sec)
					max_messages_per_sec = messages_per_sec;
				messages_per_sec = 1;
			}

            //Oportunistically find next dispatch time & create the next req
			create_req_from_trace(&trace_iter, issuer_id, trace, &op);
			next_req_dispatch = (u_int64_t) gsl_ran_poisson(r, POISSON_AVG_REQ_ARRIVAL);
		}
	}
	gsl_rng_free(r);
	return NULL;
}
