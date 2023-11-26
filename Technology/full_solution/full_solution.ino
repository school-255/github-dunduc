#define left_l A0
#define right_l A1

#define ena 9 // Вывод управления скоростью вращения мотора №1
#define enb 3 // Вывод управления скоростью вращения мотора №2
#define in1 7 // Вывод управления направлением вращения мотора №1
#define in2 6 // Вывод управления направлением вращения мотора №1
#define in3 5 // Вывод управления направлением вращения мотора №2
#define in4 4 // Вывод управления направлением вращения мотора №2

//мотор 1 - левый, 2 - правый

int pwm = 105;


const int white = 25;
const int black = 38;
const int delta = 2;

const int kp = 8;
const int kd = 5;
const int ki = 0.05;

int left_value, right_value;

int state = 0;
/*
0 - линия
1 - толкание кубика
2 - тащить кубик
*/

int cube_count = 0;
int cross_count = 0;

int prev_mills = 0;

void drive(int motor_num, int speed){
  if(motor_num == 1){
    analogWrite(ena, speed);

    digitalWrite(in1, speed > 0);
    digitalWrite(in2, speed < 0);
  }
  else if(motor_num == 2){
    analogWrite(enb, speed);

    digitalWrite(in3, speed < 0);
    digitalWrite(in4, speed > 0);
  }
}

void cross_skip(int dir){
  analogWrite(ena, dir * 100);
  analogWrite(enb, dir * 100);
  delay(200);
}

void cross_rotate(int dir){
  drive(1, dir * 100);
  drive(2, dir * 100);
  delay(400);

  if(dir == 1){
    while((black - delta <= left_value) && (left_value <= black + delta)){
      drive(1, 100);
      drive(2, -100);
      left_value = analogRead(left_l);
    }
  }
  else if(dir == -1){
    while((black - delta <= right_value) && (right_value <= black + delta)){
      drive(1, -100);
      drive(2, 100);
      right_value = analogRead(right_l);
    }
  }

  drive(1, 0);
  drive(2, 0);
}

bool cross_detect(){
  bool is_cross = (((black - delta) <= left_value) && 
           (left_value <= (black + delta)) && 
           ((black - delta) <= right_value) && 
           (right_value <= (black + delta)));
  if(is_cross){
    cross_count ++;
    Serial.println("cross!");
  }
  return is_cross;
}

void read_line(){
  left_value = analogRead(left_l);
  right_value = analogRead(right_l);
}

void print_info(){
  if(millis() - prev_mills >= 1000){
    prev_mills = millis();
    Serial.print("Left value: "); Serial.println(left_value);
    Serial.print("Right value: "); Serial.println(right_value);
    Serial.print("State: "); Serial.println(state);
    Serial.print("Cube count: "); Serial.println(cube_count);
    Serial.print("Cross count: "); Serial.println(cross_count);
    Serial.println("-----------------------------------------");
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(left_l, INPUT);
  pinMode(right_l, INPUT);

  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  // Команда остановки двум моторам
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

}


void loop() {

  switch(state){
    case 0://line follow
      print_info();
      read_line();

      if(cross_detect()){
        state = 1;
      }
      else{
        int err = left_value - right_value;

        int p = kp * err;

        drive(1, pwm - p);
        drive(2, pwm + p);
      } 
      print_info();
      break;

    case 1://cube push
      print_info();
      while(cross_count > 2){
        read_line();

        if(cross_detect()){
          cross_skip(1);
          cross_rotate(1);
        }
      }
      state = 2;
      print_info();
      break;

    case 2://cube pull
      print_info();
      drive(1, 0);
      drive(2, 0);
      break;

    default:
    print_info();
      break;
  }
  
  /*Serial.print(black - delta);
  Serial.print("  ");
  Serial.println(black + delta);*/

  delay(1);
}
