#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# The screen and the keyboard, over a local page, so a run can be driven by hand.
#
# There is no window toolkit to hand and installing one is the user's decision, so the display goes out as a picture over http and the keys come back the same way.
# Everything here is the standard library.
#
# The emulator runs in the main thread and never stops: sys_sleep, which is where the firmware waits for a key, and key_empty, which is how a long computation tests for
# EXIT, take whatever the page has sent. The server runs in a thread of its own and only touches two things, a queue of key codes and the last frame, both behind one
# lock.

import http.server
import json
import queue
import threading

RESTART_GRACE = 1.0                                           # seconds the emulator has to restart itself before the server thread does it

PAGE = """<!doctype html><meta charset=utf-8><title>%(title)s</title>
<style>
 :root{--gold:#E5AE5A;--blue:#7EB6BA;--red:#C0563F;--letter:#a5a5a5;--keyBg:#212121;--keyBorder:#7c7c7c;--bezel:#2B2A29;--behind:#413D38;--fn:#808080;--hover:#744a2e;--boxLine:var(--keyBorder)}
 body{background:#141414;margin:0;padding:14px;display:flex;flex-direction:column;align-items:center;gap:6px;
      font:12px -apple-system,system-ui,sans-serif;color:#8a8a8a;outline:none}
 /* The panel is drawn at one screen pixel to one page pixel and then zoomed by a whole number, so the display is never resampled and no row or column is lost. */
 #calc{background:var(--bezel);border:2px solid #111;border-radius:12px;padding:8px 9px 10px;width:418px;box-shadow:0 8px 30px #0008}
 #screen{background:var(--behind);border-radius:4px;padding:4px;width:400px}
 img{display:block;width:400px;height:240px;image-rendering:pixelated;background:#cbd0c2}
 #fn{display:grid;grid-template-columns:repeat(6,1fr);gap:6px;margin:7px 1px 9px}
 #fn button{height:33px;background:var(--keyBg);border:1px solid var(--keyBorder);border-radius:3px;padding:0}
 .row{display:grid;gap:5px;margin-bottom:5px}
 .r6{grid-template-columns:repeat(6,minmax(0,1fr))} .r5{grid-template-columns:repeat(5,minmax(0,1fr))}  /* a wide legend spills into the gap, never widens its column */
 .k{background:var(--keyBg);border:1px solid var(--keyBorder);border-radius:3px;position:relative;
    height:31px;color:#fff;font-size:13px;display:flex;align-items:center;justify-content:center;cursor:pointer;user-select:none}
 .k:hover{background:var(--hover)}
 .k:active{transform:translateY(1px)}
 .k .let{position:absolute;right:3px;bottom:1px;color:var(--letter);font-size:8px}
 .leg{display:flex;justify-content:center;align-items:center;gap:5px;font-size:10.8px;height:14px;line-height:10px;white-space:nowrap}
 .leg .f{color:var(--gold)} .leg .g{color:var(--blue)} .leg .f,.leg .g{transform:scaleY(1.2)}  /* taller only, so no legend takes more width */
 /* The frame takes the same room as the text it surrounds, so a name inside one sits on the line of the names beside it and is centred in its own frame. */
 .leg .box{border:1px solid var(--boxLine);border-radius:3px;padding:2px 3px 3px;margin:-3px 0 -4px;display:inline-flex;gap:4px}  /* deeper below, for the stretched / of I/O */
 .leg .f+.box{margin-left:-3px}  /* 2px from the plain name before it, whose text then sits about as far from the framed one as two plain names do */
 .k.f{background:var(--gold);color:#000;font-weight:600} .k.g{background:var(--blue);color:#000;font-weight:600}
 #status{font-size:11px;opacity:.75;height:14px}
 /* Stack and heap as the run goes on. Gold is what is taken now, the dark between is the room the most ever taken reached, and blue is what is left beyond that.
    A bar whose whole width has been taken goes red, which on the stack is the reading that matters: what is under it is written over from there on. Where the ELF
    names C47's block pool, the heap bar is split at the end of the pool: the pool fills leftwards from the line and every other malloc rightwards, each red on
    its own side when an allocation asked for more than that side has. Where the stack writes into the platform's heap, a red line under the stack bar names it. */
 #use{width:418px;display:flex;flex-direction:column;gap:4px;font-size:10px}
 #use .line{display:flex;justify-content:space-between;color:var(--fn)}
 #use .bar{position:relative;height:6px;background:var(--keyBg);border:1px solid var(--boxLine);border-radius:3px;overflow:hidden}
 #use .fill{position:absolute;left:0;top:0;bottom:0;background:var(--gold)}
 #use a{color:var(--fn);text-decoration:none;border-bottom:1px dotted var(--fn)}
 #use a:hover{color:var(--gold);border-color:var(--gold)}
 #use .mark{position:absolute;top:0;bottom:0;right:0;background:var(--blue)}
 #use .bar.over,#use .bar.over .fill{background:var(--red)}
 #use .bar.over .mark{display:none}
 #use .pool{position:absolute;top:0;bottom:0;background:var(--gold)} #use .room{position:absolute;top:0;bottom:0;left:0;background:var(--blue)}
 #use .pool.over,#use .fill.over{background:var(--red)} #use .split{position:relative}
 #use .edge{position:absolute;top:-3px;bottom:-3px;width:2px;background:#fff}  /* outside the bar, which clips what is inside it to its rounded corners */
 #use .warn{color:var(--red)} #use .line[hidden]{display:none}
 /* The file chooser, drawn over the panel because the platform's own takes the whole display. */
 #ask{position:fixed;inset:0;background:#000a;display:none;align-items:center;justify-content:center}
 #ask.up{display:flex}
 #box{background:var(--bezel);border:1px solid var(--boxLine);border-radius:8px;padding:12px;width:380px;color:#ddd}
 #box h1{font-size:13px;margin:0 0 8px;font-weight:600}
 #box .where{font-size:11px;color:var(--fn);margin:0 0 8px}
 #list{max-height:250px;overflow-y:auto;border:1px solid var(--boxLine);border-radius:4px}
 #list div{padding:4px 8px;cursor:pointer;display:flex;justify-content:space-between;font-size:12px}
 #list div:hover,#list div.on{background:var(--hover);color:#fff}
 #list .none{color:var(--fn);cursor:default;justify-content:center;padding:12px}
 #box input{width:100%%;margin-top:8px;padding:5px 8px;background:var(--keyBg);color:#ddd;
            border:1px solid var(--boxLine);border-radius:4px;font:inherit;box-sizing:border-box}
 #box .row{display:flex;gap:8px;justify-content:flex-end;margin-top:10px}
 #box button{padding:5px 14px;background:var(--keyBg);color:#ddd;border:1px solid var(--boxLine);border-radius:4px;font:inherit;cursor:pointer}
 #box button:hover{background:var(--hover);color:#fff}
</style>
<body tabindex=0>
<div id=calc>
  <div id=screen><img id=v src="frame.bmp"></div>
  <div id=fn>%(softkeys)s</div>
  <div id=keys></div>
</div>
<div id=use>
  <div class=line><span>stack <a href=# id=stackReset>reset</a></span><span id=stackText></span></div>
  <div class=bar><div class=fill id=stackFill></div><div class=mark id=stackMark></div></div>
  <div class=line id=overLine hidden><span class=warn>overflow</span><span class=warn id=overText></span></div>
  <div class=line><span>heap <a href=# id=heapReset>reset</a></span><span id=heapText></span></div>
  <div class=split><div class=bar><div class=room id=poolRoom hidden></div><div class=pool id=poolFill hidden></div>
    <div class=fill id=heapFill></div><div class=mark id=heapMark></div></div><div class=edge id=poolEdge hidden></div></div>
  <div class=line><span>run <a href=# id=restart>restart</a></span><span>the image is read from disk again, so a new build of it runs</span></div>
</div>
<div id=status>connecting</div>
<div id=ask><div id=box>
  <h1 id=askTitle></h1>
  <p class=where id=askWhere></p>
  <div id=list></div>
  <input id=askName spellcheck=false>
  <div class=row><button id=askCancel>Cancel</button><button id=askOk></button></div>
</div></div>
<script>
const named = %(named)s, letters = %(letters)s, digits = %(digits)s, shift = %(shift)d;
const KEYS = %(keys)s;                // rows of [dmcp, main, f, g, letter, optional {span, tint, box}]
const v = document.getElementById('v'), st = document.getElementById('status');
// A press and its release go over separately, so a key held down stays down and whatever the firmware shows in between is on the screen while it is.
let held = null;
function down(c) {
  if (held !== null) return;
  held = c;
  fetch('key?c=' + c, {method:'POST'});
  st.textContent = 'key ' + c + ' down';
}
function up() {
  if (held === null) return;
  held = null;
  fetch('key?c=0', {method:'POST'});
  st.textContent = 'released';
}
addEventListener('mouseup', up);
addEventListener('blur', up);

const board = document.getElementById('keys');
const span = (key) => key[5] && key[5].span || 1;
for (const row of KEYS) {
  const cols = row.reduce((n, key) => n + span(key), 0);   // ENTER takes two, so its row is six columns wide like the ones above it
  const legends = document.createElement('div');
  legends.className = 'row r' + cols;
  const keys = document.createElement('div');
  keys.className = 'row r' + cols;
  for (const key of row) {
    const [code, main, f, g, letter, opts] = key;
    const wide = span(key) > 1 ? `span ${span(key)}` : '';
    const l = document.createElement('div');
    l.className = 'leg';
    l.style.gridColumn = wide;
    const box = opts && opts.box || '';                    // a menu has a frame on the calculator, and where f and g are both menus one frame surrounds the two
    const fs = f ? `<span class=f>${f}</span>` : '', gs = g ? `<span class=g>${g}</span>` : '';
    l.innerHTML = box === 'fg' ? `<span class=box>${fs}${gs}</span>`
                : (box === 'f' ? `<span class=box>${fs}</span>` : fs) + (box === 'g' ? `<span class=box>${gs}</span>` : gs);
    legends.appendChild(l);
    const k = document.createElement('div');
    k.className = 'k' + (opts && opts.tint ? ' ' + opts.tint : '');
    k.style.gridColumn = wide;
    k.innerHTML = main + (letter ? `<span class=let>${letter}</span>` : '');
    k.onmousedown = (e) => { e.preventDefault(); down(code); };
    k.onmouseleave = () => { if (held === code) up(); };
    keys.appendChild(k);
  }
  board.appendChild(legends);
  board.appendChild(keys);
}
document.querySelectorAll('#fn button').forEach((b, i) => { b.onmousedown = (e) => { e.preventDefault(); down(38 + i); }; });

let busy = false;
async function frame() {
  if (busy) return; busy = true;
  try { const r = await fetch('frame.bmp?' + Date.now()); v.src = URL.createObjectURL(await r.blob()); }
  catch (e) { st.textContent = 'no connection'; }
  busy = false;
}
setInterval(frame, 120);

// The file chooser. The emulator puts a question up while it waits inside the DMCP entry the firmware called, and the answer is the bare name, or nothing for a
// screen that was left. Saving offers the same list, because overwriting a file there is how it is done on the calculator, with the name to write in the field.
const ask = document.getElementById('ask'), askTitle = document.getElementById('askTitle'), askWhere = document.getElementById('askWhere');
const list = document.getElementById('list'), askName = document.getElementById('askName'), askOk = document.getElementById('askOk');
let asking = null;

function answer(name) {
  if (asking === null) return;
  asking = null;
  ask.classList.remove('up');
  fetch('chose', {method:'POST', body:name});
  document.body.focus();
}

function show(q) {
  asking = q;
  askTitle.textContent = q.title;
  askWhere.textContent = q.folder + '  (*' + q.ext + ')';
  askOk.textContent = q.saving ? 'Save' : 'Load';
  askName.style.display = q.saving ? '' : 'none';
  askName.value = q.suggested || '';
  list.innerHTML = '';
  if (!q.files.length) {
    const none = document.createElement('div');
    none.className = 'none';
    none.textContent = q.saving ? 'no file here yet' : 'no file to load here';
    list.append(none);
  }
  for (const f of q.files) {
    const row = document.createElement('div');
    row.innerHTML = '<span></span><span></span>';
    row.children[0].textContent = f.name;
    row.children[1].textContent = f.size + ' B';
    row.onclick = () => {
      if (!q.saving) { answer(f.name); return; }
      for (const other of list.children) other.classList.remove('on');
      row.classList.add('on');
      askName.value = f.name;                                 // choosing a file when saving names it, so the write goes over that one
      askName.focus();
    };
    list.append(row);
  }
  ask.classList.add('up');
  if (q.saving) { askName.focus(); askName.select(); }
}

askCancel.onclick = () => answer('');
askOk.onclick = () => { if (asking && asking.saving) answer(askName.value.trim()); };
askName.onkeydown = e => {
  e.stopPropagation();
  if (e.key === 'Enter') answer(askName.value.trim());
  if (e.key === 'Escape') answer('');
};

async function pending() {
  try {
    const q = await (await fetch('chooser?' + Date.now())).json();
    if (q && asking === null) show(q);
    else if (!q && asking !== null) { asking = null; ask.classList.remove('up'); }
  } catch (e) {}
}
setInterval(pending, 150);

// Stack and heap, taken from what the emulator recorded at the last call it answered.
const bar = (fill, mark, live, most, room) => {
  fill.parentNode.classList.toggle('over', Math.max(live, most) >= room);
  fill.style.width = (100 * Math.min(live, room) / room) + '%%';
  mark.style.left = (100 * Math.min(most, room) / room) + '%%';
};
const kb = n => (n / 1024).toFixed(1) + 'k';
const pct = (n, of) => (100 * Math.max(0, Math.min(n, of)) / of) + '%%';
// The heap split at the end of C47's pool: the pool from the line leftwards, gold for taken now and blue for what its peak never reached, malloc rightwards.
function heap(u) {
  const split = u.poolRoom > 0;
  for (const part of [poolRoom, poolFill, poolEdge]) part.hidden = !split;
  if (!split) {
    heapFill.style.left = '0';
    heapFill.classList.remove('over');
    bar(heapFill, heapMark, u.heapNow, u.heapPeak, u.heapSize);
    heapText.textContent = kb(u.heapNow) + ' of ' + kb(u.heapSize) + ', peak ' + kb(u.heapPeak) + ', largest free ' + kb(u.heapSpan);
    return;
  }
  const size = u.heapSize, pool = u.poolRoom, other = size - pool;
  const otherNow = u.heapNow - pool, otherPeak = u.heapPeak - pool;
  poolEdge.style.left = 'calc((100%% - 2px) * ' + pool / size + ')';   // past the bar's 1px border and back by half the line's 2px width
  poolRoom.style.width = pct(pool - u.poolPeak, size);
  poolFill.classList.toggle('over', u.poolPeak >= pool);
  poolFill.style.left = pct(u.poolPeak >= pool ? 0 : pool - u.poolNow, size);
  poolFill.style.width = pct(u.poolPeak >= pool ? pool : u.poolNow, size);
  heapFill.parentNode.classList.remove('over');
  heapFill.classList.toggle('over', otherPeak >= other);
  heapFill.style.left = pct(pool, size);
  heapFill.style.width = pct(otherNow, size);
  heapMark.style.left = pct(pool + otherPeak, size);
  const whole = n => Math.round(n / 1024) + 'k';                       // a pool peak past its room is what the refused allocation asked for, as at RAM is full
  heapText.textContent = 'C47 ' + kb(u.poolNow) + ', peak ' + kb(u.poolPeak) + ' of ' + whole(pool)
                       + ' | malloc ' + kb(otherNow) + ', peak ' + kb(otherPeak) + ' of ' + whole(other);
}
async function usage() {
  try {
    const u = await (await fetch('state?' + Date.now())).json();
    const deep = Math.max(u.stackDeep, u.overflow ? u.overflow.deep : 0);   // the pattern stops above the platform's heap and the stack pointer does not
    bar(stackFill, stackMark, u.stackNow, deep, u.stackRoom);   // as the heap bar below: the fill is what is taken now and the mark the most ever taken
    heap(u);
    stackText.textContent = u.stackSpoiled ? 'the build marks this stretch itself, so read its own STCKHI'
                          : 'deepest ' + kb(deep) + ' of ' + kb(u.stackRoom) + ', now ' + kb(u.stackNow);
    overLine.hidden = !u.overflow;
    if (u.overflow) overText.textContent = 'first into the heap at ' + u.overflow.at + ' from ' + u.overflow.from;
  } catch (e) {}
}
setInterval(usage, 250);
stackReset.onclick = e => { e.preventDefault(); fetch('forget', {method:'POST'}); document.body.focus(); };
heapReset.onclick = e => { e.preventDefault(); fetch('forgetHeap', {method:'POST'}); document.body.focus(); };
// The server goes away with the process it restarts, so the fetch is expected to fail and the page waits for the new one to answer on the same port.
restart.onclick = e => { e.preventDefault(); fetch('restart', {method:'POST'}).catch(() => {}); st.textContent = 'restarting'; document.body.focus(); };

function code(e) {
  const k = e.key, c = e.code;
  if (k === 'Enter' || k === 'Return' || c === 'Enter' || c === 'NumpadEnter') return named.ENTER;
  if (k === 'Escape' || k === 'Esc' || c === 'Escape') return named.EXIT;
  if (k === 'Tab' || c === 'Tab') return shift;
  if (k === 'Backspace' || c === 'Backspace') return named.BSP;
  if (k === 'ArrowUp' || c === 'ArrowUp') return named.UP;
  if (k === 'ArrowDown' || c === 'ArrowDown') return named.DOWN;
  if (/^F[1-6]$/.test(k)) return 37 + Number(k[1]);
  if (k === '+') return named.ADD;
  if (k === '-') return named.SUB;
  if (k === '*') return named.MUL;
  if (k === '/') return named.DIV;
  if (k === '.') return named.DOT;
  if (k.length === 1 && digits[k] !== undefined) return digits[k];
  if (k.length === 1 && letters[k.toUpperCase()] !== undefined) return letters[k.toUpperCase()];
  return null;
}
addEventListener('keydown', e => {
  if (asking !== null) { if (e.key === 'Escape') answer(''); return; }   // the chooser has the keyboard while it is up
  if (e.repeat) { e.preventDefault(); return; }               // the operating system repeat is not a second press
  const c = code(e);
  if (c === null) { st.textContent = 'no key for ' + (e.key || '?'); return; }
  e.preventDefault();
  down(c);
});
addEventListener('keyup', e => { if (asking !== null) return; e.preventDefault(); up(); });
// The largest whole zoom the window has room for, so the calculator arrives whole. The panel is measured rather than written down, so changing a key height here
// cannot leave a stale number behind.
const calc = document.getElementById('calc');
calc.style.zoom = Math.max(1, Math.min(3, Math.floor((innerHeight - 90) / calc.getBoundingClientRect().height)));
document.body.focus();
</script>"""



# The face of each model: rows of [DMCP code, the key, the f legend, the g legend, the alpha letter], and a last element with the span, the tint or the frame. The two
# bezels, read out of the firmware's own keyboard tables rather than written down by hand: kbd_std_C47 and kbd_std_R47f_g in src/c47/assign.c give the place of every key,
# its f and its g function and the letter it types, and indexOfItems in src/c47/items.c gives the text each of those functions is printed with on the keyboard. The key
# numbering there is by place, row then column, so what is left to supply is the DMCP code each place sends. On C47 that is the place itself. On R47 seven of them differ,
# from the grid in convertKeyCode, and every one of those was confirmed by running R47.pg5 and reading the screen: see keyscript.py.
#
# What the grid prints on the R47 keys is the silkscreen and not what they do: on the R47 the key that performs DRG is printed SIN, and the two
# the grid calls SHIFT and TAN are the gold and the blue shift, side by side with nothing written on either. The function is what is drawn here, so that nobody is
# told to press XEQ and given LN.
#
# A menu has a frame on the calculator and a plain function does not, which the tables already show: a menu is written there as a negative number. Where the f and the g
# of one key are both menus the calculator draws a single frame around the pair, so box is one of f, g or fg. A photograph of the R47 confirms every frame this produces.
FACE_C47 = [
  [[1, '\u03a3+', '\u2192I', 'a b/c', 'A'], [2, '1/x', 'y\u02e3', '#', 'B'], [3, '\u221ax', 'x\u00b2', '.ms', 'C'], [4, 'LOG', '10\u02e3', '.d', 'D'],
   [5, 'LN', 'e\u02e3', 'LBL', 'E'], [6, 'XEQ', '\u03b1', 'GTO', 'F']],
  [[7, 'STO', '|x|', '\u2221', 'G'], [8, 'RCL', '%', '\u0394%', 'H'], [9, 'R\u2193', '\u03c0', '\u02e3\u221ay', 'I'], [10, 'SIN', 'ASIN', 'i', 'J'],
   [11, 'COS', 'ACOS', '\u2192R', 'K'], [12, 'TAN', 'ATAN', '\u2192P', 'L']],
  [[13, 'ENTER', 'COMPLEX', 'CPX', '', {'box': 'g', 'span': 2}], [14, 'x\u21c4y', 'LASTx', 'STK', 'M', {'box': 'g'}],
   [15, 'CHS', 'MODE', 'TRG', 'N', {'box': 'fg'}], [16, 'EEX', 'DISP', 'EXP', 'O', {'box': 'fg'}], [17, '\u2190', '\u21a9', 'CLR', '', {'box': 'g'}]],
  [[18, '\u2191', 'BST', 'REGS', ''], [19, '7', 'EQN', 'HOME', 'P', {'box': 'fg'}], [20, '8', 'ADV', 'FIN', 'Q', {'box': 'fg'}],
   [21, '9', 'MATX', 'X.FN', 'R', {'box': 'fg'}], [22, '\u00f7', 'STAT', 'PLOT', 'S', {'box': 'fg'}]],
  [[23, '\u2193', 'SST', 'FLGS', ''], [24, '4', 'BASE', 'BITS', 'T', {'box': 'fg'}], [25, '5', 'CONV', 'CLK', 'U', {'box': 'fg'}],
   [26, '6', 'FLAG', 'REAL', 'V', {'box': 'fg'}], [27, '\u00d7', 'PROB', 'INTS', 'W', {'box': 'fg'}]],
  [[28, '', '', '', '', {'tint': 'f'}], [29, '1', 'ASN', 'KEYS', 'X', {'box': 'g'}], [30, '2', 'USER', '\u03b1.FN', 'Y', {'box': 'g'}],
   [31, '3', 'P.FN', 'LOOP', 'Z', {'box': 'fg'}], [32, '-', 'PRINT', 'I/O', '_', {'box': 'fg'}]],
  [[33, 'EXIT', 'OFF', 'SNAP', ''], [34, '0', 'VIEW', 'STOPW', ':'], [35, '.', 'SHOW', 'INFO', ',', {'box': 'g'}],
   [36, 'R/S', 'PRGM', 'TEST', '?', {'box': 'g'}], [37, '+', 'CAT', 'CNST', '', {'box': 'fg'}]],
]

FACE_R47 = [
  [[1, 'x\u00b2', 'i', '\u2192R', 'A'], [2, '\u221ax', 'i\u2299', '\u2192P', 'B'], [3, '1/x', 'x!', '.ms', 'C'],
   [4, 'y\u02e3', '\u02e3\u221ay', '.d', 'D'], [5, 'LOG', '10\u02e3', '\u2192I', 'E'], [6, 'LN', 'e\u02e3', '#', 'F']],
  [[7, 'STO', '|x|', '\u2221', 'G'], [8, 'RCL', '%', '\u0394%', 'H'], [9, 'R\u2193', '\u03c0', 'R\u2191', 'I'], [10, 'DRG', 'USER', 'ASN', 'J'],
   [28, '', '', '', '', {'tint': 'f'}], [12, '', '', '', '', {'tint': 'g'}]],
  [[13, 'ENTER', 'COMPLEX', 'CPX', '', {'box': 'g', 'span': 2}], [14, 'x\u21c4y', 'LASTx', 'STK', 'K', {'box': 'g'}],
   [16, 'CHS', 'DISP', 'TRG', 'L', {'box': 'fg'}], [15, 'EEX', 'PFX', 'EXP', 'M', {'box': 'fg'}], [17, '\u2190', '\u21a9', 'CLR', '', {'box': 'g'}]],
  [[11, 'XEQ', '\u03b1', 'GTO', '_'], [19, '7', 'SIN', 'ASIN', 'N'], [20, '8', 'COS', 'ACOS', 'O'], [21, '9', 'TAN', 'ATAN', 'P'],
   [22, '\u00f7', 'STAT', 'PLOT', 'Q', {'box': 'fg'}]],
  [[18, '\u2191', 'BST', 'REGS', ''], [24, '4', 'BASE', 'BITS', 'R', {'box': 'fg'}], [25, '5', 'INTS', 'REAL', 'S', {'box': 'fg'}],
   [26, '6', 'MATX', 'X.FN', 'T', {'box': 'fg'}], [27, '\u00d7', 'EQN', 'ADV', 'U', {'box': 'fg'}]],
  [[23, '\u2193', 'SST', 'FLGS', ''], [29, '1', 'PREF', 'KEYS', 'V', {'box': 'fg'}], [30, '2', 'CONV', 'CLK', 'W', {'box': 'fg'}],
   [31, '3', 'FLAG', '\u03b1.FN', 'X', {'box': 'fg'}], [32, '-', 'PROB', 'FIN', 'Y', {'box': 'fg'}]],
  [[33, 'EXIT', 'OFF', 'INFO', '', {'box': 'g'}], [34, '0', 'VIEW', 'I/O', 'Z', {'box': 'g'}], [35, '.', 'SHOW', 'a b/c', ','],
   [36, 'R/S', 'PRGM', 'P.FN', '?', {'box': 'g'}], [37, '+', 'CAT', 'CNST', '', {'box': 'fg'}]],
]

FACES = {'C47': FACE_C47, 'R47': FACE_R47}


class Screen:
  """What the server hands out and what the emulator puts there, behind one lock."""

  def __init__(self, page):
    self.page = page.encode('utf-8')
    self.frame = b''
    self.keys = queue.Queue()
    self.asked = b'null'                                      # the file the emulator is waiting to have named, as the page takes it
    self.answer = queue.Queue()
    self.restarting = False                                   # set by the page's restart link, acted on by the emulator where it takes keys
    self.lock = threading.Lock()

  def put(self, bmp):
    with self.lock:
      self.frame = bmp

  def take(self):
    with self.lock:
      return self.frame

  def ask(self, question):
    """Put a question to the page, or take it down again with None."""
    with self.lock:
      self.asked = json.dumps(question).encode('utf-8') if question else b'null'

  def asking(self):
    with self.lock:
      return self.asked


def _state(emu):
  """The stack and heap numbers the page displays, out of what the emulator keeps as it runs.

  Nothing is read from the emulated processor here, because this is answered on the server's own thread while the emulator runs on another. The stack pointer is the one
  recorded at the last DMCP call, which is as often as the firmware crosses to the host. The room is measured down to the end of the arena where the two share a region,
  because the allocator may hand out every byte of it, and down to the foot of the region where they do not. Measuring to the highest address handed out so far would
  flatter the figure, because that mark moves down as soon as the firmware allocates more. The overflow is the one the report names: the first write the stack made into
  the platform's heap, and how deep its pointer went.
  """
  top = emu.target.stack_top
  region = next(r for r in emu.target.regions if r.addr < top <= r.addr + r.size)
  floor = emu.alloc.limit if emu.alloc.base < top else region.addr
  depth, spoiled = emu.live_depth()
  pool = emu.pool_state()
  room, used = pool if pool else (0, 0)
  overflow, seen = None, emu.overflow                         # taken once, as the emulator's thread replaces it whole
  if seen is not None:
    address, pc, lowest = seen
    overflow = {'at': '0x%08x' % address, 'from': emu.function_at(pc), 'deep': top - lowest}
  return json.dumps({
    'stackRoom': top - floor,
    'stackNow': top - emu.sp_now,
    'stackDeep': depth, 'stackSpoiled': spoiled,
    'heapSize': emu.alloc.size,
    'heapNow': emu.alloc.taken,
    'heapPeak': emu.alloc.peak,
    'heapSpan': emu.alloc.largest_free(),
    'poolRoom': room,
    'poolNow': used,
    'poolPeak': max(used, emu.pool_peak) if pool else 0,
    'overflow': overflow,
  }).encode('utf-8')


def _handler(screen, emu):
  class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *args):
      pass                                                    # the emulator's own report is the output, not a request log

    def _send(self, kind, body):
      self.send_response(200)
      self.send_header('Content-Type', kind)
      self.send_header('Content-Length', str(len(body)))
      self.send_header('Cache-Control', 'no-store')
      self.end_headers()
      self.wfile.write(body)

    def do_GET(self):
      if self.path.startswith('/frame.bmp'):
        self._send('image/bmp', screen.take())
      elif self.path.startswith('/chooser'):
        self._send('application/json', screen.asking())
      elif self.path.startswith('/state'):
        self._send('application/json', _state(emu))
      else:
        self._send('text/html; charset=utf-8', screen.page)

    def do_POST(self):
      if self.path.startswith('/key?c='):
        try:
          screen.keys.put(int(self.path.split('=', 1)[1]))
        except ValueError:
          pass
      elif self.path.startswith('/restart'):
        screen.restarting = True                              # the emulator's own thread does it, where it next takes keys
        later = threading.Timer(RESTART_GRACE, emu.restart)    # and this thread does it where the firmware computes without asking for a key
        later.daemon = True
        later.start()
      elif self.path.startswith('/forgetHeap'):
        emu.forget_heap()                                     # the heap and pool peaks start again from what is taken now
      elif self.path.startswith('/forget'):
        emu.forget_depth()                                    # start the deepest reading again, so one operation can be measured on its own
      elif self.path.startswith('/chose'):
        length = int(self.headers.get('Content-Length') or 0)
        screen.answer.put(self.rfile.read(length).decode('utf-8'))    # the name chosen, or empty for a screen that was left
      self._send('application/json', b'{}')
  return Handler


def start(emu, layout, port):
  """Put the page up and give back the Screen the emulator writes frames to."""
  page = PAGE % {
    'title': '%s %s' % (emu.prog_info.name, emu.prog_info.version),
    'named': json.dumps(layout.named),
    'letters': json.dumps(layout.letters),
    'digits': json.dumps(dict(_digits())),
    'shift': layout.shift,
    'keys': json.dumps(FACES.get(layout.name, FACE_C47)),
    'softkeys': '<button></button>' * 6,
  }
  screen = Screen(page)
  server = http.server.ThreadingHTTPServer(('127.0.0.1', port), _handler(screen, emu))
  threading.Thread(target=server.serve_forever, daemon=True).start()
  return screen, server.server_address[1]


def _digits():
  import keyscript
  return keyscript.DIGITS.items()
