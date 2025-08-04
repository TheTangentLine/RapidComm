/**
 * Configuration module for RapidComm upload interface
 */

export const CONFIG = {
  // Backend configuration
  BACKEND_PORT: "8080",
  
  // Upload settings
  MAX_RETRIES: 3,
  UPLOAD_TIMEOUT: 10 * 60 * 1000, // 10 minutes
  RETRY_DELAY_BASE: 2000, // Base delay for exponential backoff
  
  // UI settings
  PARTICLE_COUNT: 50,
  PROGRESS_UPDATE_INTERVAL: 100,
  MESSAGE_AUTO_HIDE_DELAY: 3000,
  
  // File settings
  LARGE_FILE_THRESHOLD: 10 * 1024 * 1024, // 10MB
  PROGRESS_LOG_INTERVAL: 5 // Log progress every 5%
};

/**
 * Get backend URL dynamically based on current window location
 * @returns {string} Backend URL
 */
export function getBackendUrl() {
  const protocol = window.location.protocol;
  const hostname = window.location.hostname;
  return `${protocol}//${hostname}:${CONFIG.BACKEND_PORT}`;
}

/**
 * Get upload endpoint URL
 * @returns {string} Upload endpoint URL
 */
export function getUploadUrl() {
  return `${getBackendUrl()}/upload`;
}

/**
 * Log configuration information
 */
export function logConfiguration() {
  console.log("========================================");
  console.log("  RapidComm File Upload Client");
  console.log("========================================");
  console.log(`Frontend URL: ${window.location.origin}`);
  console.log(`Backend URL:  ${getBackendUrl()}`);
  console.log(`Upload URL:   ${getUploadUrl()}`);
  console.log("========================================");
}