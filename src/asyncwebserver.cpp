#include "asyncwebserver.h"
#include "messages.h"
#include "webhandler.h"

#ifdef ENABLE_SERVER

AsyncWebServer server(80);

void initWebServer()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
                                       "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.on("/", HTTP_GET, startGui);
  server.onNotFound(
      [](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Page not found!"); });

  // Route to handle
  // http://your-server/message?text=Hello&repeat=3&id=42&delay=30&graph=1,2,3,4&miny=0&maxy=15
  server.on("/api/message", HTTP_GET, handleMessage);
  server.on("/api/removemessage", HTTP_GET, handleMessageRemove);

  server.on("/api/info", HTTP_GET, handleGetInfo);

  // Handle API request to set an active plugin by ID
  server.on("/api/plugin", HTTP_PATCH, handleSetPlugin);

  // Handle API request to set the brightness (0..255);
  server.on("/api/brightness", HTTP_PATCH, handleSetBrightness);
  server.on("/api/data", HTTP_GET, handleGetData);

  // Scheduler
  server.on("/api/schedule", HTTP_POST, handleSetSchedule);
  server.on("/api/schedule/clear", HTTP_GET, handleClearSchedule);
  server.on("/api/schedule/stop", HTTP_GET, handleStopSchedule);
  server.on("/api/schedule/start", HTTP_GET, handleStartSchedule);

  server.on("/api/storage/clear", HTTP_GET, handleClearStorage);
  
    // Configuration endpoints
    server.on("/api/config", HTTP_GET, handleGetConfig);
    server.on(
        "/api/config",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {
          // Response is sent after full body is received.
        },
        nullptr,
        handleSetConfigBody);
    server.on("/api/config/reset", HTTP_POST, handleResetConfig);
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>LED Config</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}.container{background:#fff;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,.3);max-width:600px;width:100%;padding:40px}h1{color:#333;margin-bottom:10px}#msg{padding:12px;margin:15px 0;border-radius:5px;display:none;word-wrap:break-word;font-size:13px;max-height:100px;overflow-y:auto}#msg.ok{background:#d4edda;color:#155724;border:1px solid #c3e6cb}#msg.err{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}label{display:block;margin-top:15px;font-weight:600}input{width:100%;padding:10px;margin-top:5px;border:2px solid #e0e0e0;border-radius:8px}input:focus{outline:0;border-color:#667eea}button{margin-top:20px;padding:12px 20px;border:none;border-radius:8px;font-weight:600;cursor:pointer;margin-right:10px}#save{background:#667eea;color:#fff}#reset{background:#ff4757;color:#fff}.hint{font-size:11px;color:#999;margin-top:3px}</style></head><body><div class="container"><h1>⚙️ LED Config</h1><div id="msg"></div><form id="form"><label>Weather Location<input id="wl" placeholder="Hamburg"><span class="hint">City name (e.g., Turin, Milan, Hamburg)</span></label><label>NTP Server<input id="ntp" placeholder="de.pool.ntp.org"><span class="hint">Time sync server</span></label><label>Timezone<input id="tz" placeholder="CET-1CEST,M3.5.0,M10.5.0/3"><span class="hint">POSIX timezone (CET-1CEST for Italy/Germany)</span></label><button type="submit" id="save">💾 Save</button><button type="button" id="reset">⚠️ Reset</button></form></div><script>const msg=document.getElementById('msg');function show(t,ok){msg.textContent=t;msg.className=ok?'ok':'err';msg.style.display='block';console.log((ok?'✅':'❌')+' '+t);setTimeout(()=>{msg.style.display='none'},6e3)}async function load(){console.log('[Config] Loading...');try{const r=await fetch('/api/config');console.log('[Config] Response:',r.status);if(!r.ok){show('Load failed ('+r.status+')',0);return}const d=await r.json();console.log('[Config] Data:',d);document.getElementById('wl').value=d.weatherLocation||'';document.getElementById('ntp').value=d.ntpServer||'';document.getElementById('tz').value=d.tzInfo||'';show('✅ Loaded',1)}catch(e){console.error('[Config] Load error:',e);show('Load error: '+e.message,0)}}document.getElementById('form').onsubmit=async e=>{e.preventDefault();const data={weatherLocation:document.getElementById('wl').value,ntpServer:document.getElementById('ntp').value,tzInfo:document.getElementById('tz').value,autoStartSchedule:false};console.log('[Config] Saving:',data);try{const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)});console.log('[Config] Response:',r.status);if(!r.ok){const err=await r.text();console.error('[Config] Error:',err);show('Save failed: '+err,0);return}show('✅ Saved!',1);setTimeout(load,500)}catch(e){console.error('[Config] Exception:',e);show('Save error: '+e.message,0)}};document.getElementById('reset').onclick=async()=>{if(!confirm('Reset all?'))return;console.log('[Config] Resetting');try{const r=await fetch('/api/config/reset',{method:'POST'});if(!r.ok){show('Reset failed',0);return}show('✅ Reset!',1);setTimeout(load,500)}catch(e){show('Reset error: '+e.message,0)}};load()</script></body></html>
    )");
  });

  server.begin();
}

#endif
