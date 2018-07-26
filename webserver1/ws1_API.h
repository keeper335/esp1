
// pin d0 -> GPIO16
// pin d1 -> GPIO5
// pin d2 -> GPIO4
// pin d3 -> GPIO0
// pin d4 -> GPIO2

static int pin_states[16] = {0};

void handleWebSocketAPIGetState(JsonObject& obj) {
  int state_;
  int run_[] = {5,4,0,2};

  for (int i: run_) {
    state_ = pin_states[i];
    if (state_ == 0)
      obj["pin_d" + String(i)] = false;
    else if (state_ == 1024)
      obj["pin_d" + String(i)] = true;
    else if (state_ > 0 && state_ < 1024)
      obj["pin_a" + String(i)] = state_;
  }
}

void handleWebSocketAPI(const char* data, char *backMsg_) {
  StaticJsonBuffer<128> json1, json2;
  JsonObject& evt = json1.parseObject(data);
  JsonObject& backEvt = json2.createObject();
  if (!evt.success()) {
    Serial.println("Failed to parse event");
    return;
  }

  String id = evt["id"];
  String backMsg_id_ = "";
  JsonVariant pin_num = evt["pinId"];
  JsonVariant state = evt["state"];
  
  if (id.equals("pin_set")) {
    int dpin_num = pin_num.as<int>();
    String state_ = state.as<String>();
    if (dpin_num >= 0) {
        backMsg_id_ = "pin_d" + String(dpin_num);
  
        if (state_.equals("ON")) {
          digitalWrite(dpin_num, LOW); //on
          pin_states[dpin_num] = 1024;
          backEvt[backMsg_id_] = true;
        }
        else if (state_.equals("OFF")) {
          digitalWrite(dpin_num, HIGH); //off
          pin_states[dpin_num] = 0;
          backEvt[backMsg_id_] = false;
        }
        else {
          Serial.println("Unexpected _state_ type");
        }
    }
    else {
      Serial.print("Unexpected pin number ");
      pin_num.printTo(Serial);
      Serial.println("");
    }
  }
  else if (id.equals("pin_pwm")) {
    int dpin_num = pin_num.as<int>();
    int state_ = state.as<int>();
    if (dpin_num >= 0) {
      backMsg_id_ = "pin_a" + String(dpin_num);
      if (state_ >= 0 && state_ <= 1023) {
        analogWrite(dpin_num, state_);
        pin_states[dpin_num] = state_;
        backEvt[backMsg_id_] = state_;
      }
      else {
        Serial.println("Unexpected _state_ type");
      }
    }
    else {
      Serial.print("Unexpected pin number ");
      pin_num.printTo(Serial);
      Serial.println("");
    }
  }
  else if (id.equals("get_state")) {
    Serial.println("Read pins");
    handleWebSocketAPIGetState(backEvt);
  }
  else {
    Serial.println("Event not handled: " + id);
  }

  Serial.print("To client: ");
  backEvt.printTo(Serial);
  Serial.println("");
  backEvt.printTo(backMsg_, 128);
}

