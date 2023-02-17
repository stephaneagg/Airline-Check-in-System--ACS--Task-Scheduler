# Airline-Check-in-System--ACS--Task-Scheduler
ACS is a multi-threaded task scheduler written in C using the pthread library.

This project was an assignment for CSC 360 Operating Systems intended to learn multi-threading in C

General flow of ACS.c
  5 "clerk" threads are initialized with the task of waiting until there is a customer to serve. When this happens the clerk will signal a customer and send it its clerk_id, then wait for the customer to finish being served.
  i "customer" threads are initialized with the task of entering a queue and waiting for its arrival-time, then for a clerk thread to signal it. When a customer is signaled by a clerk, it will record the clerk id, its wait-time and then sleeps for its service time. After that it will exit the queue and signal its clerk to serve another customer


Input File Format
The input file is a text file and has a simple format. The first line contains the total number of customers that will
be simulated. After that, each line contains the information about a single customer, such that:
    1. The first character specifies the unique ID of customers.
    2. A colon(:) immediately follows the unique number of the customer.
    3. Immediately following is an integer equal to either 1 (indicating the customer belongs to business class) or 0 
        (indicating the customer belongs to economy class).
    4. A comma(,) immediately follows the previous number.
    5. Immediately following is an integer that indicates the arrival time of the customer.
    6. A comma(,) immediately follows the previous number.
    7. Immediately following is an integer that indicates the service time of the customer.
    8. A newline (\n) ends a line.

Sample Input
    8
    52 1:0,2,60
    53 2:0,4,70
    54 3:0,5,50
    55 4:1,7,30
    56 5:1,7,40
    57 6:1,8,50
    58 7:0,10,30
    59 8:0,11,50

Sample Output
    A customer arrives: customer ID  1. 
    A customer enters a queue: the queue ID  0, and length of the queue  1 
    A clerk starts serving a customer: start time 0.20, the customer ID  1, the clerk ID 1. 
    A customer arrives: customer ID  2. 
    A customer enters a queue: the queue ID  0, and length of the queue  1 
    A clerk starts serving a customer: start time 0.40, the customer ID  2, the clerk ID 2. 
    A customer arrives: customer ID  3. 
    A customer enters a queue: the queue ID  0, and length of the queue  1 
    A clerk starts serving a customer: start time 0.50, the customer ID  3, the clerk ID 3. 
    A customer arrives: customer ID  4. 
    A customer enters a queue: the queue ID  1, and length of the queue  1 
    A customer arrives: customer ID  5. 
    A customer enters a queue: the queue ID  1, and length of the queue  2 
    A clerk starts serving a customer: start time 0.70, the customer ID  5, the clerk ID 5. 
    A clerk starts serving a customer: start time 0.70, the customer ID  4, the clerk ID 4. 
    A customer arrives: customer ID  6. 
    A customer enters a queue: the queue ID  1, and length of the queue  1 
    A customer arrives: customer ID  7. 
    A customer enters a queue: the queue ID  0, and length of the queue  1 
    A clerk finishes serving a customer: end time 3.70, the customer ID  4, the clerk ID 4. 
    A clerk starts serving a customer: start time 0.80, the customer ID  6, the clerk ID 4. 
    A clerk finishes serving a customer: end time 4.70, the customer ID  5, the clerk ID 5. 
    A clerk starts serving a customer: start time 1.00, the customer ID  7, the clerk ID 5. 
    A clerk finishes serving a customer: end time 5.50, the customer ID  3, the clerk ID 3. 
    A clerk finishes serving a customer: end time 6.20, the customer ID  1, the clerk ID 1. 
    A clerk finishes serving a customer: end time 7.40, the customer ID  2, the clerk ID 2. 
    A clerk finishes serving a customer: end time 7.70, the customer ID  7, the clerk ID 5. 
    A clerk finishes serving a customer: end time 8.70, the customer ID  6, the clerk ID 4. 
    The average waiting time for all customers in the system is: 0.943276 seconds. 
    The average waiting time for all business-class customers is: 0.967180 seconds. 
    The average waiting time for all economy-class customers is: 0.925347 seconds. 
