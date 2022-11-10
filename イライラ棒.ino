#include <EEPROM.h>
#define elements 10
String data_string; //シリアルで受け取る全文字列
char *p; //文字列をカンマで分割するstrtok処理で使うポインタ
String p_string; //上記ポインタで区切った文字列の仮格納用
String data_array[elements]; //カンマ分割されたstrデータを格納する

//ピン設定
const int reset_sw = 2; //リセットスイッチ
const int time_sw = 5; //タイムモードスイッチ
const int around_sw = 6; //周回モードスイッチ
const int L_limit = 8; //左端のセンサー
const int R_limit = 9; //右端のセンサー
const int sens = 10; //メインのセンサー
const int led = 12; //LED
const int sp = 13; //スピーカー

int time_limit = 60; //周回モードの制限時間
int penalty_interval = 50; //ペナルティのカウント間隔

int penalty_count; //ペナルティの時間計測
int start_point; //開始位置
float penalty; //ペナルティカウント
float local_time; //開始時の内部時間
float result_time; //タイムモードの結果
int count; //周回モードのカウント

void setup() {
  //入出力の設定
  pinMode(reset_sw, INPUT);
  pinMode(time_sw, INPUT);
  pinMode(around_sw, INPUT);
  pinMode(L_limit, INPUT);
  pinMode(R_limit, INPUT);
  pinMode(sens, INPUT);
  pinMode(led, OUTPUT);
  pinMode(sp, OUTPUT);

  Serial.begin(9600); //シリアルの設定
  if (EEPROM.read(0xFF) != 0) {
    EEPROM.write(0x00, 60);
    EEPROM.write(0x01, 50);
    EEPROM.write(0xFF, 0);
  }
  time_limit = EEPROM.read(0x00);
  penalty_interval = EEPROM.read(0x01);

  Serial.print("  timer ");
  Serial.print(time_limit);
  Serial.println("秒");
  Serial.print("  interval ");
  Serial.print(penalty_interval);
  Serial.println("ミリ秒");
}


void loop() {
  digitalWrite(led, HIGH);
  if (digitalRead(time_sw) == HIGH) { //タイムモードスイッチが押されたとき
    time(); //タイムモードの処理に飛ぶ
  } else if (digitalRead(around_sw) == HIGH) { //周回モードスイッチが押されたとき
    around(); //周回モードの処理に飛ぶ
  } else if ( Serial.available() ) {
    command();
  }
}


//タイムモード
void time() {
  Serial.println("タイムモード");
  if (start() == 1) { //エラーの場合最初に戻る
    return 0;
  }

  if (start_point == 0) { //左から開始
    while (digitalRead(R_limit) == HIGH) { //右に着くまでループ
      if (digitalRead(sens) == LOW && millis() - penalty_count >= penalty_interval) { //衝突時の処理
        penalty++; //ペナルティカウントの増加
        tone(sp, 440, penalty_interval + 10); //衝突音
        digitalWrite(led, LOW);
        penalty_count = millis();
      }
      digitalWrite(led, HIGH);
    }
    result_time = (millis() - local_time) / 1000 + penalty / 2; //結果の処理
    tone(sp, 880, 100); //終了音
    Serial.println(String(result_time, 5)); //結果の表示
  } else {
    while (digitalRead(L_limit) == HIGH) { //左に着くまでループ
      if (digitalRead(sens) == LOW && millis() - penalty_count >= penalty_interval) { //衝突時の処理
        penalty++; //ペナルティカウントの増加
        tone(sp, 440, penalty_interval + 10); //衝突音
        digitalWrite(led, LOW);
        penalty_count = millis();
      }
      digitalWrite(led, HIGH);
    }
    result_time = (millis() - local_time) / 1000 + penalty / 2; //結果の処理
    tone(sp, 880, 100); //終了音
    Serial.println(String(result_time, 5)); //結果の表示
  }
}


//周回モード
void around() {
  Serial.println("周回モード");
  if (start() == 1) { //エラーの場合最初に戻る
    return 0;
  }
  while (millis()  <= local_time + (time_limit - penalty) * 1000) { //時間がなくなったら終了
    if (start_point == 0) { //左から開始
      while (digitalRead(R_limit) == HIGH) { //右に着くまでループ
        if (digitalRead(sens) == LOW && millis() - penalty_count >= penalty_interval) {  //衝突時の処理
          penalty++; //ペナルティカウントの増加
          tone(sp, 440, penalty_interval + 10); //衝突音
          if (millis()  >= local_time + (time_limit - penalty) * 1000) { //時間がなくなったら終了
            break;
          }
          digitalWrite(led, LOW);
          penalty_count = millis();
        }
      }
      tone(sp, 880, 50); //終了音
      count++; //カウントの増加
      start_point = 1; //右
    } else {
      while (digitalRead(L_limit) == HIGH) { //左に着くまでループ
        if (digitalRead(sens) == LOW && millis() - penalty_count >= penalty_interval) { //衝突時の処理
          penalty++; //ペナルティカウントの増加
          tone(sp, 440, penalty_interval + 10); //衝突音
          if (millis()  >= local_time + (time_limit - penalty) * 1000) { //時間が無くなったら終了
            break;
          }
          digitalWrite(led, LOW);
          penalty_count = millis();
        }
      }
      tone(sp, 880, 50); //終了音
      count++; //カウントの増加
      start_point = 0; //左
    }
  }
  tone(sp, 880, 1000); //終了音
  Serial.println(count / 2); //結果の表示
}


//開始時の処理
int start() {
  penalty = 0; //ペナルティカウントの初期化
  count = 0; //カウントの初期化

  while (digitalRead(L_limit) == HIGH &&  digitalRead(R_limit) == HIGH) { //リミットに触れるまで待機
  }

  for (int i = 0; i < 3; i++) { //カウント音
    tone(sp, 440, 100);
    delay(900);
  }
  if (digitalRead(L_limit) == HIGH &&  digitalRead(R_limit) == HIGH) { //フライングしたらリセットする
    Serial.println("Error:フライングです");
    for (int i = 0; i < 4; i++) { //エラー音
      tone(sp, 440, 200);
      delay(200);
    }
    return 1; //最初に戻る
  }

  if (digitalRead(L_limit) == LOW) { //開始位置の保存
    start_point = 0; //左
  } else {
    start_point = 1; //右
  }
  local_time = millis(); //開始タイミングの保存
  tone(sp, 880, 100); //開始音
}


int command() {
  serial_read();
  if (data_array[0] == "help") {
    Serial.println("  help コマンド確認");
    Serial.println("  config 設定変更");
    Serial.println("  show 設定確認");
    Serial.println("  reboot 再起動");
  } else if (data_array[0] == "config") {
    if (data_array[1] == "help") {
      Serial.println("  timer タイムモードの制限時間設定(1～255秒)");
      Serial.println("  interval ペナルティのカウント間隔設定(1～255ミリ秒)");
    } else if (data_array[1] == "timer") {
      if (data_array[2].toInt() >= 1 && data_array[2].toInt() <= 255) {
        EEPROM.write(0x00, data_array[2].toInt());//EEPROMに書き込む
        time_limit = EEPROM.read(0x00);//EEPROMからの読み取り値を入れる
        Serial.print("  timer ");
        Serial.print(time_limit);
        Serial.println("秒に設定しました");
      } else {
        Serial.println("Error:引数が不正です(1～255です)");
      }
    } else if (data_array[1] == "interval") {
      if (data_array[2].toInt() >= 1 && data_array[2].toInt() <= 255) {
        EEPROM.write(0x01, data_array[2].toInt());//EEPROMに書き込む
        penalty_interval = EEPROM.read(0x01);//EEPROMからの読み取り値を入れる
        Serial.print("  interval ");
        Serial.print(penalty_interval);
        Serial.println("ミリ秒に設定しました");
      } else {
        Serial.println("Error:引数が不正です(1～255です)");
      }
    } else {
      Serial.println("Error:不正なコマンドです");
    }
  } else if (data_array[0] == "show") {
    if (data_array[1] == "help") {
      Serial.println("  config 設定の確認");
    } else if (data_array[1] == "config") {
      Serial.print("  timer ");
      Serial.print(time_limit);
      Serial.println("秒");
      Serial.print("  interval ");
      Serial.print(penalty_interval);
      Serial.println("ミリ秒");
    } else {
      Serial.println("Error:不正なコマンドです");
    }
  } else if (data_array[0] == "reboot") {
    delay(100);
    void(* resetFunc) (void) = 0;
    resetFunc();
  } else {
    if (data_array[0] != "") {
      Serial.println("Error:不正なコマンドです");
    }
  }
  Serial.print("> ");
}


//この部分ほぼコピペだからよくわからん
int serial_read() {
  for (int i = 0; i < elements; i++) { //データの初期化
    data_array[i] = "";
  }
  data_string = Serial.readStringUntil(0x0a); //シリアルデータを改行記号が現れるまで読み込む
  data_string.trim(); //文字列を念のためトリミングする
  int data_len = data_string.length() + 1; //str→char変換用にデータの長さを調べる
  char data_char[data_len]; //str→char変換用のchar配列を準備
  data_string.toCharArray(data_char, data_len); //ようやくstr→charに変換
  p = strtok(data_char, " "); //カンマ分割の1要素目を行う
  p_string = p; //一旦strにいれる
  data_array[0] = p_string; //最終目的の配列に1要素目を格納

  for (int i = 1; i < elements; i++) { //2要素名以降について、要素数分だけデータ配列に格納
    p = strtok(NULL, " "); //カンマ分割の2要素目以降のstrtokはこの書式になる
    if (p != NULL) { //要素が空でない場合はその要素をデータ配列に格納
      p_string = p;
      data_array[i] = p_string;
    } else {
      i++; //要素が空の場合はデータ配列に0を格納
    }
  }
  //データを表示する
  for (int i = 0; i < elements ; i++) {
    Serial.print(data_array[i]); //データ配列の中身を0項から最終項まで表示する
    Serial.print(" "); //カンマ区切りではなくスラッシュ区切りで表示
  }
  Serial.println();
}
