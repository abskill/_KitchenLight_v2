

// MQTT // -----------------------------------------------------------------------------

void mqtt_call() {

  delay(3); // Крайне важная задержка в обеспечение wifi

  // подключаемся к MQTT серверу
  if (WiFi.status() == WL_CONNECTED) //  &&  millis() - mqtt_timer > mqtt_refresh_time)
  {
    //mqtt_timer = millis(); // сброс таймера
    if (!client.connected() )
    {
      online_cr = true; // После подключения выдать статус online

      //if ((millis() - timer_of_trying_to_connect_mqtt) > delay_of_trying_to_connect_mqtt)
      //{
      //  timer_of_trying_to_connect_mqtt = millis();

      if (debug) Serial.print("Connecting to MQTT server " + String(mqtt_server) + "...");

      client.setServer(mqtt_server, mqtt_port  );
      client.setCallback(callback);

      //if (client.connect(MQTT::Connect(mqtt_unique_client_id).set_auth(mqtt_user, mqtt_pass)))
      //if (client.connect(mqtt_unique_client_id))  // connect without willTopic

      if (client.connect(mqtt_unique_client_id, "wm:online", 1, true, String("0").c_str())) // connect with willTopic (топик статуса данного клиента и сообщение на случай потери связи с брокером)
      {
        if (debug == 1) Serial.println("ok");
        //mqtt_err_counter = 0; // сброс счетчика ошибок в случае успешного подключения

        //client.set_callback(callback);


        // подписываемся под топики
        /*
          client.subscribe("led_state");
          client.subscribe("mode_auto");
          client.subscribe("sensor_command");
          client.subscribe("low_light");
          client.subscribe("btn_value");
          //client.subscribe("vcc");
          client.subscribe("d_time");
        */
        //client.subscribe("kl:OTA_on");
        //client.subscribe("kl:MQTT_on");
        client.subscribe("kl:btn_pressed");
        client.subscribe("kl:br_target");

        client.subscribe("kl:up_time_in", 1);
        client.subscribe("kl:down_time_in", 1);
        client.subscribe("kl:up_time_weekend_in", 1);
        client.subscribe("kl:down_time_weekend_in", 1);
        client.subscribe("kl:time_command_in", 1);

      }
      else
      {
        if (debug == 1) Serial.println("fail");
        mqtt_err_counter++;

        error_status = "MQTT error #" + String(mqtt_err_counter);
        error_status_cr = true;

        if (mqtt_err_counter == mqtt_number_of_trying)
        {
          delay_of_trying_to_connect_mqtt *= 10;
          if (debug) Serial.println("!_Increase interval to connect MQTT server_!");
        }
      }
      //}
    }

    if (client.connected()) {
      //if (debug) Serial.println("------client.connected()");

      client.loop();
      refreshData();
    }
  }
}



// MQTT: Функция отправки показаний // -------------------------------------------------

void refreshData() {
  //if (debug) Serial.println("------------refreshData");

  //bool retain = true;

  //if (mqtt_sending_request) {

  if (online_cr) {
    client.publish("kl:online", String("1").c_str(), true); // Сообщаем в топик willTopic, что связь есть
    online_cr = false;
  }

  if (time_command_cr) {
    client.publish("kl:time_command_out", String(time_command).c_str(), true);
    time_command_cr = false;
  }

  if (up_time_cr) {
    //client.publish("hh_up", String(hh_up).c_str());
    //client.publish("mm_up", String(mm_up).c_str());
    String up_time_out = time2string(hh_up, mm_up);
    client.publish("kl:up_time_out", up_time_out.c_str(), true);
    up_time_cr = false;
  }

  if (down_time_cr) {
    //client.publish("hh_down", String(hh_down).c_str());
    //client.publish("mm_down", String(mm_down).c_str());
    String down_time_out = time2string(hh_down, mm_down);
    client.publish("kl:down_time_out", down_time_out.c_str(), true);
    down_time_cr = false;
  }

  if (up_time_weekend_cr) {
    //client.publish("hh_up", String(hh_up).c_str());
    //client.publish("mm_up", String(mm_up).c_str());
    String up_time_weekend_out = time2string(hh_up_weekend, mm_up_weekend);
    client.publish("kl:up_time_weekend_out", up_time_weekend_out.c_str(), true);
    up_time_weekend_cr = false;
  }

  if (down_time_weekend_cr) {
    //client.publish("hh_down", String(hh_down).c_str());
    //client.publish("mm_down", String(mm_down).c_str());
    String down_time_weekend_out = time2string(hh_down_weekend, mm_down_weekend);
    client.publish("kl:down_time_weekend_out", down_time_weekend_out.c_str(), true);
    down_time_weekend_cr = false;
  }

  if (current_status_cr) {
    client.publish("kl:current_status", current_status.c_str(), true);
    current_status_cr = false;
  }

  if (error_status_cr) {
    client.publish("kl:error_status", error_status.c_str(), true);
    error_status_cr = false;
  }

  mqtt_sending_request = false;
  //}

  delay(1);
}




String time2string(byte HH, byte MM) {
  String time_string = "";
  if (HH < 10) time_string += "0";
  time_string += String(HH) + ":";
  if (MM < 10) time_string += "0";
  time_string += String (MM);

  return time_string;
}





// MQTT: Функция получения данных от сервера // -----------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  if (debug) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  }


  String payload_string = "";

  for (int i = 0; i < length; i++) {
    //if (debug) Serial.print((char)payload[i]);
    payload_string.concat((char)payload[i]);
  }
  if (debug) Serial.println(String(payload_string));

  payload_string = (String)payload_string;

  // проверяем из нужного ли нам топика пришли данные
  if (String(topic) == "kl:btn_pressed") btn_pressed_A_mqtt = payload_string.toInt();
  if (String(topic) == "kl:br_target") br_target = payload_string.toInt();

  //if (String(topic) == "OTA_on") OTA_on = payload_string.toInt();
  //if (String(topic) == "MQTT_on") MQTT_on = payload_string.toInt();
  /*  if (String(topic) == "wm:motor_rotate") motor_rotate = payload_string.toInt();
    if (String(topic) == "wm:motor_go_up") motor_go_up = payload_string.toInt();
    if (String(topic) == "wm:motor_go_down") motor_go_down = payload_string.toInt();
  */
  //if (String(topic) == "hh_up_from_outside") hh_up_from_outside = payload_string.toInt();
  //if (String(topic) == "mm_up_from_outside") mm_up_from_outside = payload_string.toInt();
  //if (String(topic) == "hh_down_from_outside") hh_down_from_outside = payload_string.toInt();
  //if (String(topic) == "mm_down_from_outside") mm_down_from_outside = payload_string.toInt();
  /*
    if (String(topic) == "wm:motor_speed_in") {
      motor_speed_in = payload_string.toInt();
      change_time = true;
    }
    if (String(topic) == "wm:calibrate_command") {
      calibrate_command = payload_string.toInt();
      change_time = true;
    }
  */

  if (String(topic) == "kl:up_time_in") {
    up_time_in = payload_string;  // эти данные принимаются типом стринг
    change_time = true;
  }
  if (String(topic) == "kl:down_time_in") {
    down_time_in = payload_string;  // эти данные принимаются типом стринг
    change_time = true;
  }
  if (String(topic) == "kl:up_time_weekend_in") {
    up_time_weekend_in = payload_string;  // эти данные принимаются типом стринг
    change_time = true;
  }
  if (String(topic) == "kl:down_time_weekend_in") {
    down_time_weekend_in = payload_string;  // эти данные принимаются типом стринг
    change_time = true;
  }
  if (String(topic) == "kl:time_command_in") {
    time_command_in = payload_string.toInt();
    change_time = true;
  }


  //  if ( (String(topic) == "wm:time_command_in") or  (String(topic) == "wm:up_time_in") or (String(topic) == "wm:down_time_in") or (String(topic) == "wm:up_time_weekend_in") or (String(topic) == "wm:down_time_weekend_in") ) change_time = true;


}
