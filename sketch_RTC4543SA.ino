// Digital I/O
#define CE 2
#define WR 3
#define CLK 4
#define DATA 5

// リード・ライトbit数
#define DATA_N 52

// シーケンス定数
#define SEQ_SEC 0
#define SEQ_MIN 1
#define SEQ_HOUR 2
#define SEQ_WEEK 3
#define SEQ_DAY 4
#define SEQ_MONTH 5
#define SEQ_YEAR 6

// 初期時刻
#define INI_SEC   0x30
#define INI_MIN   0x10
#define INI_HOUR  0x15
#define INI_WEEK  0x01
#define INI_DAY   0x20
#define INI_MONTH 0x10
#define INI_YEAR  0x30
typedef unsigned char UC;

// 曜日
const char* WEEK[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

// グローバル時刻
UC year   = 0x00;
UC month  = 0x00;
UC day    = 0x00;
UC week   = 0x00;
UC hour   = 0x00;
UC minute = 0x00;
UC second = 0x00;

void setup() {

  Serial.begin(9600);

  // 入出力初期化
  pinMode(CE, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(CLK, OUTPUT);

  // 書き込み時刻をRTCに書き込み
  writeData(INI_YEAR, INI_MONTH, INI_DAY, INI_WEEK, INI_HOUR, INI_MIN, INI_SEC);
}

void loop() {
    delay(1000);
  readData();
  Serial.print("20");   Serial.print(cnvBCDtoBYTE(year)); Serial.print("年");
  Serial.print(cnvBCDtoBYTE(month));  Serial.print("月");
  Serial.print(cnvBCDtoBYTE(day));    Serial.print("日(");
  Serial.print(WEEK[week]);   Serial.print(") ");
  Serial.print(cnvBCDtoBYTE(hour));   Serial.print("時");
  Serial.print(cnvBCDtoBYTE(minute)); Serial.print("分");
  Serial.print(cnvBCDtoBYTE(second)); Serial.println("秒");

}

// データライト(引数はBCD)
void writeData(UC year, UC month, UC day, UC week, UC hour, UC minute, UC second){
  UC snd_year    = year; 
  UC snd_month   = month   & 0x1F;
  UC snd_day     = day     & 0x3F;
  UC snd_week    = week    & 0x07;
  UC snd_hour    = hour    & 0x3F;
  UC snd_minute  = minute  & 0x7F;
  UC snd_second  = second  & 0x7F;
  UC seq = SEQ_SEC;
  UC pos = 0;
  
  // Write出力初期化
  pinMode(DATA, OUTPUT);
  digitalWrite(CLK, LOW);
  digitalWrite(WR, HIGH); delay(1);
  digitalWrite(CE, HIGH); delay(1);

  // Write開始
  while(1){
    switch(seq){
      case SEQ_SEC:   writeBit(snd_second, pos);break;
      case SEQ_MIN:   writeBit(snd_minute, pos);break;
      case SEQ_HOUR:  writeBit(snd_hour, pos);  break;
      case SEQ_WEEK:  writeBit(snd_week, pos);  break;
      case SEQ_DAY:   writeBit(snd_day, pos);   break;
      case SEQ_MONTH: writeBit(snd_month, pos); break;
      case SEQ_YEAR:  writeBit(snd_year, pos);  break;
    }

    pos++;
    if((seq == SEQ_WEEK && pos == 4) || (pos == 8)){
      seq = seq + 1;
      pos = 0;
    }

    if(seq == SEQ_YEAR + 1) break;
  }

  digitalWrite(WR, LOW); delay(1);
  digitalWrite(CE, LOW); delay(1);
}

void writeBit(UC data, UC pos){
  if(data & (0x01 << pos))   digitalWrite(DATA, HIGH);
  else                       digitalWrite(DATA, LOW);
  delay(1);
  digitalWrite(CLK,HIGH); delay(1);
  digitalWrite(CLK,LOW);  delay(1);
}

void readData(){
  UC seq = SEQ_SEC;
  UC pos = 0;
  UC rcv, calc;

  initTime();
  
  // Read出力初期化
  digitalWrite(CLK, LOW);
  digitalWrite(WR, LOW);  delay(1);
  digitalWrite(CE, HIGH); delay(1); 
  pinMode(DATA, INPUT);
  
  // Read開始
  while(1){
    rcv = readBit() << pos;
    switch(seq){
      case SEQ_SEC:   second += rcv;  break;
      case SEQ_MIN:   minute += rcv;  break;
      case SEQ_HOUR:  hour += rcv;    break;
      case SEQ_WEEK:  week += rcv;    break;
      case SEQ_DAY:   day += rcv;     break;
      case SEQ_MONTH: month += rcv;   break;
      case SEQ_YEAR:  year += rcv;    break;
    }
    pos++;
    if((seq == SEQ_WEEK && pos == 4) || (pos == 8)){
      seq = seq + 1;
      pos = 0;
    }

    if(seq == SEQ_YEAR + 1) break;
  }

  digitalWrite(WR, LOW); delay(1);
  digitalWrite(CE, LOW); delay(1);
  pinMode(DATA, OUTPUT);
}

UC readBit(){
  UC ret;
  digitalWrite(CLK, HIGH); delay(1);
  if(digitalRead(DATA) == HIGH) ret = 0x01;
  else                          ret = 0x00;
  digitalWrite(CLK, LOW);  delay(1);
  return ret;
}

UC cnvBCDtoBYTE(UC BCD){
  return ((((BCD & 0xF0) >> 4) * 10) + (BCD & 0x0F));
}

void initTime(){
  second = 0; minute = 0; hour = 0;
  week = 0; day = 0;  month = 0;  year = 0;
}
