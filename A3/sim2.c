#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "sim2.h"


/* delays for a certain number of seconds, does nothing */
int delay(long seconds)
{
    time_t t1;
    t1 = time(0) + seconds;
    while (time(0) < t1)
        ;
}

/* complex queue thread function */
void *complex_func(void *sharedQ) {
    customer_queue *q = (customer_queue *) sharedQ;
    customer *curr_customer;

    /*  lock the queue, pop a value, serve them and add them to the pay queue
        then calculate the ending time for said customer */
    pthread_mutex_lock(&complex_queue_lock);
    curr_customer = peek_customer(q);
    serve(curr_customer);
    pthread_mutex_unlock(&complex_queue_lock);

    pthread_mutex_lock(&pay_queue_lock);
    insert_customer(pay_queue, curr_customer);
    pay_func(pay_queue);
    pthread_mutex_unlock(&pay_queue_lock);

    gettimeofday(&end_times[curr_customer->id]);
    add_customer_time(curr_customer);
    free(curr_customer);

    pthread_exit(NULL);
}

/* simple queue thread function */
void *simple_func(void *sharedQ) {
    customer_queue *q = (customer_queue *) sharedQ;
    customer *curr_customers[NUM_POTS];
    int i;

    // get a lock to wait on a pot, then wait for it to be freed
    pthread_mutex_lock(&free_pots_lock);
    while (free_pots == 0)
        ;
    free_pots--;
    pthread_mutex_unlock(&free_pots_lock);

    /*  now that we have a pot, figure out which one we can lock
        after acquiring a lock, the thread is served and removed from the queue */
    for (i = 0; i < NUM_POTS; i++) {
        if (pthread_mutex_trylock(&pots[i]) == 0) {
            pthread_mutex_lock(&simple_queue_lock);
            curr_customers[i] = peek_customer(q);
            pthread_mutex_unlock(&simple_queue_lock);
            serve(curr_customers[i]);

            pthread_mutex_lock(&pay_queue_lock);
            insert_customer(pay_queue, curr_customers[i]);
            pay_func(pay_queue);
            pthread_mutex_unlock(&pay_queue_lock);

            // get time of the day for the current thread
            gettimeofday(&end_times[curr_customers[i]->id]);

            pthread_mutex_lock(&time_lock);
            add_customer_time(curr_customers[i]);

            pthread_mutex_unlock(&time_lock);

            // done with customer, free memory
            free(curr_customers[i]);

            // free a pot
            free_pots++;
            pthread_mutex_unlock(&pots[i]);
            pthread_exit(NULL);
        }
    }
}

void add_customer_time(customer *curr_customer) {
    // pthread_mutex_lock(&time_lock);
    if (curr_customer->type)
        simple_total += (end_times[curr_customer->id].tv_sec - start_times[curr_customer->id].tv_sec);
    else
        complex_total += (end_times[curr_customer->id].tv_sec - start_times[curr_customer->id].tv_sec);

    // pthread_mutex_unlock(&time_lock);
}

/* pay the barista then leave the pay queue */
void pay_func(customer_queue *q) {
    customer *curr_customer;
    if (curr_customer = peek_customer(q))
        money_earned = money_earned + curr_customer->cost;
}

int main(int argc, char **argv) {
    int i, rc;
    int simple_threads = 0;
    int complex_threads = 0;

    long int totaltime = 0;

    pthread_t thr[NUM_THREADS];
    customer_queue *simple_queue, *complex_queue;

    /* create a temporary customer */
    customer *temp;

    /* timers */
    struct timeval start, end;

    /* create queues */
    if ((simple_queue = malloc(sizeof(customer_queue))) == NULL)
        return EXIT_FAILURE;
    if ((complex_queue = malloc(sizeof(customer_queue))) == NULL)
        return EXIT_FAILURE;
    if ((pay_queue = malloc(sizeof(customer_queue))) == NULL)
        return EXIT_FAILURE;

    /* init locks */
    pthread_mutex_init(&simple_queue_lock, NULL);
    pthread_mutex_init(&complex_queue_lock, NULL);
    pthread_mutex_init(&pay_queue_lock, NULL);
    pthread_mutex_init(&free_pots_lock, NULL);
    pthread_mutex_init(&time_lock, NULL);

    for (i = 0; i < NUM_POTS; i++)
        pthread_mutex_init(&pots[i], NULL);

    srand(time(NULL)); // seed random number generator
    gettimeofday(&start, NULL);

    for (i = 0; i < NUM_THREADS; ++i) {

        /* allocate space for element in queue */
        if ((temp = malloc(sizeof(customer))) == NULL)
            return EXIT_FAILURE;

        temp->id = i;
        temp->cost = rand() % 31;
        temp->type = temp->cost % 2;

        /*  customers with type 1 are simple orders
            type 0 are complex
        */
        if (temp->type) {
            simple_threads++;
            insert_customer(simple_queue, temp);
            gettimeofday(&start_times[i], NULL);
            if (rc = pthread_create(&thr[i], NULL, simple_func, (void *)simple_queue)) {
                printf("FAO:\n");
                fprintf(stderr, "error: pthread_create, i: %d rc: %d\n", i, rc);
                return EXIT_FAILURE;
            }
        } else {
            complex_threads++;
            insert_customer(complex_queue, temp);
            gettimeofday(&start_times[i], NULL);
            if (rc = pthread_create(&thr[i], NULL, complex_func, (void *)complex_queue)) {
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
    totaltime += (end.tv_sec - start.tv_sec);
    printf("SIMULATION 2 RESULTS WITH %d threads\n", NUM_THREADS);
    printf("Complex avg turnaround time: %.01f seconds %d customers\n", (float)complex_total / (float)complex_threads, complex_threads);
    printf("Simple avg turnaround time: %.01f seconds %d customers\n", (float)simple_total / (float)simple_threads, simple_threads);
    printf("Total time: %lu seconds\n", totaltime);
    printf("Money earned: $%d\n", money_earned);
    return EXIT_SUCCESS;
}

/* inserts a customer into the given queue, and updates pointers to first/last */
int insert_customer(customer_queue *q, customer *customer) {
    if(q->first == NULL) {
        q->first = q->last = customer; //if list is empty, first and last = customer
    } else {
        q->last->next = customer;
        q->last = customer; // point "last" pointer to the new node
    }
    return 0;
}

int is_empty(customer_queue *q) {
    if (q->first == NULL)
        return 1;
    return 0;
}

/* removes the first customer in a queue and updates pointers */
customer* peek_customer(customer_queue *q) {
    customer *temp, *old;
    old = q->first;
    if (q->first == q->last) { // only customer in queue
        q->first = q->last = NULL;
    } else {
        temp = q->first->next;
        q->first = temp; // moves the first pointer to the next item
    }
    return old; // return success
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
