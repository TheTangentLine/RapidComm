import { Command } from "@tauri-apps/plugin-shell";

const API_BASE_URL = "http://localhost:8081/api";
const FRONTEND_PORT = 3000;

let toggleBtn, statusLight, statusText, ipAddressEl, ipAddressContainer;
let isServerRunning = false;

function initializeApp() {
  toggleBtn        = document.getElementById("toggleBtn");
  statusLight      = document.getElementById("statusLight");
  statusText       = document.getElementById("statusText");
  ipAddressEl      = document.getElementById("ipAddress");
  ipAddressContainer = document.getElementById("ipAddressContainer");

  createParticles();

  toggleBtn.addEventListener("click", toggleServer);
  ipAddressContainer.addEventListener("click", copyIpAddress);

  startBackend(); // This just launches the C++ process
}

function createParticles() {
  const particles = document.getElementById("particles");
  if (!particles) return;
  const particleCount = 50;
  for (let i = 0; i < particleCount; i++) {
    const particle = document.createElement("div");
    particle.className = "particle";
    particle.style.left = Math.random() * 100 + "%";
    particle.style.animationDelay = Math.random() * 15 + "s";
    particle.style.animationDuration = Math.random() * 10 + 10 + "s";
    particles.appendChild(particle);
  }
}

async function startBackend() {
  console.log("Attempting to start backend C++ process...");
  try {
    const binary = import.meta.env.DEV ? "../../build/bin/rapidcomm-server" : "rapidcomm-server";
    const command = Command.sidecar(binary);

    command.on("close", (data) => {
      console.log(`Backend process finished with code ${data.code} and signal ${data.signal}`);
      updateStatus(false, "Control Panel Stopped", "-");
    });
    command.on("error", (error) => {
      console.error(`Backend process error: ${error}`);
      updateStatus(false, "Launch Failed", "-");
    });
     command.stdout.on("data", (line) => console.log(`[Backend] ${line}`));
     command.stderr.on("data", (line) => console.error(`[Backend ERR] ${line}`));


    await command.spawn();
    console.log("Backend C++ process spawned.");
    
    // Start polling for the control panel's status
    setTimeout(checkStatus, 1500);
    setInterval(checkStatus, 5000);

  } catch (e) {
    console.error("Failed to spawn backend process:", e);
    updateStatus(false, "Launch Error", "-");
  }
}

async function checkStatus() {
  try {
    const response = await fetch(`${API_BASE_URL}/status`);
    if (!response.ok) throw new Error(`Status check failed: ${response.status}`);
    const data = await response.json();
    
    isServerRunning = data.isRunning;
    const fullUrl = data.ipAddress ? `http://${data.ipAddress}:${FRONTEND_PORT}`: "-";
    updateStatus(data.isRunning, data.isRunning ? "Running" : "Ready to Start", fullUrl);

  } catch (error) {
    console.warn("Could not connect to control panel. It might still be starting...");
    updateStatus(false, "Connecting...", "-");
    setButtonState("Start Server", true, false); // Disable button while disconnected
  }
}

function updateStatus(isRunning, status, url) {
  isServerRunning = isRunning; // Keep the state in sync
  statusText.textContent = status;
  ipAddressEl.textContent = url;
  
  if (isRunning) {
    statusLight.style.background = "var(--status-running)";
    statusLight.classList.add("running");
    setButtonState("Stop Server", false, true);
  } else {
    statusLight.style.background = "var(--status-stopped)";
    statusLight.classList.remove("running");
    setButtonState("Start Server", false, false);
  }
}

// A helper to manage the button's state
function setButtonState(text, disabled, isRunning) {
    toggleBtn.textContent = text;
    toggleBtn.disabled = disabled;
    if (isRunning) {
        toggleBtn.classList.add("running");
    } else {
        toggleBtn.classList.remove("running");
    }
}

async function toggleServer() {
  const command = isServerRunning ? "stop" : "start";
  
  // Optimistic UI update
  setButtonState(isServerRunning ? "Stopping..." : "Starting...", true, isServerRunning);
  
  try {
    await fetch(`${API_BASE_URL}/${command}`, { method: "POST" });
    // The regular polling from checkStatus will handle the final state update.
    // We can trigger one immediately for a slightly faster final update.
    setTimeout(checkStatus, 300); 
  } catch (error) {
    console.error(`Failed to ${command} server:`, error);
    // If the command fails, revert the UI after a short delay
    setTimeout(checkStatus, 500);
  }
}

function copyIpAddress() {
  const url = ipAddressEl.textContent;
  if (url && url !== "-" && !url.includes("Connecting")) {
    navigator.clipboard.writeText(url).then(() => {
        const tooltip = document.querySelector(".copy-tooltip");
        tooltip.textContent = "Copied!";
        setTimeout(() => {
          tooltip.textContent = "Click to copy";
        }, 2000);
      },
      (err) => {
        console.error("Could not copy text: ", err);
      }
    );
  }
}

document.addEventListener("DOMContentLoaded", initializeApp);
