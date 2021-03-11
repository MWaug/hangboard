/*
 * Hangboard logic 
 */
enum state {
  IN_HANG,
  ON_GROUND
};

unsigned long lastMeasuredTime = 0;
enum state cur_state = ON_GROUND;

/* 
 *  FSM update logic
 */
float tick_hangboard() {
  float weight = read_hx711_weight();
  cur_weight = weight;
  if( (cur_state == ON_GROUND) && (weight > hang_threshold) ) {
      StartHang = millis();
      Serial.println("Hanging!");
      cur_state = IN_HANG;
      maxWeight = 0;
      integratedWeight = 0;
      aveWeight = 0;
    }
  // Calculate hang statistics
  if ( cur_state == IN_HANG ) {
    if( weight > maxWeight ) { maxWeight = weight; }
    integratedWeight = integratedWeight + ( weight * (millis() - lastMeasuredTime) );
    aveWeight = integratedWeight / (millis() - StartHang);
    cur_hang_secs = (millis() - StartHang) / 1000.0;
  }
  if( (cur_state == IN_HANG) && weight < hang_threshold - 5) {
      EndHang = millis();
      unsigned long hang_dur = EndHang - StartHang;
      float hang_dur_secs = hang_dur / 1000.0;
      Serial.println("On the ground");
      Serial.println(hang_dur_secs);
      last_hang_secs = hang_dur_secs;
      cur_hang_secs = 0;
      cur_state = ON_GROUND;
      if(dataLogEnable == 1) {
        postData(); // Send data to google sheet
      }
  }
  lastMeasuredTime = millis();
  return weight;
}
