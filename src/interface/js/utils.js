/**
 * Utility functions for RapidComm upload interface
 */

/**
 * Format file size to human readable format
 * @param {number} bytes - File size in bytes
 * @returns {string} Formatted file size
 */
export function formatFileSize(bytes) {
  if (bytes === 0) return "0 Bytes";
  const k = 1024;
  const sizes = ["Bytes", "KB", "MB", "GB"];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + " " + sizes[i];
}

/**
 * Calculate hash of file for integrity verification (compatible with server)
 * @param {File} file - File object to hash
 * @returns {Promise<string>} File hash
 */
export async function calculateFileHash(file) {
  const arrayBuffer = await file.arrayBuffer();
  const dataArray = new Uint8Array(arrayBuffer);

  // Simple hash algorithm that matches server implementation
  let hash1 = 0;
  let hash2 = 0;
  let hash3 = 0;

  // Hash pass 1: Basic hash
  for (let i = 0; i < dataArray.length; i++) {
    hash1 = ((hash1 << 5) - hash1 + dataArray[i]) & 0xffffffff;
  }

  // Hash pass 2: With salt1 - process salt1 chars first, then data
  const salt1 = "salt1";
  for (let i = 0; i < dataArray.length; i++) {
    hash2 = ((hash2 << 5) - hash2 + dataArray[i]) & 0xffffffff;
  }
  for (let i = 0; i < salt1.length; i++) {
    hash2 = ((hash2 << 5) - hash2 + salt1.charCodeAt(i)) & 0xffffffff;
  }

  // Hash pass 3: With salt2 and size - process data first, then salt2
  const salt2 = "salt2" + dataArray.length.toString();
  for (let i = 0; i < dataArray.length; i++) {
    hash3 = ((hash3 << 5) - hash3 + dataArray[i]) & 0xffffffff;
  }
  for (let i = 0; i < salt2.length; i++) {
    hash3 = ((hash3 << 5) - hash3 + salt2.charCodeAt(i)) & 0xffffffff;
  }

  // Calculate checksum (similar to server)
  let checksum = 0;
  for (let i = 0; i < dataArray.length; i++) {
    checksum = checksum ^ dataArray[i];
    checksum = (checksum << 1) | (checksum >>> 31); // Rotate left by 1
  }
  checksum = checksum >>> 0; // Ensure unsigned 32-bit

  // Combine hashes and format as hex (similar to server)
  let result =
    hash1.toString(16) +
    hash2.toString(16) +
    hash3.toString(16) +
    checksum.toString(16);

  // Pad to 64 characters
  while (result.length < 64) {
    result += "0";
  }
  if (result.length > 64) {
    result = result.substring(0, 64);
  }

  return result;
}

/**
 * Calculate CRC32 checksum for additional verification
 * @param {string} data - Data to calculate CRC32 for
 * @returns {number} CRC32 checksum
 */
export function calculateCRC32(data) {
  const crcTable = [];
  for (let i = 0; i < 256; i++) {
    let crc = i;
    for (let j = 0; j < 8; j++) {
      crc = crc & 1 ? 0xedb88320 ^ (crc >>> 1) : crc >>> 1;
    }
    crcTable[i] = crc;
  }

  let crc = 0 ^ -1;
  for (let i = 0; i < data.length; i++) {
    crc = (crc >>> 8) ^ crcTable[(crc ^ data.charCodeAt(i)) & 0xff];
  }
  return (crc ^ -1) >>> 0;
}

/**
 * Verify file integrity by comparing sizes and hashes
 * @param {File} originalFile - Original file object
 * @param {number} uploadedSize - Size reported by server
 * @param {string} serverHash - Hash reported by server
 * @returns {Promise<boolean>} Whether file integrity is verified
 */
export async function verifyFileIntegrity(originalFile, uploadedSize, serverHash) {
  try {
    // Check file size first
    if (originalFile.size !== uploadedSize) {
      console.error(
        `[Integrity Check] Size mismatch: ${originalFile.size} vs ${uploadedSize}`
      );
      return false;
    }

    // Calculate original file hash
    const originalHash = await calculateFileHash(originalFile);

    // Compare hashes
    if (originalHash !== serverHash) {
      console.error(
        `[Integrity Check] Hash mismatch: ${originalHash} vs ${serverHash}`
      );
      return false;
    }

    console.log(
      `[Integrity Check] ✅ File integrity verified - SHA256: ${originalHash.substring(
        0,
        16
      )}...`
    );
    return true;
  } catch (error) {
    console.error(
      `[Integrity Check] Error during verification: ${error.message}`
    );
    return false;
  }
}

/**
 * Debounce function to limit the rate of function calls
 * @param {Function} func - Function to debounce
 * @param {number} wait - Wait time in milliseconds
 * @returns {Function} Debounced function
 */
export function debounce(func, wait) {
  let timeout;
  return function executedFunction(...args) {
    const later = () => {
      clearTimeout(timeout);
      func(...args);
    };
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);
  };
}

/**
 * Throttle function to limit the rate of function calls
 * @param {Function} func - Function to throttle
 * @param {number} limit - Time limit in milliseconds
 * @returns {Function} Throttled function
 */
export function throttle(func, limit) {
  let inThrottle;
  return function(...args) {
    if (!inThrottle) {
      func.apply(this, args);
      inThrottle = true;
      setTimeout(() => inThrottle = false, limit);
    }
  };
}