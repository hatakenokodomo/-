//ピン設定
const int reset_sw = 2; //リセットスイッチ
const int time_sw = 5; //タイムモードスイッチ
const int around_sw = 6; //周回モードスイッチ
const int L_limit = 8; //左端のセンサー
const int R_limit = 9; //右端のセンサー
const int sens = 10; //メインのセンサー
const int sp = 13; //スピーカー

const int time_limit = 60; //周回モードの制限時間

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
  pinMode(sp, OUTPUT);

  Serial.begin(9600); //シリアルの設定
}


void loop() {
  if (digitalRead(time_sw) == HIGH) { //タイムモードスイッチが押されたとき
    time(); //タイムモードの処理に飛ぶ
  } else if (digitalRead(around_sw) == HIGH) { //周回モードスイッチが押されたとき
    around(); //周回モードの処理に飛ぶ
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
      if (digitalRead(sens) == LOW) { //衝突時の処理
        penalty++; //ペナルティカウントの増加
        tone(sp, 440, 50); //衝突音
        delay(30);
      }
    }
    result_time = (millis() - local_time) / 1000 + penalty / 2; //結果の処理
    tone(sp, 880, 100); //終了音
    Serial.println(String(result_time, 2)); //結果の表示
  } else {
    while (digitalRead(L_limit) == HIGH) { //左に着くまでループ
      if (digitalRead(sens) == LOW) { //衝突時の処理
        penalty++; //ペナルティカウントの増加
        tone(sp, 440, 50); //衝突音
        delay(30);
      }
    }
    result_time = (millis() - local_time) / 1000 + penalty / 2; //結果の処理
    tone(sp, 880, 100); //終了音
    Serial.println(String(result_time, 2)); //結果の表示
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
        if (digitalRead(sens) == LOW) {  //衝突時の処理
          penalty++; //ペナルティカウントの増加
          tone(sp, 440, 60); //衝突音
          if (millis()  >= local_time + (time_limit - penalty) * 1000) { //時間がなくなったら終了
            break;
          }
          delay(50);
        }
      }
      tone(sp, 880, 50); //終了音
      count++; //カウントの増加
      start_point = 1; //右
    } else {
      while (digitalRead(L_limit) == HIGH) { //左に着くまでループ
        if (digitalRead(sens) == LOW) { //衝突時の処理
          penalty++; //ペナルティカウントの増加
          tone(sp, 440, 60); //衝突音
          if (millis()  >= local_time + (time_limit - penalty) * 1000) { //時間が無くなったら終了
            break;
          }
          delay(50);
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
