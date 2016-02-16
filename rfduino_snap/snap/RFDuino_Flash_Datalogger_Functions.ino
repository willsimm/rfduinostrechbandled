/*
This sketch fills the RFDuino flash memory with unsigned longs.

This code can be used for a data logger and will store 256 * avialble flash pages values.

an array in memory is filled with long ints unitl it reaches 256 values
this is then saved out to the last available flash page
memory array filled again and then saved out

it does this until all flash pages are filled

when all flash pages are filled with values it will loop around and overwrite the original values.

use readOutWholeFlash() to read out the values that have been saved to flash. Follow that with readOutRAMValues() to get the ram buffer array contents.

if you use findMinFlashPage it will erase all flash pages until it finds the minimum usable one and sets the value. 
Otherwise the value of the minFlashPage variable can be set
The last flash page available will change with different sketches

Will Simm 2015 will@wasdesign.net

*/



// double level of indirection required to get gcc
// to apply the stringizing operator correctly
//needed?
#define  str(x)   xstr(x)
#define  xstr(x)  #x

int maxFlashPage = 251;
int minFlashPage = 154; //148 found using findMinFlashPage()
int currentFlashPage = maxFlashPage; //strat from the last flash page
int loopedRoundFlash = false;
int timeInRAMCount = 0;


//our data structure, not sure if the array actually needs to be in a struct or not, probably not
struct data_t
{
  unsigned long time[256];
};

//Ram buffer array
struct data_t usage;

//find the first available flash page not taken up by the program
//WORKS BY ERASING FLASH AND CHECKING FOR SUCCESS!
int findMinFlashPage(int maxFlashPage) {
  int minPage = 251;
  int rc;
  for (int page = maxFlashPage; page > 120; page--) { 
    data_t *p = (data_t*)ADDRESS_OF_PAGE(page);

    if (radioState){
      while(!RFduinoBLE.radioActive){} //wait until the radio is active, wastes time, but ensures we will get the most usage of non active cpu time
      delay(6); // roughly the amount of time, from when the radio notification triggers to when we are inactive
    }
    rc = flashPageErase(PAGE_FROM_ADDRESS(p));
    if (rc == 0) {
      minPage = page;
      //Serial.print(page);
    }
  }
  return minPage;
}

//save usage timestamp to ram, and out to flash if ram is filled
int saveUsage(unsigned long now){
  //Serial.println("saving usage: ");
  if (timeInRAMCount < 256){
    
      Serial.print(now);
      Serial.print(" saving to ram index: ");
      Serial.println(timeInRAMCount);
      
      usage.time[timeInRAMCount] = now;
      timeInRAMCount++;
      return 1;
  }
  else {
    
   //save out to flash
   Serial.println("filled ram, so saving page to flash ");
   saveToFlash(usage);
   timeInRAMCount = 0; 
   usage.time[timeInRAMCount] = now;
   
   Serial.print(now);
   Serial.print(" saved to ram index: ");
   Serial.println(timeInRAMCount);
   
   timeInRAMCount++;
   return 2;
  }  
}



//save array of longs to next avaliable flash page, if full then overwrite the first
int saveToFlash(struct data_t toSave){
  Serial.println("saving to flash ");

  //if we've reached the lowest flash page, go back to the top but set the flag so we know when reading it back out
  if (currentFlashPage < minFlashPage ){
    Serial.println("reached min flash page so looping ");
    currentFlashPage = maxFlashPage;
    loopedRoundFlash = true;
  }

  int rc;
  data_t *p = (data_t*)ADDRESS_OF_PAGE(currentFlashPage);
  
  //we need to erase flash page first
  Serial.print("Attempting to erase flash page : ");
  if (radioState) {
    while(!RFduinoBLE.radioActive){} //wait until the radio is active, wastes time, but ensures we will get the most usage of non active cpu time
    delay(6); // roughly the amount of time, from when the radio notification triggers to when we are inactive
  }
  rc = flashPageErase(PAGE_FROM_ADDRESS(p));
  if (rc == 0)
    Serial.println("Success");
  else if (rc == 1)
    Serial.println("Error - the flash page is reserved");
  else if (rc == 2)
    Serial.println("Error - the flash page is used by the sketch");    
    
  
  //save it
  Serial.print("Attempting to write data to flash page : ");
  if (radioState) {
    while(!RFduinoBLE.radioActive){} //wait until the radio is active, wastes time, but ensures we will get the most usage of non active cpu time
    delay(6); // roughly the amount of time, from when the radio notification triggers to when we are inactive
  }
  rc = flashWriteBlock(p, &toSave, sizeof(toSave));
  if (rc == 0)
    Serial.println("Success");
  else if (rc == 1)
    Serial.println("Error - the flash page is reserved");
  else if (rc == 2)
    Serial.println("Error - the flash page is used by the sketch"); 
    
  currentFlashPage--;  
  return 1;
}
/*
//read a single flash page into memory
int readOutFlashPage(int page){
  data_t *p = (data_t*)ADDRESS_OF_PAGE(page);
  pdata = p->time;
}*/


// read out the flash memory, starting from max down to current page. If we've looped ouroudn the whole flash memeory then read out current down to min first.
int readOutWholeFlash(){
  bool readAnything = false;
  //Serial.println("readout method");
  //unsigned long readOut[255];
  //iff we've looped round, then start at current page 
  if (loopedRoundFlash){
    Serial.println("looped round");
    for (int page = currentFlashPage; page >= minFlashPage; page--) { 
      
      data_t *p = (data_t*)ADDRESS_OF_PAGE(page);
      //readOut = p->time;
      
      
      //readOutFlashPage(page, &readOut);
      //send over BLE
      sendHistory(page, p->time);      
    }
  }
  Serial.println("not looped");
  //count down to flash pages
  //Serial.print("max f");
  //Serial.println(maxFlashPage);
  //Serial.print("current flash");
  //Serial.println(currentFlashPage);
  //fudge it for testing
  //currentFlashPage = 148;
  //maxFlashPage=149;
  
  for (int page = maxFlashPage; page > currentFlashPage; page--) { 
    //Serial.print("reading page");
    //Serial.println(page);
    data_t *p = (data_t*)ADDRESS_OF_PAGE(page);
    //readOut = p->time;
   
    //readOutFlashPage(page, &readOut);
    //send over BLE
    //Serial.print("contents of p->time[2] should be 2000 :");
    //Serial.println(p->time[2]);
 
    //Serial.println("sending history");
    sendHistory(page, p->time); 
    readAnything = true;
 }
 
 if (!readAnything){
  Serial.println("nowt in flash to read");
 } 
  //Serial.println("end readout");
  return 1;
}



//dump out the contents of the ram buffer
void readOutRAMValues(){
   Serial.println("dumping ram");
   
   int ss=true;
   unsigned long start=0;
   String toSend;
   
   for (int c = 0; c < timeInRAMCount; c++) { 
     
    if (ss){
      start = usage.time[c];
      ss = !ss;
    }
    else{      
       //SEND individual values OVER BLE HERE
       toSend = String(start) +  "/" + String(usage.time[c]);
       sendMessage(toSend);
       Serial.print("sending: ");
       Serial.println(toSend);
       ss=!ss;
    }     

      //Serial.print("ram index: ");
      //Serial.print(c);
      //Serial.print(" ram value: ");
      //Serial.println(usage.time[c] );
  }

}



//send history over BLE. At the moment is just prints to serial
int sendHistory(int page, unsigned long readOut[256]){
  
  //can send contents of readOut[] array over BLE here as complete array or individual values below
  
  Serial.print("history contents of page: ");
  Serial.println(page);
  
  //Serial.print("value of 53rd element: ");
  //Serial.println(readOut[53]);
  
  int ss=true;
  unsigned long start=0;
  String toSend;

  for (int c = 0; c < 256; c++) { 
    //Serial.println(c);
    if (ss){
      start = readOut[c];
      ss = !ss;
    }
    else{      
       //SEND individual values OVER BLE HERE
       
       toSend = String(start) +  "/" + String(readOut[c]);
       sendMessage(toSend);
       
       
       /*
       Serial.print(" start: ");
       Serial.print(start);
       Serial.print(" stop: ");
       Serial.println(readOut[c]);
       */
       ss=!ss;
    }     
   }
  return 1;
}

void fillFlashMemory(int pagesToFill){
    int multiplier=1;
  for (int numPages = 0; numPages < pagesToFill; numPages++)  {  
      //fill the ram
      for (int c = 0; c < 256; c++)  {   
        saveUsage( c + multiplier);
        
      }
    multiplier++;
  }
  
  
  
}


/*
void setup() {
  Serial.begin(115200);
  Serial.println("All output will appear on the serial monitor.");
  
  
  //fill the memory to fill numPages pages with increasing values
  int multiplier=1;
  for (int numPages = 0; numPages < 14; numPages++)  {  
      //fill the ram
      for (int c = 0; c < 256; c++)  {   
        saveUsage( c + multiplier);
        
      }
    multiplier++;
  }
  
  // aaand another value just to bump it over
  saveUsage( 123);

  //now read out the contents of all the flash pages we've written
  readOutWholeFlash();
  //and whats left in the RAM buffer
  readOutRAMValues();
  
  
}




void loop() {
}

*/
