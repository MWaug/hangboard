// Serving a web page (from flash memory)
// formatted as a string literal!
char webpage[] PROGMEM = R"=====(
<html>
<body onload="javascript:init()">
<div>
  <h1 id="currentHang">Current Hang: None</h1>
  <h1 id="lastHang">Last Hang: None</h1>
  <h1 id="maxWeight">Max Weight: None</h1>
  <h1 id="aveWeight">Ave Weight: None</h1>
  <h1 id="curWeight">Current Weight: None</h1>
</div>
<!-- Add a dropdown for controlling the workout type -->
<select id="rung" oninput="sendDeviceData()">
  <option value="largest-edge" data-number="40">Top Edge</option>
  <option value="middle-edge" data-number="23">Middle Edge</option>
  <option value="smallest-edge" data-number="15">Small Edge</option>
</select>
<!-- Add a dropdown for controlling the person working out -->
<select id="person" oninput="sendDeviceData()">
  <option value="Caroline" data-number="caroline">Caroline</option>
  <option value="Max" data-number="max">Max</option>
  <option value="Other" data-number="other">Other</option>
</select>
<!-- Add a button to turn off data logging -->
<label for="dataLogEnable">Data logging:</label>
<select id="dataLogEnable" oninput="sendDeviceData()">
  <option value="1" data-number="1">Enabled</option>
  <option value="0" data-number="0">Disabled</option>
</select>
<!-- Add a field for additional notes -->
<hr />
<label for="note">Additional Note - NO SPACES or SPECIAL CHARACTERS:</label>
<input type="text" id="note" name="note" oninput="sendDeviceData()" />
<hr />
<!-- Adding a slider for controlling scale calibration slope -->
<div>
  <input type="range" min="9000" max="18000" value="15600" id="calibSlider" oninput="sendDeviceData()" />
  <label for="calibSlider" id="calibSliderLabel">Scale Units Per LB: 15600</label>
</div>
<!-- Debugging section -->
<div>
  <p id="debugMsg">None</p>
</div>

<!-- Adding a websocket to the client (webpage) -->
<script>
  var webSocket;
  function init() {
    webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
    webSocket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      var today = new Date();
      var t = today.getMinutes() + ":" + today.getSeconds();
      document.getElementById("currentHang").innerHTML = "Current Hang: " + data.ch + "s";
      document.getElementById("lastHang").innerHTML = "Last Hang: " + data.lh + "s";
      document.getElementById("maxWeight").innerHTML = "Max Weight: " + data.max + "lb";
      document.getElementById("aveWeight").innerHTML = "Ave Weight: " + data.ave + "lb";
      document.getElementById("curWeight").innerHTML = "Weight: " + data.w + "lb";
    }
  }
  function sendDeviceData(){
    var msg_obj = new Object();
    msg_obj.calib = document.getElementById("calibSlider").value;
    msg_obj.rung = document.getElementById("rung").value;
    msg_obj.person = document.getElementById("person").value;
    msg_obj.dataLogEnable = document.getElementById("dataLogEnable").value;
    msg_obj.note = document.getElementById("note").value;
    document.getElementById("calibSliderLabel").innerHTML = "Scale Units Per LB: " + msg_obj.calib;
    var msg = JSON.stringify(msg_obj);
    document.getElementById("debugMsg").innerHTML = msg;
    webSocket.send(msg);
  }
</script>
</body>
</html>
)=====";