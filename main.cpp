#include <Wire.h>
#include <LiquidCrystal_I2C.h>
const int triggerPin = 3;     // HC-SR04觸發引腳
const int echoPin = 4;        // HC-SR04回音引腳
const int buzzerPin = 5;
const int yellowLedPin = 6;
const int redLedPin = 7;
const int lcdAddr = 0x27;
const int cols = 16;
const int rows = 2;

LiquidCrystal_I2C lcd(lcdAddr, cols, rows);

int failureCount = 0;
int completedRuns = 0;
bool countdownActive = false;
bool testFinished = true;  // 新增測試是否完成的狀態
bool rundetect = true;


void setup() {
  Serial.begin(9600);  // 設定通信速率為 9600 bps
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  lcd.begin();
  lcd.backlight();
}
void loop() {
  if (!countdownActive && testFinished) {
    // 在測試完成且等待按鈕時進入下一輪測試
    resetTest();
    countdownActive = true;
    lcd.clear();
    lcd.print("start!");
    delay(3000);  // 3秒倒數
    runTest();
  }
  // 檢查是否有 Serial 資料可用
  if (Serial.available() > 0) {
    char incomingChar = Serial.read();

    // 如果接收到 "start"，則重新開始測試
    if (incomingChar == 'S' || incomingChar == 's') {
      resetTest();
      countdownActive = true;
      lcd.clear();
      lcd.print("start!");
      delay(3000);  // 3秒倒數
      runTest();
    }
  }
}

void runTest() {
  while (failureCount < 2) {
    float runTime = calculateRunTime(completedRuns);
    float remainingTime = runTime;

    int distance = pulseIn(echoPin, HIGH, 30000) * 0.034 / 2;
    Serial.print("Initial Distance: ");
    Serial.println(distance);

    completedRuns += 2;

    lcd.clear();
    lcd.print("run: ");
    lcd.print(completedRuns);
    lcd.setCursor(0, 1);
    lcd.print("time: ");
    lcd.print(runTime, 1);
    lcd.print("s");

    digitalWrite(triggerPin, LOW);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // 一次性讀取距離
    // int distance = pulseIn(echoPin, HIGH) * 0.034 / 2;

    while (true) {
      digitalWrite(triggerPin, LOW);
      delayMicroseconds(2);  // 至少保持低電平2微秒
      digitalWrite(triggerPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(triggerPin, LOW);

      int distance = pulseIn(echoPin, HIGH) * 0.034 / 2;



      if (remainingTime <= 0) {
        rundetect = false;
        break;
      }

      lcd.clear();
      lcd.print("run: ");
      lcd.print(completedRuns);
      lcd.setCursor(0, 1);
      lcd.print("time: ");
      lcd.print(remainingTime, 1);
      lcd.print("s");

      remainingTime -= 0.1;
      delay(100);

      if (distance < 50) {
        // 距離小於等於50公分時跳出循環
        lcd.clear();
        break;
      }
    }

    if (rundetect == true) {
      lcd.clear();
      lcd.print("OK!");
      delay(3000);
      countdownActive = false;
      testFinished = false;
      rundetect = true;
    } else {
      lcd.clear();
      lcd.print("define!");
      failureCount++;
      delay(3000);
      rundetect = true;
      handleFailure();
    }


    Serial.print("Current Distance: ");
    Serial.println(distance);
  }


  lcd.clear();
  lcd.print("finsh");
  lcd.setCursor(0, 1);
  lcd.print("run: ");
  lcd.print(completedRuns);
}

float calculateRunTime(float runNumber) {
  if (runNumber < 7) {
    return 9;
  } else {
    float calculatedTime = 9 - 0.5 * (runNumber - 6);




    // 增加時間下限，當秒數小於5，將不再減少
    return (calculatedTime < 5) ? 5 : calculatedTime;
  }
}

void handleFailure() {
  if (failureCount == 1) {
    digitalWrite(yellowLedPin, HIGH);
    tone(buzzerPin, 1000);  // 頻率1000Hz
    delay(2000);  // 2秒
    noTone(buzzerPin);
    // digitalWrite(yellowLedPin, LOW);
  } else if (failureCount == 2) {
    digitalWrite(redLedPin, HIGH);
    tone(buzzerPin, 2000);  // 頻率2000Hz
    delay(2000);  // 2秒
    noTone(buzzerPin);
    // digitalWrite(redLedPin, LOW);
    lcd.clear();
    lcd.print("define!");
  }
}

void resetTest() {
  // 在這裡重置相關變數，以進入下一輪測試
  completedRuns = 0;
  failureCount = 0;
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  lcd.clear();
}


