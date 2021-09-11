#include <LiquidCrystal_I2C.h>
int lcdColumns = 20;
int lcdRows = 4;
String Area1_EmptyString = "          ";
String Area2_EmptyString = "          ";
String Area3_EmptyString = "                    ";

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
void Display_Handler(){
  lcd.init();
  lcd.backlight();
}

void clearArea1(){
  lcd.setCursor(0, 0);
  lcd.print(Area1_EmptyString);

}
void clearArea2(){
  lcd.setCursor(10, 0);
  lcd.print(Area2_EmptyString);

}
void clearArea3(){
  lcd.setCursor(0, 1);
  lcd.print(Area3_EmptyString);
  lcd.setCursor(0, 2);
  lcd.print(Area3_EmptyString);
  lcd.setCursor(0, 3);
  lcd.print(Area3_EmptyString);

}

void DisplayArea1 (String content ){
  clearArea1();
  if(content.length()>9){
    content = content.substring(0,9);
  }
  lcd.setCursor(0, 0);
  lcd.print(content);
}

void DisplayArea2 (String content ){
  clearArea2();
  lcd.setCursor(10, 0);
  lcd.print(Area2_EmptyString);
  if(content.length()>10){
    content = content.substring(0,9);
  }
  lcd.setCursor(0, 0);
  lcd.print(content);
}

void DisplayArea3 (String content ){
  clearArea3();

  String line1 = "";
  String line2 = "";
  String line3 = "";

  if(content.length()>60){
    line1 = content.substring(0,20);
    line2 = content.substring(20,40);
    line3 = content.substring(40,60);
  }else if (content.length()>40){
    line1 = content.substring(0,20);
    line2 = content.substring(20,40);
    line3 = content.substring(40);
  }else if (content.length()>20){
    line1 = content.substring(0,20);
    line2 = content.substring(20);
  }else
  {
    line1 = content;
  }

  lcd.setCursor(0, 1);
  lcd.print(line1);
  lcd.setCursor(0, 2);
  lcd.print(line2);
  lcd.setCursor(0, 3);
  lcd.print(line3);

}
