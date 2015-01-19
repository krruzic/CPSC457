#define NUM_THREADS 10
#define NUM_POTS 3


/*  definition of a customer, contains id (order generated), cost of the customer's order,
    type (simple or complex) and a pointer to the next guy (or girl ;) ) in the line */
typedef struct _customer {
    int id;
    int cost;
    int type;
} customer;

void *simple_func(void *sharedC);
void *complex_func(void *sharedC);
void serve(customer *q);
void pay_func(customer *q);
void add_customer_time(customer *curr_customer);

// declare locks
pthread_mutex_t complex_lock;
pthread_mutex_t simple_lock;
pthread_mutex_t pay_lock;
pthread_mutex_t free_pots_lock;
pthread_mutex_t pots[NUM_POTS];

// start off with all pots free
int free_pots = NUM_POTS;

// money made
int money_earned;

// global time variables
struct timeval end_times[NUM_THREADS];
struct timeval start_times[NUM_THREADS];
long int simple_total = 0;
long int complex_total = 0;