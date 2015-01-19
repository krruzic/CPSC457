#define NUM_THREADS 10
#define NUM_BARISTAS 2


/*  definition of a customer, contains id (order generated), cost of the customer's order,
    type (simple or complex) and a pointer to the next guy (or girl ;) ) in the line */
typedef struct _customer {
    int id;
    int cost;
    int type;
    struct _customer *next;
} customer;

/* keeps track of the front and back of the queue */
typedef struct _customer_queue {
    customer *first; // pointer to the first item
    customer *last; // pointer to the last item
} customer_queue;

void *barista_func(void *sharedQ);
int insert_customer(customer_queue *q, customer *data);
customer* peek_customer(customer_queue *q);
void serve(customer *q);
void pay_func(customer *curr_customer);
void add_customer_time(customer *curr_customer);

// declare locks
pthread_mutex_t order_queue_lock;
pthread_mutex_t free_baristas_lock;
pthread_mutex_t baristas[NUM_BARISTAS];
pthread_mutex_t time_lock[NUM_BARISTAS];

// start off with all baristas free
int free_baristas = NUM_BARISTAS;

// global order queue
customer_queue *order_queue;

// money made
int money_earned;

// global time variables
struct timeval end_times[NUM_THREADS];
struct timeval start_times[NUM_THREADS];
long int simple_total = 0;
long int complex_total = 0;