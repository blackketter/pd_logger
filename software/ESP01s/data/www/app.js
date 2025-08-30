async function fetchLatest(){
  try{
    const r = await fetch('/api/measure/latest', { cache:'no-store' });
    if(!r.ok) throw new Error(r.statusText);
    const j = await r.json();

    // normalize voltage to volts (compat: old firmwares might log mV)
    let v = Number(j.busV);
    if (Number.isFinite(v) && v > 60) v = v / 1000.0;

    const i = Number(j.currmA);
    const p = (Number.isFinite(v) && Number.isFinite(i)) ? v * (i/1000.0) : NaN;

    document.getElementById('v').textContent = Number.isFinite(v) ? v.toFixed(3) : '—';
    document.getElementById('i').textContent = Number.isFinite(i) ? i.toFixed(0) : '—';
    document.getElementById('p').textContent = Number.isFinite(p) ? p.toFixed(3) : '—';
    const now = new Date();
    document.getElementById('info').textContent =
    'Updated at ' + now.toLocaleTimeString([], {hour: '2-digit', minute: '2-digit', second: '2-digit'});
  
  }catch(e){
    document.getElementById('info').textContent = 'No data';
  }
}

// poll roughly with sampling cadence
setInterval(fetchLatest, 5000);
fetchLatest();