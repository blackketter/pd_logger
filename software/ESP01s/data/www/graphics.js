function $(sel){ return document.querySelector(sel); }

function parseCSV(text){
  const lines = text.trim().split(/\n+/);
  if(!lines.length) return [];
  const out = [];
  for(let i=1;i<lines.length;i++){
    const L = lines[i].trim();
    if(!L) continue;
    const [e, v, iA] = L.split(';');
    const epoch = Number(e);
    if(!Number.isFinite(epoch)) continue;

    // bus_V can be in V or mV (older/newer firmware); normalize to V
    let bus = Number(v);
    if (!Number.isFinite(bus)) continue;
    if (bus > 60) bus = bus / 1000.0; // assume mV -> V

    const currmA = Number(iA);
    if (!Number.isFinite(currmA)) continue;

    const powerW = bus * (currmA / 1000.0);

    out.push({ t: epoch*1000, v: bus, i: currmA, p: powerW });
  }
  out.sort((a,b)=>a.t-b.t);
  return out;
}

function niceNum(n, decimals){
  return Number.isFinite(n) ? n.toFixed(decimals) : '—';
}

function drawSeries(canvas, data, key, color, yUnit){
  const cw = canvas.clientWidth, ch = canvas.clientHeight;
  if(canvas.width !== cw) canvas.width = cw;
  if(canvas.height !== ch) canvas.height = ch;
  const ctx = canvas.getContext('2d');
  ctx.clearRect(0,0,cw,ch);

  if(!data.length){
    ctx.fillStyle = '#666';
    ctx.font = '14px system-ui, sans-serif';
    ctx.fillText('No data', 12, 22);
    return;
  }

  const left = 52, right = 8, top = 8, bottom = 24;
  const W = cw - left - right;
  const H = ch - top - bottom;

  const xs = data[0].t, xe = data[data.length-1].t;
  let ymin = Infinity, ymax = -Infinity;
  for(const d of data){
    const y = d[key];
    if(!Number.isFinite(y)) continue;
    if(y < ymin) ymin = y;
    if(y > ymax) ymax = y;
  }
  if(!Number.isFinite(ymin) || !Number.isFinite(ymax) || ymin === ymax){
    ymin = (Number.isFinite(ymin) ? ymin : 0) - 1;
    ymax = (Number.isFinite(ymax) ? ymax : 0) + 1;
  }
  const pad = (ymax - ymin) * 0.08;
  ymin -= pad; ymax += pad;

  const xmap = t => left + ( (t - xs) / (xe - xs || 1) ) * W;
  const ymap = y => top + H - ((y - ymin) / (ymax - ymin || 1)) * H;

  // axes
  ctx.strokeStyle = '#ddd'; ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(left, top); ctx.lineTo(left, top+H); ctx.lineTo(left+W, top+H);
  ctx.stroke();

  // grid
  ctx.strokeStyle = '#eee';
  for(let g=0; g<=5; g++){
    const yy = top + (H*g/5);
    ctx.beginPath(); ctx.moveTo(left, yy); ctx.lineTo(left+W, yy); ctx.stroke();
  }

  // line
  ctx.strokeStyle = color; ctx.lineWidth = 1.5; ctx.beginPath();
  let started = false;
  for(const d of data){
    const x = xmap(d.t), y = ymap(d[key]);
    if(!started){ ctx.moveTo(x,y); started = true; } else { ctx.lineTo(x,y); }
  }
  ctx.stroke();

  // Y labels
  ctx.fillStyle = '#666';
  ctx.font = '12px system-ui, sans-serif';
  for(let g=0; g<=5; g++){
    const yv = ymin + (ymax - ymin) * (1 - g/5);
    const yy = top + (H*g/5);
    let decimals = 0;
    if (key === 'v' || key === 'p') decimals = 3;
    const label = niceNum(yv, decimals) + ' ' + yUnit;
    ctx.fillText(label, 6, yy + 4);
  }

  // X labels (start/end)
  const t0 = new Date(xs), t1 = new Date(xe);
  ctx.textAlign = 'left';
  ctx.fillText(t0.toLocaleTimeString([], {hour12:false}), left, top+H+18);
  ctx.textAlign = 'right';
  ctx.fillText(t1.toLocaleTimeString([], {hour12:false}), left+W, top+H+18);
  ctx.textAlign = 'start';
}

async function loadRange(sec){
  const qs = sec === 'max' ? 'sec=max' : ('sec=' + String(sec|0));
  const res = await fetch('/api/logs/range?' + qs, { cache:'no-store' });
  if(!res.ok){
    $('#info').textContent = 'No data (' + res.status + ')';
    return;
  }
  const txt = await res.text();
  const rows = parseCSV(txt);

  // meta
  const span = rows.length ? ( (rows[rows.length-1].t - rows[0].t) / 1000 ) : 0;
  $('#info').textContent = rows.length + ' points · span ≈ ' + Math.round(span/60) + ' min';

  // draw
  drawSeries($('#cvV'), rows, 'v', '#0b5fff', 'V');
  drawSeries($('#cvI'), rows, 'i', '#e11d48', 'mA');
  drawSeries($('#cvP'), rows, 'p', '#10b981', 'W');
}

// --- NEW: Delete all data flow ---
async function clearAllData(selectedSec){
  const btn = $('#btnClear');
  if (!btn) return;
  if (!confirm('Delete all stored data? This cannot be undone.')) return;

  // disable while in-flight
  btn.disabled = true;
  const prevText = btn.textContent;
  btn.textContent = 'Deleting…';
  $('#info').textContent = 'Deleting logs…';

  try {
    const res = await fetch('/api/logs/clear', { method: 'POST' });
    if (!res.ok) throw new Error('HTTP ' + res.status);
    // some backends return JSON; we don’t rely on body here

    // reload charts with the same range selection
    await loadRange(selectedSec);
    $('#info').textContent = 'Logs cleared.';
  } catch (err) {
    console.error(err);
    $('#info').textContent = 'Delete failed.';
    alert('Failed to delete logs.\n' + err.message);
  } finally {
    btn.disabled = false;
    btn.textContent = prevText;
  }
}

function setup(){
  const DEFAULT_SEC = 1800;  // 30 min
  const POLL_MS = 5000;      // sync with sampling

  // keep current range selection across auto-refresh
  let selectedSec = DEFAULT_SEC;

  // initialize from checked radio
  const checked = document.querySelector('input[name="range"]:checked');
  if (checked) {
    selectedSec = checked.value === 'max' ? 'max' : Number(checked.value) || DEFAULT_SEC;
  }

  // range radios
  document.querySelectorAll('input[name="range"]').forEach(inp=>{
    inp.addEventListener('change', ()=>{
      if (!inp.checked) return;
      selectedSec = inp.value === 'max' ? 'max' : Number(inp.value) || DEFAULT_SEC;
      loadRange(selectedSec).catch(()=> $('#info').textContent='Load error');
    });
  });

  // hook up delete button
  const btnClear = $('#btnClear');
  if (btnClear) {
    btnClear.addEventListener('click', ()=> clearAllData(selectedSec));
  }

  let inFlight = false;
  async function refresh() {
    if (inFlight) return;
    inFlight = true;
    try { await loadRange(selectedSec); }
    catch(e){ /* ignore */ }
    finally { inFlight = false; }
  }

  // initial
  refresh();

  // polling when visible
  setInterval(()=>{ if (!document.hidden) refresh(); }, POLL_MS);
  document.addEventListener('visibilitychange', ()=>{
    if (!document.hidden) refresh();
  });
}

window.addEventListener('load', setup);