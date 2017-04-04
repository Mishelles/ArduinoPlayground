#define BUT_PIN 4

void buttonHandler(){
  int buttonNow=digitalRead(BUT_PIN);
  if ((buttonState == 0) && (buttonNow)){
    buttonState = 1;
    buz();
    lcd.display();
  }else if ((buttonState == 1) && (buttonNow)){
    buttonState = 0;
    lcd.noDisplay();
    buz();
    }
}
