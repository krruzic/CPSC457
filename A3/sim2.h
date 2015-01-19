#define NUM_THREADS 10
#define NUM_POTS 3


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

void *simple_func(void *sharedQ);
void *complex_func(void *sharedQ);
int insert_customer(customer_queue *q, customer *data);
customer* peek_customer(customer_queue *q);
void serve(customer *q);
void pay_func(customer_queue *q);
void add_customer_time(customer *curr_customer);

// declare locks
pthread_mutex_t simple_queue_lock;
pthread_mutex_t complex_queue_lock;
pthread_mutex_t pay_queue_lock;
pthread_mutex_t free_pots_lock;
pthread_mutex_t pots[NUM_POTS];
pthread_mutex_t time_lock;

// start off with all pots free
int free_pots = NUM_POTS;

// global pay queue
customer_queue *pay_queue;

// money made
int money_earned;

// global time variables
struct timeval end_times[NUM_THREADS];
struct timeval start_times[NUM_THREADS];
long int simple_total = 0;
long int complex_total = 0;