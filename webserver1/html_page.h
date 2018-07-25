static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html><head><title>ESP8266</title><style>
body {}
#intLedState {background: red;}
#intLedState.on {background: green;}
#intLedState.half {background: yellow;}
</style>
<script>
    var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
    
    function sendXMLData(data, cb) {
        var req_ = new XMLHttpRequest();
        req_.onreadystatechange = function() {
            if(req_.readyState === 4 && req_.status === 200 && typeof cb === "function") {
                var res_ = req_.responseText;
                document.getElementById("output").innerHTML = "response: " + res_;
                res_ = JSON.parse(res_);
                cb(res_);
            }
        };
        req_.open("GET", "/submit?button=" + data.button + "&state=" + data.state + "&rand=" + Date.now(), false);
        req_.send();
        //req_.send(JSON.stringify(data));
    }
    
    function sendData(api) {ws.send(api);};
    
    ws.onmessage = function(evt) {
          //console.log(evt);
        document.getElementById("output").innerHTML = "response: " + evt.data;
        if (evt.data === 'internal_led:on') document.getElementById('intLedState').className = 'on';
        else if (evt.data === 'internal_led:off') document.getElementById('intLedState').className = 'off';
        else if (evt.data === 'internal_led:half') document.getElementById('intLedState').className = 'half';
        else {console.log('unknown event');}
    };
</script>
</head><body>
<h1>Test some stuff with my esp8266</h1>
<p>Internal LED
    <button onclick="sendData('internal_led:on:set')">ON</button>&nbsp;
    <button onclick="sendData('internal_led:off:set')">OFF</button>&nbsp;
    <button onclick="sendData('internal_led:flash50:set')">HALF</button>
    <span id='intLedState'>state</span>
 </p>
 <br/><span id='output'></span>

</body>
</html>
)rawliteral";

static const char PROGMEM NOTFOUND_HTML[] = R"rawliteral(<h1>Ooops</h1><p>Someone destroyed the page you are looking for</p>)rawliteral";
