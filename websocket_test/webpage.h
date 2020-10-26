// Serving a web page (from flash memory)
// formatted as a string literal!
char webpage[] PROGMEM = R"=====(
<html>
<div>
  <h1 id="currentHang">Current Hang: None</h1>
  <h1 id="lastHang">Last Hang: None</h1>
  <h1 id="maxWeight">Max Weight: None</h1>
  <h1 id="aveWeight">Ave Weight: None</h1>
</div>
<!-- Adding a data chart using Chart.js -->
<head>
  <script src='https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.5.0/Chart.min.js'></script>
</head>
<body onload="javascript:init()">
<!-- Adding a slider for controlling scale calibration slope -->
<div>
  <input type="range" min="9000" max="18000" value="15600" id="calibSlider" oninput="sendCalib()" />
  <label for="calibSlider" id="calibSliderLabel">Scale Units Per LB: 15600</label>
</div>
<hr />
<div>
  <canvas id="line-chart" width="800" height="450"></canvas>
</div>

<!-- Adding a websocket to the client (webpage) -->
<script>
  var webSocket, dataPlot;
  var maxDataPoints = 20;
  function removeData(){
    dataPlot.data.labels.shift();
    dataPlot.data.datasets[0].data.shift();
  }
  function addData(label, data) {
    if(dataPlot.data.labels.length > maxDataPoints) removeData();
    dataPlot.data.labels.push(label);
    dataPlot.data.datasets[0].data.push(data);
    dataPlot.update();
  }
  function init() {
    webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
    dataPlot = new Chart(document.getElementById("line-chart"), {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          data: [],
          label: "Weight (lbs)",
          borderColor: "#3e95cd",
          fill: false
        }]
      },
      options: {
        scales: {
            yAxes: [{
                ticks: {
                    suggestedMin: -10,
                    suggestedMax: 250
                }
            }]
        }
      }
    });
    webSocket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      var today = new Date();
      var t = today.getMinutes() + ":" + today.getSeconds();
      addData(t, data.value);
      document.getElementById("currentHang").innerHTML = "Current Hang: " + data.ch + "s";
      document.getElementById("lastHang").innerHTML = "Last Hang: " + data.lh + "s";
      document.getElementById("maxWeight").innerHTML = "Max Weight: " + data.max + "lb";
      document.getElementById("aveWeight").innerHTML = "Ave Weight: " + data.ave + "lb";
    }
  }
  function sendCalib(){
    var calib = document.getElementById("calibSlider").value;
    webSocket.send(calib);
    document.getElementById("calibSliderLabel").innerHTML = "Scale Units Per LB: " + calib;
  }
</script>
</body>
</html>
)=====";
