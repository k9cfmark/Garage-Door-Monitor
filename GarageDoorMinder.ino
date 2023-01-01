// ---------------------------------------------------------------- //
// Adruino Garage Door Minder using the Ultrasoninc Sensor HC-SR04 
// as the touchless method of detecting the door
// Also using a single relay to close the garage door button contacts
// Mark L. Martin
// 
// Released into the public domain in the hope that it will be
// useful or educational.
// Use at your own risk.
// 
// Mark L. Martin
// 1/1/2023  (Happy new year!)
// ---------------------------------------------------------------- //

#define echoPin 3   // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 2   //attach pin D3 Arduino to pin Trig of HC-SR04
#define relayPin 4  // momentary relay to simulate person pressing button
#define debugPin 5  // ground this to have a 30 second deployment instead of a 15 minute timer...

#define PERSISTANCEMAX 10           // I found the sonar could be noisy at times...
#define DOOR_TOO_LONG 900000        // 900,000 milliseconds FOR 900 seconds (15 minute) delpoyment time
#define DOOR_TOO_LONG_DEBUG 60000   // 60 SECOND FOR DEBUGGING                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      this is in miliseconds

// defines variables
long duration;                     // variable for the duration of sound wave travel
int distance;                      // variable for the distance measurement
int distance_limit = 40;           // set open/closed threshold to 40cm....  if it's less than 40, the door is open....  
                                   // remember we are detecting the door near the ceiling.  So close show's the roll up door is open.
boolean door_open = false;
boolean closed_door = true;
boolean door_just_opened = false;
boolean door_counter_running = false;
boolean dont_chatter_the_door = false;
unsigned long int door_open_counter = 0l;
unsigned long int door_open_timestamp;
unsigned long int door_open_duration;
unsigned long int door_too_long = 0l;
unsigned long int last_millisecond = 0l;
unsigned long int this_millisecond = 0l;

boolean debug = false;
  
int persistance[PERSISTANCEMAX];
int counter = 0;
int fill_counter = 0;

// start with the door closed....
void clear_persistance()
{
  int i;
  for ( i = 0; i < PERSISTANCEMAX; i++ )
     persistance[i] = distance_limit * 2;
}

void setup() 
{
  pinMode( LED_BUILTIN, OUTPUT );
  pinMode( trigPin, OUTPUT ); // Sets the trigPin as an OUTPUT
  pinMode( echoPin, INPUT ); // Sets the echoPin as an INPUT
  pinMode( relayPin, OUTPUT ); // Sets the relayPin as output
  pinMode( debugPin, INPUT ); // debug pin
  
 
  Serial.begin(9600); // Serial Communication is starting with 9600 of baudrate speed
  Serial.println( "Garage Door Minder Version 1.0  12/31/2022" );
  Serial.println( "Using Ultrasonic Sensor HC-SR04 Test with Arduino Pro Mini" );
  clear_persistance();
  digitalWrite( LED_BUILTIN, LOW );
  digitalWrite( relayPin, LOW );

  this_millisecond = millis();
  last_millisecond = this_millisecond;
}  //end setup()


void close_the_door( void )
{
      Serial.println("*** Closing Door++++");
      digitalWrite( relayPin, HIGH );  //press the close button
      digitalWrite( LED_BUILTIN, HIGH );
      clear_persistance();
      delay(500);  // add a half second to signal door.
      digitalWrite( LED_BUILTIN, LOW );
      digitalWrite( relayPin, LOW );   // unpress the clsoe button
 
} // end close_the_door()

boolean is_door_open( void )  // simple function to return the Logical AND of all the door open status
{
  boolean all_open = true;
  boolean all_closed = true;
  for ( counter = 0; counter < PERSISTANCEMAX; counter++ )
  {
     all_open = ( all_open && ( persistance[counter] < distance_limit ) );
     all_closed = ( all_closed && ( persistance[counter] < distance_limit ) );
  }
  return ( all_open );
} // end boolean is_door_open()

void read_distance_w_persistance( void )
{
  // Clears the trigPin condition 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)  this is spooky accurate
  // Displays the distance on the Serial Monitor
  Serial.print( "Distance: " );
  Serial.print( distance );
  Serial.print( " cm" );
  if ( distance > distance_limit )
    Serial.print( " Closed" );
  else
    Serial.print( " Open" );

  debug = ( digitalRead( debugPin ) == LOW );
  if ( debug ) 
    Serial.println( " debug ");
  else 
    Serial.println( " normal ");
   
  persistance[fill_counter] = distance;
  fill_counter++;
  if ( fill_counter > PERSISTANCEMAX )
    fill_counter = 0;
}  // end void read_distance_w_persistance()


void loop()   // main function  
{
 read_distance_w_persistance();   // read the ultra sound distance;
 door_open = is_door_open();      // determine if the door is open. 
 if ( door_open )
 {
    if ( !door_just_opened )   // did it just open? rising edge detected
    {
      door_just_opened = true; 
      door_counter_running = true;
      door_open_counter = 0l;
      door_open_timestamp = millis();
      dont_chatter_the_door = false;
    }
    else
    {                         // continuing in the door open state
      door_open_counter++;    // count forward
      this_millisecond = millis();  // calculate how long the door is open this loop;
      if ( this_millisecond < last_millisecond )  // the millis() function rolls over every 50 something days.  Door could get stuck open forever.
      {                                      // so if the last millisecond was really high and this one really low, the roll over occured.
        door_too_long = 1;  //  force door closed n case of roll over 
      }
      else
      {
        door_open_duration = this_millisecond - door_open_timestamp; // calculate how long the door is open this loop;
        if ( debug )
           door_too_long = DOOR_TOO_LONG_DEBUG;
        else
           door_too_long = DOOR_TOO_LONG;
      }   
      
      if ( door_open_duration > door_too_long )  // is the door open and time to close it?
      {
         if ( ! dont_chatter_the_door ) // closing the door will take some time for it to close and be readable
                                        // so close only once 
         {
           close_the_door( ); // close the door.  Probably have to do state clean ups...but the door will take some time.
           dont_chatter_the_door = true;
         } // end if door don't chatter is troo
       }  // end if door was open too long
    }  // end if door just opened 
    last_millisecond = this_millisecond;  // needed to capture the roll over
 }  // end if door is open
 else
 {
      // door is closed  
    door_just_opened = false;
    door_counter_running = false;
    door_open_counter = 0l;
    dont_chatter_the_door = false;
 }  // end if door is closed
 
 delay( 500 );
}  // end main loop
