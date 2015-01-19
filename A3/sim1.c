#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "sim1.h"


/* delays for a certain number of seconds, does nothing */
int delay(long seconds)
{
    time_t t1;
    t1 = time(0) + seconds;
    while (time(0) < t1)
        ;
}

void *barista_func(void *sharedQ) {
    customer_queue *q = (customer_queue *) sharedQ;
    customer *curr_customers[NUM_BARISTAS];
    int i;
    // get a lock to wait on a barista, then wait for it to be freed
    pthread_mutex_lock(&free_baristas_lock);
    while (free_baristas == 0)
        ;
    free_baristas--;
    pthread_mutex_unlock(&free_baristas_lock);

    /*  now that we have a pot, figure out which one we can lock
        after acquiring a lock, the thread is served and removed from the queue
    */
    for (i = 0; i < NUM_BARISTAS; i++) {
        if (pthread_mutex_trylock(&baristas[i]) == 0) {
            pthread_mutex_lock(&order_queue_lock);
            curr_customers[i] = peek_customer(q);
            pthread_mutex_unlock(&order_queue_lock);
            serve(curr_customers[i]);
            pay_func(curr_customers[i]);

            gettimeofday(&end_times[curr_customers[i]->id]);
            add_customer_time(curr_customers[i]);

            // free memory and free pot the customer was at.
            free(curr_customers[i]);
            free_baristas++;
            pthread_mutex_unlock(&baristas[i]);

            pthread_exit(NULL);
        }
    }
}

void add_customer_time(customer *curr_customer) {
        if (curr_customer->type)
        simple_total += (end_times[curr_customer->id].tv_sec - start_times[curr_customer->id].tv_sec);
    else
        complex_total += (end_times[curr_customer->id].tv_sec - start_times[curr_customer->id].tv_sec);
}

/* pay the barista */
void pay_func(customer *curr_customer) {
    // delay(1);
    money_earned = money_earned + curr_customer->cost;
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

    /* create queues */
    if ((order_queue = malloc(sizeof(customer_queue))) == NULL)
        return EXIT_FAILURE;

    /* init locks */
    pthread_mutex_init(&order_queue_lock, NULL);
    pthread_mutex_init(&free_baristas_lock, NULL);
    for (i = 0; i < NUM_BARISTAS; i++)
        pthread_mutex_init(&baristas[i], NULL);

    gettimeofday(&start, NULL);
    srand(time(NULL)); // seed random number generator

    for (i = 0; i < NUM_THREADS; ++i) {
        /* allocate space for element in queue */
        if ((temp = malloc(sizeof(customer))) == NULL)
            return EXIT_FAILURE;

        temp->id = i;
        temp->cost = rand() % 31;
        temp->type = temp->cost % 2;
        /*  customers with type 1 are simple orders
            type 0 are complex  */
        insert_customer(order_queue, temp);
        if (temp->type)
            simple_threads++;
        else
            complex_threads++;

        gettimeofday(&start_times[i], NULL);
        if (rc = pthread_create(&thr[i], NULL, barista_func, (void *)order_queue)) {
            printf("FAO:\n");
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }
    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(thr[i], NULL);
    }

    gettimeofday(&end, NULL);
    totaltime = (end.tv_sec - start.tv_sec);
    printf("SIMULATION 1 RESULTS WITH %d THREADS\n", NUM_THREADS);
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
    old = q->first; // what I want to return AKA current front
    if (q->first == q->last) { // only customer in queue
        // free(q->first);
        q->first = q->last = NULL;
    } else {
        temp = q->first->next; // new first guy
        // free(q->first); // deletes the old first node
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
