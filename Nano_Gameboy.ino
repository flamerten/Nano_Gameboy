/*
 * ST7789 SPI display without CS pin, with joystick no switch
 * TFT Display: https://github.com/cbm80amiga/Arduino_ST7789_Fast (NOTE  only for 240x240 display)
 * Joystick: https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-joystick-with-serial-monitor-1f04f0
 * Adafruit gfx: https://learn.adafruit.com/adafruit-gfx-graphics-library/overview
 * 
 * Project Snake Game
 * --------------------------------
 * HardwareSerial in Arduino has no print F?
 * 
 * 
 */

#include <Arduino_ST7789_Fast.h> 
#include <SPI.h>
#include <Adafruit_GFX.h>

#define TFT_DC  10
#define TFT_RST 9

#define DELAY_VALUE 100 //increase speed for slower movement

//Joystick
const int VRx = A4;
const int VRy = A3;
int SnakeDirection = 0; //NESW 1234 --> 0 if nothing Don't move at first
#define JOYSTICK_LIM 400

//Set these to centre coordinates
#define SQUARE_SIZE 14 //EVEN pls?
//#define STARTX 120 //Made them random
//#define STARTY 120
const uint16_t SNAKE_COLOUR = BLUE;
const uint16_t APPLE_COLOUR = RED;
const uint16_t MAXLENGTH = 100; //Data structure

//DATA STRUCTURE for storing the snake boxes
//StartPos = StartPos - 1 > Next Position
//StartPos = StartPos + Length > Last Position
uint16_t SnakeX[MAXLENGTH];
uint16_t SnakeY[MAXLENGTH];
uint16_t StartPos = 0;

//Initialise ApplePosition as impossible value
uint16_t AppleX = 2000; 
uint16_t AppleY = 2000;
bool AppleEaten = true; //So that we have one apple at the start

uint16_t PossibleCoord[240/SQUARE_SIZE]; //for starting snake and random apples
uint16_t Length = 1; //Starting length of the snake - seems to give a length of 2?
uint16_t Score = 0; //Score and Length are diff as the Length is restricted to MAXLENGTH

const int SCREEN_BORDER_MAX = 240 - (SQUARE_SIZE);
const int SCREEN_BORDER_MIN = 0 + (SQUARE_SIZE);

Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST);
 
void UpdateJoyStick(){  
  int xPosition = analogRead(VRx);
  int yPosition = analogRead(VRy);
  int mapX = map(xPosition, 0, 1023, -512, 512);
  int mapY = map(yPosition, 0, 1023, -512, 512);

  int newSnakeDirection;

  if(mapX < -JOYSTICK_LIM)            newSnakeDirection = 1; //N
  else if( mapY < -JOYSTICK_LIM)      newSnakeDirection = 2; //E
  else if( mapX >  JOYSTICK_LIM)      newSnakeDirection = 3; //S
  else if( mapY >  JOYSTICK_LIM)      newSnakeDirection = 4; //W
  else return; //Same direction just continue along

  if(SnakeDirection == 0){ //At the beginning all directions are possible
    SnakeDirection = newSnakeDirection;
  }
  else if( (SnakeDirection+2 == newSnakeDirection) || (SnakeDirection-2 == newSnakeDirection)){
    return; //Ignore commands for going opposite direction
  }
  else{
    SnakeDirection = newSnakeDirection;
  }

  //else no change in SnakeDirection
}

void myFillRect(int x, int y, uint16_t colour){
  //x, y is centre
  tft.fillRect(
    x - SQUARE_SIZE/2,
    y - SQUARE_SIZE/2,
    SQUARE_SIZE,
    SQUARE_SIZE,
    colour); 
}

void initSnake(){
  //Draw the first box or starting snake, initialise data structure
  //Fill PossibleCoordinates
  int STARTX, STARTY;

  //Dont use the start and the end
  for(int i = 1; i < 240/SQUARE_SIZE - 1; i++){
    PossibleCoord[i] = i*SQUARE_SIZE;
  }
  randomSeed(analogRead(0)); //0 is unconnected - seed is different, start place is different each time
  
  STARTX = PossibleCoord[random(1,240/SQUARE_SIZE-1)];
  STARTY = PossibleCoord[random(1,240/SQUARE_SIZE-1)];
  
  SnakeX[StartPos] = STARTX;
  SnakeY[StartPos] = STARTY;
  myFillRect(SnakeX[StartPos],SnakeY[StartPos],SNAKE_COLOUR);

  //Initial length is for testing
  //Centre to right - testing
  /*
  const int LENGTH = 1;
  for(int i = 1; i < LENGTH; i++){
    SnakeX[StartPos + i] = STARTX + i*(SQUARE_SIZE);
    SnakeY[StartPos + i] = STARTY;
  }
  for(int i = 0; i < LENGTH; i++){
    myFillRect(SnakeX[StartPos + i],SnakeY[StartPos + i],SNAKE_COLOUR);
  }
  */

  Serial.println("Snake init Done");


}

void initTFT(){
  tft.begin();
  tft.setRotation(0); //upside down, for this game boy
  tft.fillScreen(BLACK);

  initSnake();

  Serial.println("TFT init done");
  
}

void UpdateCoordinateDS(int NewXCoord,int NewYCoord, bool eat){
  int NewPosition;

  if(StartPos == 0) NewPosition = MAXLENGTH - 1; //Go to the end of the array
  else NewPosition = StartPos - 1;

  SnakeX[NewPosition] = NewXCoord;
  SnakeY[NewPosition] = NewYCoord;


  //Update to new pos and increase length if the snake has eaten something,, restrict to max length
  StartPos = NewPosition;
  if(eat) Length = min(Length + 1,MAXLENGTH); 
  

}

void DebugMsg(){
  
  Serial.print("Current Pos"); Serial.print(StartPos);
  Serial.print(" End Pos:"); Serial.println( (StartPos + Length)%MAXLENGTH );
  
}

void GameOver(){
  //print Gameover and stop the program with while function  
  
  /*
  for(int i = 0; i < MAXLENGTH; i++){
    Serial.print(SnakeX[i]);
    Serial.print(" ");
  }

  Serial.println();

  for(int i = 0; i < MAXLENGTH; i++){
    Serial.print(SnakeY[i]);
    Serial.print(" ");
  }

  Serial.println();*/

  tft.setTextColor(WHITE);  

  // try this https://forum.arduino.cc/t/adafruit-oled-how-to-center-text/617181/6?
  // Get the position of cursor and save it to be used through below comments
  // Allow user to see his snake progress

  /*
  tft.setTextSize(4);
  int16_t  x1, y1;
  uint16_t w, h;
  tft.getTextBounds("GAME OVER", 0, 0, &x1, &y1, &w, &h); 
  tft.fillRect(x1,y1,w,h,BLACK);
  Serial.println(240/2 - w/2);
  Serial.println(240/2 - h/2);
  */
  tft.setTextSize(4);
  tft.setCursor(12, 94 );
  tft.print("GAME OVER");

  Length = Length - 1; //Count score based on number of apples eaten
  tft.setTextSize(2);
  tft.setCursor( 66, 130 );
  tft.print("Score: ");
  if(Score < 10){
    tft.print("0");
    tft.print(Score);
  }
  else tft.print(Score);
  
  
  while(1);
}

void GenerateApple(){
  if(!AppleEaten) return;
  bool AppleGenerated = false;
  while(!AppleGenerated){
    AppleX = PossibleCoord[random(1,240/SQUARE_SIZE - 1)]; // So its not so close to the border
    AppleY = PossibleCoord[random(1,240/SQUARE_SIZE - 1)];

    AppleGenerated = !(isSnake(AppleX,AppleY)); //Keep generating Apple until it is not in the snake

  }

  myFillRect(AppleX,AppleY,APPLE_COLOUR);
  AppleEaten = false;
  
}

bool isSnake(int xCoord, int yCoord){
  //check if coordinates is part of the snake PROBLEM
  int pos;
  for(int i = 0; i < Length + 1; i++){
    pos = (StartPos + i)%MAXLENGTH; //restrict
    if( (xCoord == SnakeX[pos]) && (yCoord == SnakeY[pos])){
      /*DebugMsg();
      Serial.print("isSNAKE: Pos: "); Serial.print(pos); 
      Serial.print(" x: "); Serial.print(xCoord);
      Serial.print(" y: "); Serial.println(yCoord);*/
      return true;
    }
  }

  return false;

}

void Move(int Direction){
  //Eat - true if the length of the snake increases
  uint8_t CurXCoord = SnakeX[StartPos];
  uint8_t CurYCoord = SnakeY[StartPos];
  bool eat = false;

  Serial.print("NEW X: ");
  Serial.print(CurXCoord);
  Serial.print(" Y:");
  Serial.print(CurYCoord);
  Serial.print(" START_POS");
  Serial.println(StartPos);
  
  int NewXCoord, NewYCoord;
  int LastXCoord, LastYCoord;

  if(!Direction) return; //0 dont move

  switch(Direction){
    case 1: //N
      NewYCoord = CurYCoord - SQUARE_SIZE;
      NewXCoord = CurXCoord;
      break;
    case 2:
      NewXCoord = CurXCoord + SQUARE_SIZE;
      NewYCoord = CurYCoord;
      break;
    case 3:
      NewYCoord = CurYCoord + SQUARE_SIZE;
      NewXCoord = CurXCoord;
      break;
    case 4:
      NewXCoord = CurXCoord - SQUARE_SIZE;
      NewYCoord = CurYCoord;
      break;
  }

  if( (NewXCoord > SCREEN_BORDER_MAX) || (NewXCoord < SCREEN_BORDER_MIN) )          GameOver();
  else if ( (NewYCoord > SCREEN_BORDER_MAX) || (NewYCoord < SCREEN_BORDER_MIN) )    GameOver();
  else if( isSnake(NewXCoord,NewYCoord) )                                           GameOver(); 
  ////Check if the NewXCoord and NewYCoord is in CoordDS --> snake hits itself
  

  //Contraint to borders - should game over for this case
  /*
  NewXCoord = min(NewXCoord,SCREEN_BORDER_MAX);
  NewXCoord = max(NewXCoord,SCREEN_BORDER_MIN);
  NewYCoord = min(NewYCoord,SCREEN_BORDER_MAX);
  NewYCoord = max(NewYCoord,SCREEN_BORDER_MIN);
  */

  //Draw new square
  myFillRect(NewXCoord,NewYCoord,SNAKE_COLOUR);

  int PreviousEnd = (StartPos + Length)%MAXLENGTH;

  LastXCoord = SnakeX[PreviousEnd];
  LastYCoord = SnakeY[PreviousEnd];

  if(NewXCoord == AppleX && NewYCoord == AppleY){
    AppleEaten = true;
    if(Length < MAXLENGTH - 1) eat = true; //Not activated if maxlength is reached
    Score++;  
  }

  UpdateCoordinateDS(NewXCoord,NewYCoord,eat); //Increase length by 1

  
  
  //Draw over last squaure if length is not extended
  if(!eat){
    myFillRect(LastXCoord,LastYCoord,BLACK);
  }

  Serial.print("END X: ");
  Serial.print(LastXCoord);
  Serial.print(" Y:");
  Serial.print(LastXCoord);
  Serial.print(" PreviousEnd:");
  Serial.println(PreviousEnd);

  

}


void setup() {

  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);

  Serial.begin(9600);
  Serial.println(F("IPS 240x240 ST7789"));

  initTFT();

  Serial.println("Setup done");

  
}

void loop() {
  GenerateApple();
  UpdateJoyStick();
  Move(SnakeDirection);

  delay(DELAY_VALUE);


}
