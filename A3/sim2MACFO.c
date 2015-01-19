#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "sim2MACFO.h"


/* delays for a certain number of seconds, does nothing */
int delay(long seconds)
{
    time_t t1;
    t1 = time(0) + seconds;
    while (time(0) < t1)
        ;
}

/* complex queue thread function */
void *complex_func(void *sharedC) {
    customer *c = (customer *) sharedC;
    pthread_mutex_lock(&complex_lock);
    serve(c);
    pthread_mutex_unlock(&complex_lock);

    pthread_mutex_lock(&pay_lock);
    pay_func(c);
    pthread_mutex_unlock(&pay_lock);

    gettimeofday(&end_times[c->id]);
    add_customer_time(c);
    free(c); // customer has been served and has payed, free memory

    pthread_exit(NULL);
}

/* simple queue thread function */
void *simple_func(void *sharedC) {
    customer *c = (customer *) sharedC;
    customer *curr_customers[NUM_POTS];
    int i;
    // get a lock to wait on a pot, then wait for it to be freed
    pthread_mutex_lock(&free_pots_lock);
    while (free_pots == 0)
        ;
    free_pots--;
    pthread_mutex_unlock(&free_pots_lock);
    /*  now that we have a pot, figure out which one we can lock
        after acquiring a lock, the thread is served and removed from the queue
    */
    for (i = 0; i < NUM_POTS; i++) {
        if (pthread_mutex_trylock(&pots[i]) == 0) {
            curr_customers[i] = c;
            serve(curr_customers[i]);
            pthread_mutex_lock(&pay_lock);
            pay_func(curr_customers[i]);
            pthread_mutex_unlock(&pay_lock);

            gettimeofday(&end_times[curr_customers[i]->id]);
            add_customer_time(curr_customers[i]);

            free_pots++;
            free(curr_customers[i]); // customer has been served and has payed, free memory
            pthread_mutex_unlock(&pots[i]);
            pthread_exit(NULL);
        }
    }
}

void add_customer_time(customer *curr_customer) {
    if (curr_customer->type) {
        simple_total += (end_times[curr_customer->id].tv_sec - start_times[curr_customer->id].tv_sec);
    } else {
        complex_total += (end_times[curr_customer->id].tv_sec - start_times[curr_customer->id].tv_sec);
    }
}

/* pay the barista */
void pay_func(customer *q) {
    // delay(1);
    money_earned = money_earned + q->cost;
}

int main(int argc, char **argv) {
    int i, rc;
    int simple_threads = 0;
    int complex_threads = 0;
    long int totaltime = 0;
    pthread_t thr[NUM_THREADS];

    /* create a temporary customer */
    customer *temp;

    struct timeval start, end;

    /* init locks */
    pthread_mutex_init(&simple_lock, NULL);
    pthread_mutex_init(&complex_lock, NULL);
    pthread_mutex_init(&pay_lock, NULL);
    pthread_mutex_init(&free_pots_lock, NULL);

    for (i = 0; i < NUM_POTS; i++)
        pthread_mutex_init(&pots[i], NULL);

    srand(time(NULL)); // seed random number generator
    gettimeofday(&start, NULL);

    for (i = 0; i < NUM_THREADS; ++i) {
        /* allocate space for element in queue */
        if ((temp = malloc(sizeof(customer))) == NULL) {
            printf("couldn't get memory!\n");
            return EXIT_FAILURE;
        }
        temp->id = i;
        temp->cost = rand() % 31;
        temp->type = temp->cost % 2;

        /*  customers with type 1 are simple orders
            type 0 are complex
        */
        if (temp->type) {
            simple_threads++;
            gettimeofday(&start_times[i], NULL);
            if (rc = pthread_create(&thr[i], NULL, simple_func, (void *)temp)) {
                printf("FAO:\n");
                fprintf(stderr, "error: pthread_create, i: %d rc: %d\n", i, rc);
                return EXIT_FAILURE;
            }
        } else {
            complex_threads++;
            gettimeofday(&start_times[i], NULL);
            if (rc = pthread_create(&thr[i], NULL, complex_func, (void *)temp)) {
                printf("FAO:\n");
                fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
                return EXIT_FAILURE;
            }
        }
    }
    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(thr[i], NULL);
    }

    gettimeofday(&end, NULL);
    totaltime = (end.tv_sec - start.tv_sec);
    printf("Complex avg turnaround time: %lu seconds\n", complex_total / complex_threads);
    printf("Simple avg turnaround time: %lu seconds\n", simple_total / simple_threads);
    printf("Total time: %lu seconds\n", totaltime);
    printf("Money earned: $%d\n", money_earned);
    return EXIT_SUCCESS;
}

/* "work" function */
void serve(customer *q) {
    if (q->type == 0) {
        delay(2);
    }
    else {
        delay(1);
    }
    // printf("Serving customer #%d order cost: %d type: %d\n", q->id, q->cost, q->type);
}
