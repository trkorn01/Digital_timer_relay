#include<SevSeg.h>  
#include <Bounce2.h>
#include <EEPROM.h>

SevSeg sevseg;

Bounce2::Button Program_But = Bounce2::Button();
Bounce2::Button Enter_But = Bounce2::Button();
Bounce2::Button Up_But = Bounce2::Button();
Bounce2::Button Down_But = Bounce2::Button();


//------------------Hard Ware plan-------------------//
#define Button_Program A3
#define Button_Enter   A2
#define Button_Up      A1
#define Button_Down    A0

#define Outpu_Relay1 7
#define Outpu_Relay2 8
#define Outpu_Relay3 9
#define Outpu_Relay4 10

#define disp_A 1
#define disp_B 0
#define disp_C 2
#define disp_D 3
#define disp_E 4
#define disp_F 5
#define disp_G 6

#define display_1 11 
#define display_2 12 
#define display_3 A4 
#define display_4 A5

//------------------Global_Variables-------------------//

bool progActive = false;
int  progPage = 0;

long out1_second=0;
long out1_on_time=0;
long out1_off_time=0;


int ton_out1_sec=0;
int ton_out1_min=0;
int ton_out1_hour=0;

int toff_out1_sec=0;
int toff_out1_min=0;
int toff_out1_hour=0;

int read_ton_out1_sec=0;
int read_ton_out1_min=0;
int read_ton_out1_hour=0;

int read_toff_out1_sec=0;
int read_toff_out1_min=0;
int read_toff_out1_hour=0;

void eepromRead(){

 read_ton_out1_sec = EEPROM.read(1);
 read_ton_out1_min = EEPROM.read(2);
 read_ton_out1_hour = EEPROM.read(3);

 read_toff_out1_sec = EEPROM.read(4);
 read_toff_out1_min = EEPROM.read(5);
 read_toff_out1_hour = EEPROM.read(6);


}
void eepromWrite(){

  EEPROM.write(1,read_ton_out1_sec);
  EEPROM.write(2,read_ton_out1_min);
  EEPROM.write(3,read_ton_out1_hour);

  EEPROM.write(4,read_toff_out1_sec);
  EEPROM.write(5,read_toff_out1_min);
  EEPROM.write(6,read_toff_out1_hour);

}

void setup() {

  //Serial.begin(9600);
  eepromRead();

  byte numDigits = 4;
  byte digitPins[] = {11, 12, A4, A5};
  byte segmentPins[] = {1, 0, 2, 3, 4, 5, 6, 7};
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = true; // Use 'true' if your decimal point doesn't exist or isn't connected
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);

  sevseg.setBrightness(200);

  sevseg.setChars("----"); //ekran açıldı sinyali vermek için konuldu

  Program_But.attach(Button_Program,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  Enter_But.attach(Button_Enter,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  Up_But.attach(Button_Up,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  Down_But.attach(Button_Down,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode

  Program_But.interval(25); // Use a debounce interval of 25 milliseconds
  Enter_But.interval(25); // Use a debounce interval of 25 milliseconds
  Up_But.interval(25); // Use a debounce interval of 25 milliseconds
  Down_But.interval(25); // Use a debounce interval of 25 milliseconds
  
  pinMode(Button_Program,INPUT);
  pinMode(Button_Enter,INPUT);
  pinMode(Button_Up,INPUT);
  pinMode(Button_Down,INPUT);

  pinMode(Outpu_Relay1,OUTPUT);
  pinMode(Outpu_Relay2,OUTPUT);
  pinMode(Outpu_Relay3,OUTPUT);
  pinMode(Outpu_Relay4,OUTPUT);

  cli();                    //Kesmeler durduruldu   
    TCCR1A = 0;               //timer 1 registerler temizleniyor
    TCCR1B = 0;               //timer 1 registerler temizleniyor
    TCNT1  = 0;               //Sayaç sıfırlandı
    OCR1A = 15624;            // ((16*10^6)/((bu değer istediğimiz kesme frekansı(1))*1024))-1 Bize 1Hz kesme frekansı sağlar 
    TCCR1B |= (1 << WGM12);   //CTC modu seçildi
    TCCR1B |= (1 << CS12) |  (1 << CS10);   // prescaler 1024
    TIMSK1 |= (1 << OCIE1A);  
  sei();                    //kesmeleri aktif hale getir
}
bool debug_led = true;

ISR(TIMER1_COMPA_vect){ // kesme her 1 saniyede gerçekleşecek

  out1_on_time = (read_ton_out1_sec) + (read_ton_out1_min * 60) + (read_ton_out1_hour*3600);
  out1_off_time = (read_toff_out1_sec) + (read_toff_out1_min * 60) + (read_toff_out1_hour*3600);

  if( (out1_on_time > 0) && (out1_off_time > 0) ){

    if(digitalRead(Outpu_Relay1) == false){
      if(out1_second >= out1_off_time){
        digitalWrite(Outpu_Relay1,HIGH);
        out1_second = 0;
      }
    }
    else if(out1_second >= out1_on_time ){
        digitalWrite(Outpu_Relay1,LOW);
        out1_second = 0;
      }
    }
  out1_second ++;
}
// d0 d1 pinlerini boşa çıkart debug olmuyor yoksa

void loop() {
 

  Serial.print(progPage);
  Serial.print("\n");
 

  Program_But.update();
  Enter_But.update();
  Up_But.update();
  Down_But.update();
  sevseg.refreshDisplay(); // Must run repeatedly

  if( progActive == false && Program_But.read() == true){ // When the button is pressed and prog mod closed
    if(Program_But.currentDuration() >= 2000){ // If 3 seconds have passed
    progActive = true;
    progPage = 100;
    //eepromRead();
    sevseg.setChars("out1");
    }   
  }

  if(progActive == true){ //main

    if(Program_But.read() == true && Enter_But.read() == true ){ // program ve enter butonuna aynı anda
      if(Program_But.currentDuration() >= 1000 && Enter_But.currentDuration() >= 1000 ){ // 3 saniye basılır ise
      progActive = false; //programlamadan çık
      progPage = 0;
      sevseg.setChars("----");
      eepromWrite();
      //eepromWrite();
      }// basılı tutma süresi programa giriş süresi ile aynı olursa burdan çıkarken program basma süresi eşit olduğu için
      //programlama başlatma hemen çalışıyor  arasına süre farkı koyarak bu hatayı engelle
    }
  }
  
    
    switch (progPage){
//--------------------------------------------------------------------------------------------------------------Out1
      case 100: //ok
        //if(Program_But.rose()){}
        if(Enter_But.rose()){   progPage = 101; sevseg.setChars("SynC"); }
        if(Up_But.rose()){      progPage = 200; sevseg.setChars("OUt2"); }
        if(Down_But.rose()){    progPage = 400; sevseg.setChars("OUt4"); }
      break;
//--------------------------------------------------------------------------------------------------------------sync
      case 101: //ok 
        if(Program_But.rose()){ progPage = 100; sevseg.setChars("OUt1"); }
        if(Enter_But.rose()){   progPage = 110; sevseg.setChars("SEC"); }
        if(Up_But.rose()){      progPage = 102; sevseg.setChars("Asyn"); }     
        if(Down_But.rose()){    progPage = 102; sevseg.setChars("Asyn"); }     
      break;
//--------------------------------------------------------------------------------------------------------------Second settings
     case 110: //ok
        if(Program_But.rose()){ progPage = 101; sevseg.setChars("SynC"); }
        if(Enter_But.rose()){   progPage = 111; sevseg.setNumber(read_ton_out1_sec,1); }
        if(Up_But.rose()){      progPage = 113; sevseg.setChars("nnIn"); }     
        if(Down_But.rose()){    progPage = 116; sevseg.setChars("HOUr"); }     
      break;

      case 111: //ok
        if(Program_But.rose()){ progPage = 110; sevseg.setChars("SEC"); read_toff_out1_sec = read_ton_out1_sec;} 
        if(Enter_But.rose()){   progPage = 110; sevseg.setChars("SEC"); read_toff_out1_sec = read_ton_out1_sec;} 
        if(Up_But.rose()){      read_ton_out1_sec++;  if(read_ton_out1_sec == 61) read_ton_out1_sec = 0; sevseg.setNumber(read_ton_out1_sec,1); }     
        if(Down_But.rose()){    read_ton_out1_sec--;  if(read_ton_out1_sec == -1) read_ton_out1_sec = 60; sevseg.setNumber(read_ton_out1_sec,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------Minute settings
      case 113: //ok
        if(Program_But.rose()){ progPage = 101; sevseg.setChars("SynC"); }
        if(Enter_But.rose()){   progPage = 114; sevseg.setNumber(read_ton_out1_min,1); }
        if(Up_But.rose()){      progPage = 116; sevseg.setChars("HOUr"); }     
        if(Down_But.rose()){    progPage = 110; sevseg.setChars("SEC"); }     
      break;

      case 114: 
        if(Program_But.rose()){ progPage = 113; sevseg.setChars("nnIn"); read_toff_out1_min = read_ton_out1_min; } 
        if(Enter_But.rose()){   progPage = 113; sevseg.setChars("nnIn"); read_toff_out1_min = read_ton_out1_min; } 
        if(Up_But.rose()){      read_ton_out1_min++;  if(read_ton_out1_min == 61) read_ton_out1_min = 0; sevseg.setNumber(read_ton_out1_min,1); }     
        if(Down_But.rose()){    read_ton_out1_min--;  if(read_ton_out1_min == -1) read_ton_out1_min = 60; sevseg.setNumber(read_ton_out1_min,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------Hour settings
      case 116: 
        if(Program_But.rose()){ progPage = 101; sevseg.setChars("SynC"); }
        if(Enter_But.rose()){   progPage = 117; sevseg.setNumber(read_ton_out1_hour,1); }
        if(Up_But.rose()){      progPage = 110; sevseg.setChars("SEC"); }     
        if(Down_But.rose()){    progPage = 113; sevseg.setChars("nIn"); }     
      break;

      case 117: 
        if(Program_But.rose()){ progPage = 116; sevseg.setChars("HOUr"); read_toff_out1_hour = read_ton_out1_hour; } 
        if(Enter_But.rose()){   progPage = 116; sevseg.setChars("HOUr"); read_toff_out1_hour = read_ton_out1_hour; }
        if(Up_But.rose()){      read_ton_out1_hour++;  if(read_ton_out1_hour == 256) read_ton_out1_hour = 0; sevseg.setNumber(read_ton_out1_hour,1); }     
        if(Down_But.rose()){    read_ton_out1_hour--;  if(read_ton_out1_hour == -1) read_ton_out1_hour = 255; sevseg.setNumber(read_ton_out1_hour,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------Asyn
    case 102: //ok
      if(Program_But.rose()){ progPage = 100; sevseg.setChars("OUt1"); }
      if(Enter_But.rose()){   progPage = 120; sevseg.setChars("Ton"); }
      if(Up_But.rose()){      progPage = 101; sevseg.setChars("SynC"); }     
      if(Down_But.rose()){    progPage = 101; sevseg.setChars("SynC"); }     
    break;
//--------------------------------------------------------------------------------------------------------------Ton
    case 120: //ok
      if(Program_But.rose()){ progPage = 102; sevseg.setChars("Asyn"); } 
      if(Enter_But.rose()){   progPage = 121; sevseg.setChars("SEC"); }
      if(Up_But.rose()){      progPage = 130; sevseg.setChars("toff"); }     
      if(Down_But.rose()){    progPage = 130; sevseg.setChars("toff"); }     
    break;
//--------------------------------------------------------------------------------------------------------------Ton Second settings
     case 121: //ok
        if(Program_But.rose()){ progPage = 120 ; sevseg.setChars("Ton"); }
        if(Enter_But.rose()){   progPage = 122; sevseg.setNumber(read_ton_out1_sec,1); }
        if(Up_But.rose()){      progPage = 123; sevseg.setChars("nnIn"); }     
        if(Down_But.rose()){    progPage = 126; sevseg.setChars("HOUr"); }     
      break;

      case 122: 
        if(Program_But.rose()){ progPage = 121; sevseg.setChars("SEC");} 
        if(Enter_But.rose()){   progPage = 121; sevseg.setChars("SEC");} 
        if(Up_But.rose()){      read_ton_out1_sec++;  if(read_ton_out1_sec == 61) read_ton_out1_sec = 0; sevseg.setNumber(read_ton_out1_sec,1); }     
        if(Down_But.rose()){    read_ton_out1_sec--;  if(read_ton_out1_sec == -1) read_ton_out1_sec = 60; sevseg.setNumber(read_ton_out1_sec,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------Ton Minute settings
      case 123: //ok
        if(Program_But.rose()){ progPage = 120; sevseg.setChars("Ton"); }
        if(Enter_But.rose()){   progPage = 124; sevseg.setNumber(read_ton_out1_min,1); }
        if(Up_But.rose()){      progPage = 126; sevseg.setChars("HOUr"); }     
        if(Down_But.rose()){    progPage = 121; sevseg.setChars("SEC"); }     
      break;

      case 124: 
        if(Program_But.rose()){ progPage = 123; sevseg.setChars("nnIn"); } 
        if(Enter_But.rose()){   progPage = 123; sevseg.setChars("nnIn"); } 
        if(Up_But.rose()){      read_ton_out1_min++;  if(read_ton_out1_min == 61) read_ton_out1_min = 0; sevseg.setNumber(read_ton_out1_min,1); }     
        if(Down_But.rose()){    read_ton_out1_min--;  if(read_ton_out1_min == -1) read_ton_out1_min = 60; sevseg.setNumber(read_ton_out1_min,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------TonHour settings
      case 126: //ok
        if(Program_But.rose()){ progPage = 120; sevseg.setChars("Ton"); }
        if(Enter_But.rose()){   progPage = 127; sevseg.setNumber(read_ton_out1_hour,1); }
        if(Up_But.rose()){      progPage = 121; sevseg.setChars("SEC"); }     
        if(Down_But.rose()){    progPage = 123; sevseg.setChars("nIn"); }     
      break;

      case 127: 
        if(Program_But.rose()){ progPage = 126; sevseg.setChars("HOUr"); } 
        if(Enter_But.rose()){   progPage = 126; sevseg.setChars("HOUr"); }
        if(Up_But.rose()){      read_ton_out1_hour++;  if(read_ton_out1_hour == 256) read_ton_out1_hour = 0; sevseg.setNumber(read_ton_out1_hour,1); }     
        if(Down_But.rose()){    read_ton_out1_hour--;  if(read_ton_out1_hour == -1) read_ton_out1_hour = 255; sevseg.setNumber(read_ton_out1_hour,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------Toff
    case 130: //ok
      if(Program_But.rose()){ progPage = 102; sevseg.setChars("Asyn"); }
      if(Enter_But.rose()){   progPage = 131 ; sevseg.setChars("SEC"); }
      if(Up_But.rose()){      progPage = 120; sevseg.setChars("ton"); }     
      if(Down_But.rose()){    progPage = 120; sevseg.setChars("ton"); }     
    break;
//--------------------------------------------------------------------------------------------------------------Toff Second settings
     case 131: 
        if(Program_But.rose()){ progPage = 130 ; sevseg.setChars("toff"); }
        if(Enter_But.rose()){   progPage = 132; sevseg.setNumber(read_toff_out1_sec,1); }
        if(Up_But.rose()){      progPage = 133; sevseg.setChars("nnIn"); }     
        if(Down_But.rose()){    progPage = 136; sevseg.setChars("HOUr"); }     
      break;

      case 132: 
        if(Program_But.rose()){ progPage = 131; sevseg.setChars("SEC");} 
        if(Enter_But.rose()){   progPage = 131; sevseg.setChars("SEC");} 
        if(Up_But.rose()){      read_toff_out1_sec++;  if(read_toff_out1_sec == 61) read_toff_out1_sec = 0; sevseg.setNumber(read_toff_out1_sec,1); }     
        if(Down_But.rose()){    read_toff_out1_sec--;  if(read_toff_out1_sec == -1) read_toff_out1_sec = 60; sevseg.setNumber(read_toff_out1_sec,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------Toff Minute settings
      case 133: 
        if(Program_But.rose()){ progPage = 130; sevseg.setChars("toff"); }
        if(Enter_But.rose()){   progPage = 134; sevseg.setNumber(read_toff_out1_min,1); }
        if(Up_But.rose()){      progPage = 136; sevseg.setChars("HOUr"); }     
        if(Down_But.rose()){    progPage = 131; sevseg.setChars("SEC"); }     
      break;

      case 134: 
        if(Program_But.rose()){ progPage = 133; sevseg.setChars("nnIn"); } 
        if(Enter_But.rose()){   progPage = 133; sevseg.setChars("nnIn"); } 
        if(Up_But.rose()){      read_toff_out1_min++;  if(read_toff_out1_min == 61) read_toff_out1_min = 0; sevseg.setNumber(read_toff_out1_min,1); }     
        if(Down_But.rose()){    read_toff_out1_min--;  if(read_toff_out1_min == -1) read_toff_out1_min = 60; sevseg.setNumber(read_toff_out1_min,1); }     
      break;
//--------------------------------------------------------------------------------------------------------------TonHour settings
      case 136: 
        if(Program_But.rose()){ progPage = 130; sevseg.setChars("toff"); }
        if(Enter_But.rose()){   progPage = 137; sevseg.setNumber(read_toff_out1_hour,1); }
        if(Up_But.rose()){      progPage = 131; sevseg.setChars("SEC"); }     
        if(Down_But.rose()){    progPage = 133; sevseg.setChars("nIn"); }     
      break;

      case 137: 
        if(Program_But.rose()){ progPage = 126; sevseg.setChars("HOUr"); } 
        if(Enter_But.rose()){   progPage = 126; sevseg.setChars("HOUr"); }
        if(Up_But.rose()){      read_toff_out1_hour++;  if(read_toff_out1_hour == 256) read_toff_out1_hour = 0; sevseg.setNumber(read_toff_out1_hour,1); }     
        if(Down_But.rose()){    read_toff_out1_hour--;  if(read_toff_out1_hour == -1) read_toff_out1_hour = 255; sevseg.setNumber(read_toff_out1_hour,1); }     
      break;




  }
}


/*
  if( Enter_But.pressed() ) count= count-10;
  if( Up_But.pressed() ) count= count+1;
  if( Down_But.pressed() ) count= count-1;


  sevseg.setNumber(count, 1); 
*/


/* TEST CODE HARDWARE OKEY
  if(digitalRead(Button_Program) == HIGH) digitalWrite(Outpu_Relay1, HIGH);
    else digitalWrite(Outpu_Relay1, LOW);

  if(digitalRead(Button_Enter) == HIGH) digitalWrite(Outpu_Relay2, HIGH);
    else digitalWrite(Outpu_Relay2, LOW);

  if(digitalRead(Button_Up) == HIGH) digitalWrite(Outpu_Relay3, HIGH);
    else digitalWrite(Outpu_Relay3, LOW);

  if(digitalRead(Button_Down) == HIGH) digitalWrite(Outpu_Relay4, HIGH);
    else digitalWrite(Outpu_Relay4, LOW);

  digitalWrite(display_1,HIGH);
  digitalWrite(display_2,HIGH);
  digitalWrite(display_3,HIGH);
  digitalWrite(display_4,HIGH);

  digitalWrite(disp_A,LOW);
  digitalWrite(disp_B,HIGH);
  digitalWrite(disp_C,LOW);
  digitalWrite(disp_D,HIGH);
  digitalWrite(disp_E,LOW);
  digitalWrite(disp_F,HIGH);
  digitalWrite(disp_G,LOW);
  
*/
  

