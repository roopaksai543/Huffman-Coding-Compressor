// ---- load the compiled C++ module ----
let Module = null;
HuffmanModule().then((m) => {
  Module = m;
  console.log("wasm module loaded");
});

// ---- grab all the elements ----
const fileInput = document.getElementById("fileInput");
const sampleBtn = document.getElementById("sampleBtn");
const filenameLabel = document.getElementById("filenameLabel");
const compressBtn = document.getElementById("compressBtn");
const decompressBtn = document.getElementById("decompressBtn");
const downloadBtn = document.getElementById("downloadBtn");
const downloadNote = document.getElementById("downloadNote");
const codeTable = document.getElementById("codeTable");

let currentBytes = null;   // the file the user picked or the sample text, as bytes
let currentName = "input.txt";
let lastCompressedBytes = null; // kept in memory so decompress has something to work with
let pendingDownload = null; // { bytes, filename } for whatever compress/decompress just produced

// ---- error banner ----
const errorBanner = document.getElementById("errorBanner");
const errorText = document.getElementById("errorText");

function showError(message) {
  errorText.textContent = message;
  errorBanner.classList.add("visible");
}
function clearError() {
  errorBanner.classList.remove("visible");
  errorText.textContent = "";
}

function getExtension(filename) {
  const dot = filename.lastIndexOf(".");
  return dot === -1 ? "" : filename.slice(dot + 1).toLowerCase();
}

function getBaseName(filename) {
  const dot = filename.lastIndexOf(".");
  return dot === -1 ? filename : filename.slice(0, dot);
}

// ---- shared: whatever file gets loaded (picked or sample) lands here ----
function loadFile(bytes, name) {
  currentBytes = bytes;
  currentName = name;
  filenameLabel.textContent = "loaded: " + name + " (" + bytes.length + " bytes)";

  clearError();
  // both actions become available; each validates the file type itself on click
  compressBtn.disabled = false;
  decompressBtn.disabled = false;

  // a new file invalidates any result tied to whatever was loaded before —
  // otherwise Decompress would silently keep reusing the old compressed
  // data instead of validating (or acting on) the file just loaded
  lastCompressedBytes = null;
  pendingDownload = null;
  downloadBtn.disabled = true;
  downloadNote.textContent = "";

  document.getElementById("origSize").textContent = "-";
  document.getElementById("compSize").textContent = "-";
  document.getElementById("saved").textContent = "-";
  document.getElementById("runtime").textContent = "-";
  document.getElementById("charCount").textContent = "-";
  document.getElementById("shortestCode").textContent = "-";
  document.getElementById("longestCode").textContent = "-";
  document.getElementById("verify").textContent = "-";
  codeTable.innerHTML = "<tr><th>Character</th><th>Frequency</th><th>Code</th></tr>";
}

// ---- input: file picker ----
fileInput.addEventListener("change", () => {
  const f = fileInput.files[0];
  if (!f) return;
  const reader = new FileReader();
  reader.onload = () => {
    loadFile(new Uint8Array(reader.result), f.name);
  };
  reader.readAsArrayBuffer(f);
});

// ---- input: sample text button ----
sampleBtn.addEventListener("click", () => {
  fetch("sample.txt")
    .then((res) => res.arrayBuffer())
    .then((buf) => {
      loadFile(new Uint8Array(buf), "sample.txt");
    });
});

// ---- progress bar helpers ----
const progressBar = document.getElementById("progressBar");
const progressLabel = document.getElementById("progressLabel");

function showProgress(label) {
  progressBar.style.display = "inline";
  progressLabel.style.display = "inline";
  progressLabel.textContent = label;
}
function hideProgress() {
  progressBar.style.display = "none";
  progressLabel.style.display = "none";
}

// ---- compress button ----
compressBtn.addEventListener("click", () => {
  clearError();

  if (!currentBytes) {
    showError("Load a file before compressing.");
    return;
  }
  if (getExtension(currentName) !== "txt") {
    showError('Only .txt files can be compressed — "' + currentName + '" isn\'t one.');
    return;
  }

  showProgress("Compressing...");
  compressBtn.disabled = true;

  // setTimeout lets the browser actually paint the progress bar
  // before the blocking wasm call starts (runCompress is synchronous,
  // so nothing else can update the page while it's running)
  setTimeout(() => {
    // 1. put the file into the wasm virtual filesystem
    Module.FS.writeFile("/input.txt", currentBytes);

    // 2. run compression, timing how long it takes
    const t0 = performance.now();
    Module.runCompress("/input.txt", "/output.huff");
    const t1 = performance.now();

    // 3. read the compressed file back out
    const compressed = Module.FS.readFile("/output.huff");
    lastCompressedBytes = compressed;

    // 4. show basic size stats
    const origSize = currentBytes.length;
    const compSize = compressed.length;
    document.getElementById("origSize").textContent = origSize + " bytes";
    document.getElementById("compSize").textContent = compSize + " bytes";
    document.getElementById("saved").textContent =
      (100 * (1 - compSize / origSize)).toFixed(1) + "%";
    document.getElementById("runtime").textContent = (t1 - t0).toFixed(2) + " ms";
    document.getElementById("charCount").textContent = origSize;

    // 5. pull the char -> code table out of the compressed file's header
    showCodeTable(compressed);

    // hold the compressed file until the user asks to save it
    const compressedName = getBaseName(currentName) + "compressed.huff";
    setPendingDownload(
      compressed,
      compressedName,
      compressedName + " is ready — click Download to save it. " +
        "It's Huffman-encoded binary, not plain text, so opening it directly won't show anything readable."
    );

    decompressBtn.disabled = false;
    compressBtn.disabled = false;
    hideProgress();
  }, 50);
});

// ---- decompress button ----
decompressBtn.addEventListener("click", () => {
  clearError();

  // decompress the loaded file itself if it's a .huff file, otherwise fall
  // back to whatever was just produced by Compress (a same-session round trip)
  let sourceBytes = null;
  let isRoundTrip = false;

  if (currentBytes && getExtension(currentName) === "huff") {
    sourceBytes = currentBytes;
  } else if (lastCompressedBytes) {
    sourceBytes = lastCompressedBytes;
    isRoundTrip = true;
  } else if (currentBytes) {
    showError('Only .huff files can be decompressed \u2014 "' + currentName + '" isn\'t one.');
    return;
  } else {
    showError("Load a .huff file to decompress, or compress a file first.");
    return;
  }

  showProgress("Decompressing...");
  decompressBtn.disabled = true;

  setTimeout(() => {
    Module.FS.writeFile("/output.huff", sourceBytes);
    Module.runDecompress("/output.huff", "/roundtrip.txt");
    const decoded = Module.FS.readFile("/roundtrip.txt");

    // only a same-session round trip has a known original to check against
    if (isRoundTrip) {
      let matches = decoded.length === currentBytes.length;
      if (matches) {
        for (let i = 0; i < decoded.length; i++) {
          if (decoded[i] !== currentBytes[i]) { matches = false; break; }
        }
      }
      document.getElementById("verify").textContent = matches ? "match \u2713" : "mismatch \u2717";
    } else {
      document.getElementById("verify").textContent = "n/a (original not loaded)";
    }

    const outName = getBaseName(currentName) + "decompressed.txt";
    setPendingDownload(
      decoded,
      outName,
      outName + " is ready \u2014 click Download to save it."
    );

    decompressBtn.disabled = false;
    hideProgress();
  }, 50);
});

// ---- parse the header out of a compressed file and show the code table ----
function showCodeTable(compressedBytes) {
  // first 4 bytes = header length (uint32, little-endian)
  const headerLen =
    compressedBytes[0] |
    (compressedBytes[1] << 8) |
    (compressedBytes[2] << 16) |
    (compressedBytes[3] << 24);

  // next headerLen bytes = the header text itself
  const headerBytes = compressedBytes.slice(4, 4 + headerLen);
  const headerStr = new TextDecoder().decode(headerBytes);

  // header is "char" + delimiter(30) + "code" + delimiter(30), repeated
  const parts = headerStr.split(String.fromCharCode(30)).filter((p) => p.length > 0);

  // count how many times each character shows up in the original input
  const freq = {};
  for (const b of currentBytes) freq[b] = (freq[b] || 0) + 1;

  const rows = [];
  for (let i = 0; i < parts.length; i += 2) {
    const char = parts[i];
    const code = parts[i + 1];
    const count = freq[char.charCodeAt(0)] || 0;
    rows.push({ char, code, count });
  }

  // sort by code length to find shortest / longest
  rows.sort((a, b) => a.code.length - b.code.length);
  if (rows.length > 0) {
    const shortest = rows[0];
    const longest = rows[rows.length - 1];
    document.getElementById("shortestCode").textContent =
      "'" + shortest.char + "' -> " + shortest.code;
    document.getElementById("longestCode").textContent =
      "'" + longest.char + "' -> " + longest.code;
  }

  // fill in the table, sorted by frequency (most frequent first)
  rows.sort((a, b) => b.count - a.count);
  codeTable.innerHTML = "<tr><th>Character</th><th>Frequency</th><th>Code</th></tr>";
  for (const r of rows) {
    const tr = document.createElement("tr");
    tr.innerHTML = `<td>${escapeHtml(r.char)}</td><td>${r.count}</td><td>${r.code}</td>`;
    codeTable.appendChild(tr);
  }
}

function escapeHtml(s) {
  if (s === " ") return "(space)";
  if (s === "\n") return "\\n";
  return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

function downloadBytes(bytes, filename) {
  const blob = new Blob([bytes]);
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  a.click();
  URL.revokeObjectURL(url);
}

// stash a finished file and let the user trigger the save themselves
function setPendingDownload(bytes, filename, note) {
  pendingDownload = { bytes, filename };
  downloadBtn.disabled = false;
  downloadNote.textContent = note;
}

// ---- download button ----
downloadBtn.addEventListener("click", () => {
  if (!pendingDownload) return;
  downloadBytes(pendingDownload.bytes, pendingDownload.filename);
});