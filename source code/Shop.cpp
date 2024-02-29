  //-----------------------------------------------------------------------------------------------------------
  //Specific functions implementation for Shop class
  //-----------------------------------------------------------------------------------------------------------
  
  
  
  #include "Shop.h"
  
  //----------------------------- Constructor: Shop( int nBarbers, int nChairs )  ---------------------------------
  //Initializes a Shop object with nBarbers and nChairs. 
  Shop::Shop( int nBarbers, int nChairs ) 
  {
      this -> nBarbers = nBarbers;
      this -> nChairs = nChairs;
  
      pthread_mutex_init(&mutex1, NULL);
  
      barbers = new Barber[nBarbers];
      for (int i = 0; i < nBarbers; i++)
      {
          barbers[i].id = i;
          pthread_cond_init(&barbers[i].barberCond, NULL);
          pthread_cond_init(&barbers[i].paid_cond, NULL);    
      
      }
  }
  
  //----------------------------- Constructor: Shop( ) ---------------------------------
  //Initializes a Shop object with default number of barbers and chairs, 1 barber and 3 chairs.
  Shop::Shop( ) 
  {
      this -> nBarbers = DEFAULT_BARBERS;
      this -> nChairs = DEFAULT_CHAIRS;
  
      pthread_mutex_init(&mutex1, NULL);
      
  
      barbers = new Barber[nBarbers];
      for (int i = 0; i < nBarbers; i++)
      {
          barbers[i].id = i;
          pthread_cond_init(&barbers[i].barberCond, NULL);
          pthread_cond_init(&barbers[i].paid_cond, NULL);       
          
      }
  }
  
  
  //----------------------------- Destructor: Shop( ) ---------------------------------
  //free space
  Shop::~Shop() 
  {
  
      pthread_mutex_destroy(&mutex1);
      for (int i = 0; i < nBarbers; i++)
      {
          pthread_cond_destroy(&barbers[i].barberCond);
          pthread_cond_destroy(&barbers[i].paid_cond);       
          
      }
  }
  
  
  //----------------------------- visitShop( int customerId ) ---------------------------------
  //Is called by a customer thread to visit the shop.
  int Shop::visitShop( int customerId ) 
  {
      pthread_mutex_lock(&mutex1);
      
      if(nChairs==0){ // if there is no chair
        if(sleepingBarbers.empty()){ // if there is no free barbers, customer leaves
          printf("customer[%i]:   leaves the shop because of 0 chairs and no available barbers. \n", customerId);
          nDropsOff++;          
          pthread_mutex_unlock(&mutex1);
          return -1;
        
        }
      }else{          
          if ((int)waitingCustomers.size() == nChairs) { //no waiting chair free, customer leaves
      
              printf("customer[%i]:   leaves the shop because of no available waiting chairs. \n", customerId);
              nDropsOff++;              
              pthread_mutex_unlock(&mutex1);
              return -1;
          }
        
        }
        
      customers[customerId] = Customer();  // create a customer object
      customers[customerId].id = customerId;
      pthread_cond_init(&customers[customerId].customerCond, NULL);
      int barberId;
  
      if (sleepingBarbers.empty()) {
          waitingCustomers.push(customerId); //push current customer into waitingCustomers queue
  
          printf("customer[%i]:   takes a waiting chair. # waiting seats available = %i \n", 
                  customerId, (int)(nChairs-waitingCustomers.size()));
          
          //while all the customers in waitingCustomers queue will wake up by a free barber, but only one assigned this barber can
          //get out the loop, the others will wait and sleep again 
          while (customers[customerId].myBarber == -1) //
          {
              pthread_cond_wait(&customers[customerId].customerCond, &mutex1); //wait for a signal from a barber who just became free 
          }
          barberId = customers[customerId].myBarber;
      }
      else {    //there are sleeping barbers in a queue
          barberId = sleepingBarbers.front(); 
          sleepingBarbers.pop();
          customers[customerId].myBarber = barberId;
          getBarber(barberId) -> myCustomer = customerId;
      }
  
      printf("customer[%i]:   moves to a service chair[%i], # waiting seats available = %i \n", 
              customerId, barberId, (int)(nChairs-waitingCustomers.size()));
  
      customers[customerId].state = CHAIR;
      
      //send  gBarbers queue if there is and to the ready barber of function helloCustomer()
      pthread_cond_signal(&(getBarber(barberId) -> barberCond)); 
      
      pthread_mutex_unlock(&mutex1);
      return barberId;
  }
  
  //----------------------------- leaveShop( int customerId, int barberId ) ---------------------------------
  //Is called by a customer thread to leave the shop.
  void Shop::leaveShop( int customerId, int barberId )
  {
      pthread_mutex_lock(&mutex1);
//////////////////////////////////////////////////////////delete
  
      pthread_mutex_unlock(&mutex1);
  }
  
  
  //----------------------------- helloCustomer( int barberId ) ---------------------------------
  //Is called by a barber thread.
  void Shop::helloCustomer( int barberId )
  {
      pthread_mutex_lock(&mutex1);
  
      if (getBarber(barberId) -> myCustomer == -1)  {
          printf("barber  [%i]:   sleeps because of no customers.\n", barberId);
          sleepingBarbers.push(barberId);
          while (getBarber(barberId) -> myCustomer == -1)
          {
              // waiting for the signal after customer's state changing into CHAIR in function visitShop()
              pthread_cond_wait(&(getBarber(barberId) -> barberCond), &mutex1); 
          }
      }
  

      while (customers[getBarber(barberId) -> myCustomer].state != CHAIR) // synchronization with customer thread
      {
          //waiting for the signal after customer's state changing into CHAIR in function visitShop()
          pthread_cond_wait(&(getBarber(barberId) -> barberCond), &mutex1); 
      }

      printf("barber  [%i]:   starts a hair-cut service for customer[%i]\n", barberId, getBarber(barberId) -> myCustomer);
  
      pthread_mutex_unlock(&mutex1);
  }
  
  
  //----------------------------- byeCustomer( int barberId ) ---------------------------------
  //Is called by a barber thread. 
  void Shop::byeCustomer( int barberId )
  {
      pthread_mutex_lock(&mutex1);
      
      printf("barber  [%i]:   says he's done with a hair-cut service for customer[%i]\n", barberId, getBarber(barberId) -> myCustomer);
      customers[getBarber(barberId) -> myCustomer].myBarber = -1;
      pthread_cond_signal(&customers[getBarber(barberId) -> myCustomer].customerCond); // send the haircut finishing signal
      
      // waiting to be aiting to be paid by customer
      printf("barber  [%i]:   waiting to be paid by customer[%i]\n", barberId, getBarber(barberId) -> myCustomer);
      pthread_cond_wait(&(getBarber(barberId) -> paid_cond), &mutex1);
      printf("barber  [%i]:   paid by customer[%i]\n", barberId, getBarber(barberId) -> myCustomer);    
      
      getBarber(barberId) -> myCustomer = -1;
      printf("barber  [%i]:   calls in another customer.\n", barberId);
      if (!waitingCustomers.empty())  // if waitingCustomers que isn't empty, serve another customer in the waitingCustomers queue
      {
          int customerId = waitingCustomers.front(); //get the customerId who has been waiting for longest time
          waitingCustomers.pop();
          getBarber(barberId) -> myCustomer = customerId;
          customers[customerId].myBarber = barberId; //assign this customer's barber as current barber
          pthread_cond_signal(&customers[customerId].customerCond); //send signal to the right customers in the waitingCustomers queue
      }
  
      pthread_mutex_unlock(&mutex1);
  }
  
  //----------------------------- getBarber(int barberId) ---------------------------------
  //To get private field barber.
  Shop::Barber* Shop::getBarber(int barberId)
  {
      for (int i = 0; i< nBarbers; i++)
      {
          if (barbers[i].id == barberId)
          {
              return &barbers[i];
          }
      }
      return NULL;
  }



