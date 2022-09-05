#include <Adafruit_TFTLCD.h> 
#include <Adafruit_GFX.h>    
#include <TouchScreen.h>
#include <avr/pgmspace.h>
#include "graphics.h"

#define LCD_CS A3 
#define LCD_CD A2 
#define LCD_WR A1 
#define LCD_RD A0 
#define LCD_RESET A4 

#define TS_MINX 110
#define TS_MINY 80
#define TS_MAXX 920
#define TS_MAXY 900

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);


bool phone_on = false;

bool drawed = false;
int screen = 5;
unsigned long previousTime = 0;
int threshold = 50;

bool first_ring = true;

int cont=0;

unsigned long previousSIMTime = 0;
bool hang_up = false;
bool pick_up = false;


void setup() {
  delay(1000);
  
  tft.reset();
  tft.begin(0x9341);
 
  tft.setRotation(0);
  tft.fillScreen(BLACK);
}


void loop() {


  if(!phone_on){
    byte on_button_width = 162;
    byte on_button_height = 162;
    if(!drawed){
      drawed = true;
      tft.fillScreen(BLACK);
      phone_off(on_button_width, on_button_height);
    }
    if(millis()-previousTime > threshold){
      phone_off_handler(on_button_width, on_button_height);
      previousTime = millis();
    }
    previousSIMTime = millis();
    return;
  }
  
  //Comprobamos si la SIM está conectado al operador.
  //Si no lo está, la reiniciamos y la volvemos a conectar
  if(millis()-previousSIMTime > 30000){
    Serial.println("AT+CREG?");
    previousSIMTime = millis();
  }
  
  if(Serial.available()){
    String readSerial = Serial.readString();
    readSerial.trim();
    //Nos llega una llamada
    if(readSerial.indexOf("RING") != -1 && hang_up==false && pick_up==false){
      drawed = false;
      screen = 4;
    }
    //Nos cuelgan la llamada
    else if(readSerial.indexOf("NO") != -1){
      drawed = false;
      screen = 0;
    }

    else if(readSerial.indexOf("CREG") != -1 && readSerial.indexOf("0,5")== -1 && readSerial.indexOf("0,1")== -1){
      tft.setCursor(10, 280);
      tft.setTextSize(2);
      tft.print("Reconnecting SIM");
      reconnect_to_operator();
      tft.fillRect(10, 280, 220, 320, BLACK);
    }
  }
  
  
  if(screen==0){
    hang_up = false;
    pick_up = false;
    
    if(!drawed){
      drawed = true;
      tft.fillScreen(BLACK);
      drawMainMenu();
    }
    if(millis()-previousTime > threshold){
      main_menu_touch_handler();
      previousTime = millis();
    }
  }
  
  else if(screen == 1){
    byte x_rect_menu = 0;
    byte y_rect_menu = 115;
    byte button_width = 73;
    byte button_height = 40;
    byte margin = 5;
    byte x_rect_number = 10;
    byte y_rect_number = 10;
    
    if(!drawed){
      drawed = true;
      tft.fillScreen(BLACK);
      draw_number_buttons_menu(x_rect_menu, y_rect_menu, button_width, button_height, margin, x_rect_number, y_rect_number);
    }
    if(millis()-previousTime > threshold){
      number_menu_touch_handler(x_rect_menu, y_rect_menu, button_width, button_height, margin, x_rect_number, y_rect_number);
      previousTime = millis();
    }
  }
    
  else if(screen == 2){
    if(!drawed){
      drawed = true;
      tft.fillScreen(BLACK);
      draw_text_messages_menu();
    }
  }

  else if(screen == 3){
    byte hang_up_button_width = 140;
    byte hang_up_button_height = 60;
    byte y0 = 200;

    
    if(drawed==false && first_ring==true){
      drawed = true;
      first_ring = false;
      tft.fillScreen(BLACK);
      call_in_progress(hang_up_button_width, hang_up_button_height, y0);
    }
    
    if(millis()-previousTime > threshold){
      call_in_progress_handler(hang_up_button_width, hang_up_button_height, y0);
      previousTime = millis();
    }
  }

  else if(screen == 4){
    byte hang_up_button_width = 180;
    byte hang_up_button_height = 60;
    byte y01 = 200;
    byte y02 = 120;
    
    if(!drawed){
      drawed = true;
      tft.fillScreen(BLACK);
      incoming_call(hang_up_button_width, hang_up_button_height, y01, y02);
    }
    
    if(millis()-previousTime > threshold){
      incoming_call_handler(hang_up_button_width, hang_up_button_height ,y01, y02);
      previousTime = millis();
    }
  }

  else if(screen == 5){
    if(!drawed){
      drawed = true;  
      tft.fillScreen(BLACK);
      unlock_phone();
    }
  }
}


////////////////////////////////////////////////////////////
////////////////////////SIM900//////////////////////////////
////////////////////////////////////////////////////////////
void turn_on_SIM(){
  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);
  delay(1000);
}

void connect_to_operator(){
  while(true){
    Serial.println("AT");
    if(Serial.available()){
      String readSerial = Serial.readString();
      readSerial.trim();
      if(readSerial.indexOf("OK")!= -1) 
        break;
    }
    delay(1000);
  }

  while(true){
    //SUSTITUYE 1234 POR EL PIN DE TU TARJETA SIM
    Serial.println("AT+CPIN=1234");  
    if(Serial.available()){
      String readSerial = Serial.readString();
      readSerial.trim();
      if(readSerial.indexOf("OK")!= -1) 
        break;
    }
    delay(1000);
  }
}

void reconnect_to_operator(){
  digitalWrite(10, HIGH);
  delay(200);
  turn_on_SIM();
  delay(200);
  turn_on_SIM();
  delay(200);
  connect_to_operator();
}

////////////////////////////////////////////////////////////
////////////////PANTALLA PARA LLAMADAS//////////////////////
////////////////////////////////////////////////////////////

//x, y: posición de la esquina superior izquierda del rectángulo
void drawNumberButton(byte number, byte x, byte y, byte rect_width, byte rect_height){
  uint32_t color = BLUE;

  tft.fillRect(x, y, rect_width, rect_height, color);

  tft.setTextColor(WHITE); 
  int text_width, text_height;
  int aux_w, aux_h;
  tft.setTextSize(4); 

  if(number==60 || number==76){
    char aux_number = (char)number;
    tft.getTextBounds((String)aux_number, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
    tft.setCursor(x+rect_width/2-text_width/2, y+rect_height/2-text_height/2);
    tft.print((char)number);
  }
  else{
    tft.getTextBounds((String)number, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
    tft.setCursor(x+rect_width/2-text_width/2, y+rect_height/2-text_height/2);
    tft.print(number);
  }
}


//X: anchura (columnas), Y: altura (filas)
void draw_number_buttons_menu(byte x_rect_menu, byte y_rect_menu, byte button_width, byte button_height, 
                              byte margin, byte x_rect_number, byte y_rect_number){
  //Esquina superior izquierda del menú de números
  //byte x_rect_menu = 0; 
  //byte y_rect_menu = 115; 
  //byte margin = 5;

  tft.drawRect(x_rect_number, y_rect_number, tft.width()-2*x_rect_number, y_rect_menu-2*y_rect_number, WHITE); 
  
  tft.fillRect(x_rect_menu, y_rect_menu, tft.width()-x_rect_menu, tft.height()-y_rect_menu, WHITE);

  //Posición esquina superior izquierda de cada rectángulo de número
  byte pos_x = 0;
  byte pos_y = 0;
  //Anchura y altura de los rectángulos de número
  //byte button_width = 73;
  //byte button_height = 40;
  byte number = 1;

  for(int i= 0; i<4; i++){
    pos_x = x_rect_menu + margin;
    pos_y = y_rect_menu + i*(button_height+margin) + margin;
    for(int j=0; j<3; j++){
      
      //Última fila
      if(i==3 && j==0) drawNumberButton('<', pos_x, pos_y, button_width, button_height);
      else if(i==3 && j==1) drawNumberButton(0, pos_x, pos_y, button_width, button_height);
      else if(i==3 && j==2) drawNumberButton('L', pos_x, pos_y, button_width, button_height);
      //Números del 1 al 9
      else drawNumberButton(number, pos_x, pos_y, button_width, button_height);
         
      pos_x += button_width + margin;
      number++;  
    }
  }
}

void number_menu_touch_handler(byte x_rect_menu, byte y_rect_menu, byte button_width, byte button_height, 
                              byte margin, byte x_rect_number, byte y_rect_number){
  struct Pixel{
    unsigned int x;
    unsigned int y;
  };

  Pixel pixel;
  
  byte number = 1;
  byte pos_x = 0;
  byte pos_y = 0;
  static unsigned long call_number = 0;
  
  int text_width, text_height;
  int aux_w, aux_h;
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();  //Get touch point
  digitalWrite(13, LOW);
  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  

  if(p.z > ts.pressureThreshhold){
    pixel.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
    pixel.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    
    for(int i= 0; i<4; i++){
      pos_x = x_rect_menu + margin;
      pos_y = y_rect_menu + i*(button_height+margin) + margin;
      for(int j=0; j<3; j++){
        if((pixel.x>=pos_x && pixel.x<=pos_x+button_width) && (pixel.y>=pos_y && pixel.y<=pos_y+button_height)){
          if(number==10) call_number /= 10;
          else if(number==11 && call_number < 100000000) call_number*=10;
          else if(number==12){
            if(call_number!=0){
              screen = 3;
              Serial.println("ATD" + String(call_number) + ";");      //Se realiza la llamada
              first_ring = true;
              call_number = 0;
            }
            else
              screen = 0;
            drawed = false;
          }
          else if(call_number<=100000000) call_number*=10, call_number+=number;
          tft.setTextSize(3);
          
          tft.fillRect(x_rect_number+5, y_rect_number+5, tft.width()-2*x_rect_number-10, y_rect_menu-2*y_rect_number-10, BLACK);
          tft.getTextBounds((String)call_number, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
          tft.setCursor((tft.width() - text_width)/2, (y_rect_menu - text_height)/2);
          tft.print((String)call_number);
        }
        pos_x += button_width + margin;
        number++;  
      }
    }
  }  
}


void call_in_progress(byte hang_up_button_width, byte hang_up_button_height, byte y0){
  int aux_w, aux_h, text_width, text_height;

  tft.fillScreen(BLACK);
  tft.fillRect(tft.width()/2-hang_up_button_width/2, y0, hang_up_button_width, hang_up_button_height, RED);

  tft.setTextSize(3);
  String hang_up_text = "COLGAR";
  tft.getTextBounds((String)hang_up_text, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, y0+(hang_up_button_height-text_height)/2);
  tft.print(hang_up_text);
  
  String call_in_progress_text_1 = "LLAMADA EN";
  String call_in_progress_text_2 = "CURSO";

  tft.getTextBounds((String)call_in_progress_text_1, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 30);
  tft.print(call_in_progress_text_1);

  tft.getTextBounds((String)call_in_progress_text_2, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 60);
  tft.print(call_in_progress_text_2);
}


void call_in_progress_handler(byte hang_up_button_width, byte hang_up_button_height, byte y0){
  struct Pixel{
    unsigned int x;
    unsigned int y;
  };

  Pixel pixel;

  int pos_x0 = (tft.width()-hang_up_button_width)/2;
  int pos_xf = pos_x0 + hang_up_button_width;
  int pos_y0 = y0;
  int pos_yf = y0 + hang_up_button_height;
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();  //Get touch point
  digitalWrite(13, LOW);
  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  

  if(p.z > ts.pressureThreshhold){
    pixel.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
    pixel.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    if(pixel.x>=pos_x0 && pixel.x<=pos_xf && pixel.y>=pos_y0 && pixel.y<=pos_yf){
      Serial.println("ATH");
      screen = 0;
      drawed = false;
    }
  }


  if(Serial.available()){
    String readSerial = Serial.readString();
    readSerial.trim();
    //Nos cuelgan la llamada
    if(readSerial.indexOf("NO") != -1){
      drawed = false;
      screen = 0;
    }
  }
}


void incoming_call(byte hang_up_button_width, byte hang_up_button_height, byte y01, byte y02){
  int aux_w, aux_h, text_width, text_height;

  tft.fillScreen(BLACK);
  tft.fillRect(tft.width()/2-hang_up_button_width/2, y01, hang_up_button_width, hang_up_button_height, RED);
  tft.fillRect(tft.width()/2-hang_up_button_width/2, y02, hang_up_button_width, hang_up_button_height, GREEN);

  tft.setTextSize(3);
  String hang_up_text = "COLGAR";
  tft.getTextBounds((String)hang_up_text, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, y01+(hang_up_button_height-text_height)/2);
  tft.print(hang_up_text);

  String pick_up_text = "DESCOLGAR";
  tft.getTextBounds((String)pick_up_text, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, y02+(hang_up_button_height-text_height)/2);
  tft.print(pick_up_text);
  
  String call_in_progress_text_1 = "LLAMADA";
  String call_in_progress_text_2 = "ENTRANTE";

  tft.getTextBounds((String)call_in_progress_text_1, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 30);
  tft.print(call_in_progress_text_1);

  tft.getTextBounds((String)call_in_progress_text_2, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 60);
  tft.print(call_in_progress_text_2);
}


void incoming_call_handler(byte hang_up_button_width, byte hang_up_button_height, byte y01, byte y02){
  struct Pixel{
    unsigned int x;
    unsigned int y;
  };

  Pixel pixel;

  int pos_x01 = (tft.width()-hang_up_button_width)/2;
  int pos_xf1 = pos_x01 + hang_up_button_width;
  int pos_y01 = y01;
  int pos_yf1 = y01 + hang_up_button_height;

  int pos_x02 = (tft.width()-hang_up_button_width)/2;
  int pos_xf2 = pos_x02 + hang_up_button_width;
  int pos_y02 = y02;
  int pos_yf2 = y02 + hang_up_button_height;
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();  //Get touch point
  digitalWrite(13, LOW);
  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  

  if(p.z > ts.pressureThreshhold){
    pixel.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
    pixel.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    if(pixel.x>=pos_x01 && pixel.x<=pos_xf1 && pixel.y>=pos_y01 && pixel.y<=pos_yf1){
      Serial.println("ATH");
      screen = 0;
      drawed = false;
      hang_up = true;
      first_ring = true;
    }

    else if(pixel.x>=pos_x02 && pixel.x<=pos_xf2 && pixel.y>=pos_y02 && pixel.y<=pos_yf2){
      Serial.println("ATA");
      screen = 3;
      drawed = false;
      pick_up = true;
      first_ring = true;
    }
  }
}



////////////////////////////////////////////////////////////
////////////////PANTALLA PARA MENSAJES//////////////////////
////////////////////////////////////////////////////////////
void drawLetterButton(String letter, byte x, byte y, byte button_width, byte button_height){
  byte rect_width = button_width;
  byte rect_height = button_height;
  uint32_t color = BLUE;

  tft.fillRect(x, y, rect_width, rect_height, color);

  tft.setTextColor(WHITE); 
  int text_width, text_height;
  int aux_w, aux_h;
  tft.setTextSize(2); 

  tft.getTextBounds(letter, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor(x+rect_width/2-text_width/2, y+rect_height/2-text_height/2);
  tft.print(letter);
}


void draw_text_messages_menu(){
  //Esquina superior izquierda del menú de números
  byte x_rect_menu = 0; 
  byte y_rect_menu = 120; 
  byte margin = 3;

  byte pos_x = 0;
  byte pos_y = 0;
  byte button_width = 20;
  byte button_height = 30;
  String first_three_lines_keyboard[3][10] = {
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"}, 
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", "Ñ"},
    {"¿", "?", "Z", "X", "C", "V", "B", "N", "M", "<"}
  };
  String fourd_line_keyboard[4] = {",", "SPACE", ".", "ENTER"};
  String last_line_keyboard[2] = {"SEND", "RETURN"};

  byte xi_rect_message = 10;
  byte yi_rect_message = 10;
  byte xf_rect_message = 220;
  byte yf_rect_message = 100;


  tft.drawRect(xi_rect_message, yi_rect_message, xf_rect_message, yf_rect_message, WHITE);
  
  tft.fillRect(x_rect_menu, y_rect_menu, tft.width()-x_rect_menu, tft.height()-y_rect_menu, WHITE);

  for(int i= 0; i<3; i++){
    pos_x = x_rect_menu + margin;
    pos_y = y_rect_menu + i*(button_height+margin) + margin;
    for(int j=0; j<10; j++){    
      drawLetterButton(first_three_lines_keyboard[i][j], pos_x, pos_y, button_width, button_height);        
      pos_x += button_width + margin;
    }
  }


  pos_x = x_rect_menu + margin;
  pos_y = y_rect_menu + 3*(button_height+margin) + margin;
  for(int i=0; i<4; i++){
    if(fourd_line_keyboard[i]=="SPACE") drawLetterButton(fourd_line_keyboard[i], pos_x, pos_y, button_width+75, button_height),    pos_x +=75;
    else if (fourd_line_keyboard[i]=="ENTER") drawLetterButton(fourd_line_keyboard[i], pos_x, pos_y, button_width+60, button_height),     pos_x += 60;
    else drawLetterButton(fourd_line_keyboard[i], pos_x, pos_y, button_width, button_height);
    pos_x += button_width + margin;
  }

  pos_x = x_rect_menu + margin;
  pos_y = y_rect_menu + 4*(button_height+margin) + margin;
  drawLetterButton(last_line_keyboard[0], pos_x, pos_y, 110, button_height), pos_x += 110+margin;
  drawLetterButton(last_line_keyboard[1], pos_x, pos_y, 110, button_height);
  
}

////////////////////////////////////////////////////////////
///////////////////////BITMAP///////////////////////////////
////////////////////////////////////////////////////////////
void drawBitmap(int16_t x, int16_t y, byte *bitmap, uint16_t color, byte scale_factor) {
  int cont = 0;

  for(int i=0; i<20*scale_factor; i+=scale_factor) {
    for(int j=0; j<20*scale_factor; j+=scale_factor) {
      byte pixel = pgm_read_byte(&(bitmap[cont]));
      for(int k=0; k<scale_factor; k++){
        for(int p=0; p<scale_factor; p++){
          if(pixel==0){
            tft.drawPixel(x+j+p, y+i+k, color);
            //tft.drawPixel(x+j+k, y+i, color);
            //tft.drawPixel(x+j, y+i+k, color);
            //tft.drawPixel(x+j+k, y+i+k, color);
          }
        }
      }
      cont++;
    }
  }
}

////////////////////////////////////////////////////////////
///////////////////MENU PRINCIPAL///////////////////////////
////////////////////////////////////////////////////////////

void drawMainMenu(){
  byte button_width = 100;
  byte button_height = 100;
  byte margin = 15;
  int pos_x = margin;
  int pos_y = margin;
  
  tft.fillRect(pos_x, pos_y, button_width, button_height, BLUE);
  drawBitmap(pos_x, pos_y, sms_icon, BLACK, 5);

  pos_x += button_width + margin;
  //pos_y += button_height + margin;
  tft.fillRect(pos_x, pos_y, 100, 100, BLUE);
  drawBitmap(pos_x, pos_y, call_icon, BLACK, 5);

  tft.fillRect((tft.width()-button_width)/2, 150, button_width, button_height, BLUE);
  drawBitmap((tft.width()-button_width)/2, 150, onoff_icon, BLACK, 5);
  
}

void main_menu_touch_handler(){

  struct Pixel{
    unsigned int x;
    unsigned int y;
  };

  Pixel pixel;
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();  //Get touch point
  digitalWrite(13, LOW);
  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  

  if(p.z > ts.pressureThreshhold){
    pixel.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
    pixel.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    if((pixel.x>10 && pixel.x<110) && (pixel.y>10&&pixel.y<110)){
      drawed = false;
      screen = 2;
    }
    else if((pixel.x>120 && pixel.x<220) && (pixel.y>10&&pixel.y<110)){
      drawed = false;
      screen = 1;
    }
    else if(pixel.x > (tft.width()-100)/2 && pixel.x < (tft.width()-100)/2 + 100 && pixel.y>150 && pixel.y<250){
      tft.fillScreen(BLACK);
      tft.setCursor(20, 20);
      tft.setTextSize(3);
      tft.print("Apagando");
      turn_on_SIM();
      drawed = false;
      phone_on = false;
      delay(1500);
    }
    
  }
}


////////////////////////////////////////////////////////////
////////////////////MÓVIL APAGADO///////////////////////////
////////////////////////////////////////////////////////////
void phone_off(byte on_button_width, byte on_button_height){
  int pos_x = (tft.width()-on_button_width)/2;
  int pos_y = (tft.height()-on_button_height)/2;

  tft.fillRect(pos_x, pos_y, on_button_width, on_button_height, BLUE);
  drawBitmap(pos_x, pos_y, onoff_icon, BLACK, 8);
}


void phone_off_handler(byte on_button_width, byte on_button_height){
  struct Pixel{
    unsigned int x;
    unsigned int y;
  };

  Pixel pixel;

  int pos_x1 = (tft.width()-on_button_width)/2;
  int pos_x2 = pos_x1 + on_button_width;
  int pos_y1 = (tft.height()-on_button_height)/2;
  int pos_y2 = pos_y1 + on_button_height;
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();  //Get touch point
  digitalWrite(13, LOW);
  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  

  if(p.z > ts.pressureThreshhold){
    pixel.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
    pixel.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    if((pixel.x>pos_x1 && pixel.x<pos_x2) && (pixel.y>pos_y1&&pixel.y<pos_y2)){
      drawed = false;
      screen = 5;
      phone_on = true;
    }
  }
}


////////////////////////////////////////////////////////////
////////////////////MÓVIL APAGADO///////////////////////////
////////////////////////////////////////////////////////////
void unlock_phone(){
  String text1 = "USAR HUELLA";
  String text2 = "DACTILAR";
  String text3 = "INICIANDO";

  int aux_w, aux_h, text_width, text_height;

  tft.setTextSize(2);
  tft.getTextBounds((String)text1, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 20);
  tft.print(text1);

  tft.getTextBounds((String)text2, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 40);
  tft.print(text2);

  delay(5000);

  tft.fillScreen(BLACK);
  tft.setTextSize(3);
  tft.getTextBounds((String)text3, 0, 0, &aux_w, &aux_h, &text_width, &text_height);
  tft.setCursor((tft.width()-text_width)/2, 20);
  tft.print(text3);

  
  turn_on_SIM();
  Serial.begin(19200);
  delay(3000);
  connect_to_operator();
  delay(1000);

  drawed = false;
  screen = 0;
}
