//-----------------------------------------------------------------------------------------------------------
//Introduction: Shop class extends the sleeping-barber problem toa multiple sleeping barbers problem where many customers 
//              visit a barbershop and receive a haircut service from any one of the available barbers in the shop.  
//It includes public functions: 
//    --Shop(int num_barbers, int num_chairs): Initializes a Shop object with nBarbers and nChairs.  
//    --Shop(): Initializes a Shop object with default number of barbers and chairs, 1 barber and 3 chairs.
//    --~Shop(): Destructor to free space.
//    --int visitShop(int id): Is called by a customer thread to visit the shop. 
//    --void leaveShop(int customer_id, int barber_id): Is called by a customer thread to leave the shop.
//    --void helloCustomer(int id): Is called by a barber thread.
//    -- byeCustomer(int id): Is called by a barber thread. 
//It includes private functions:
//    -- Barber* getBarber(int barberId): To get private barber.
//-----------------------------------------------------------------------------------------------------------



#ifndef _SHOP_H_
#define _SHOP_H_
#include <pthread.h> 
#include <queue> 
#include <map>
#include <iostream>
using namespace std;

#define DEFAULT_CHAIRS 3  // the default number of chairs for waiting = 3
#define DEFAULT_BARBERS 1 // the default number of barbers = 1

class Shop {
public:
    Shop( int nBarbers, int nChairs ); //initializes a Shop object with nBarbers and nChairs
    Shop( );  // default constructor, to initializes a Shop object with default number of barbers and chairs: 1 barber and 3 
    ~Shop( ); // destructor, to free space occupied by Shop object
    int visitShop( int customerId ); // return a non-negative number only when a customer got a service
    
    void leaveShop( int customerId, int barberId ); 
    void helloCustomer( int barberId );
    
    void byeCustomer( int barberId );

    int nDropsOff = 0; // the number of customers dropped off

private:
    int nBarbers;
    int nChairs;

    enum customerState {WAIT, CHAIR, LEAVING};

    struct Barber {

        int id;
        pthread_cond_t barberCond;// define barber condition when waiting for a customer
        pthread_cond_t paid_cond; // define paid condition, will be wakeup by signal from customer
        int myCustomer = -1;      // no customer by default
    };

    struct Customer {
        int id;
        pthread_cond_t customerCond;// define customer condition when waiting for a barber
        customerState state = WAIT; // waiting state by default
        int myBarber = -1;          // no barber by default
    };


    Barber *barbers;              // array of barber objects
    map<int, Customer> customers; // container for customer objects


    queue<int> waitingCustomers;
    queue<int> sleepingBarbers;

    pthread_mutex_t mutex1;
        


    Barber* getBarber(int barberId); // to get private field barber.
};

#endif