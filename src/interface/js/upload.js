/**
 * Upload management module for RapidComm upload interface
 */

import { CONFIG, getUploadUrl, getBackendUrl } from './config.js';
import { formatFileSize, verifyFileIntegrity, throttle } from './utils.js';

/**
 * Upload Manager class to handle file uploads
 */
export class UploadManager {
  constructor(uiManager) {
    this.ui = uiManager;
    this.activeUploads = new Map();
    this.uploadStats = {
      totalFiles: 0,
      uploadedFiles: 0,
      failedFiles: 0,
      totalBytes: 0,
      uploadedBytes: 0
    };
  }

  /**
   * Upload multiple files sequentially
   * @param {Array} files - Array of File objects to upload
   * @returns {Promise} Promise that resolves when all uploads complete
   */
  async uploadFiles(files) {
    if (!files || files.length === 0) {
      throw new Error('No files selected');
    }

    // Initialize upload stats
    this.uploadStats = {
      totalFiles: files.length,
      uploadedFiles: 0,
      failedFiles: 0,
      totalBytes: files.reduce((sum, file) => sum + file.size, 0),
      uploadedBytes: 0
    };

    this.ui.setUploadingState(true);
    
    const results = [];
    let hasErrors = false;

    try {
      // Upload files sequentially
      for (let i = 0; i < files.length; i++) {
        const file = files[i];
        const fileIndex = i + 1;

        this.ui.setSubmitButtonState(false, `Uploading file ${fileIndex}/${files.length}: ${file.name}`);

        try {
          const result = await this.uploadSingleFile(file, fileIndex, files.length);
          results.push(result);
          this.uploadStats.uploadedFiles++;
          
          console.log(
            `[Upload Complete] File ${fileIndex}/${files.length} - ${result.type || 'file'}: ${result.filename} - Status: SUCCESS`
          );
        } catch (error) {
          console.error(`[Upload] Failed to upload ${file.name}:`, error);
          results.push({ error: error.message, filename: file.name });
          this.uploadStats.failedFiles++;
          hasErrors = true;
        }
      }

      // Show final result
      this.showFinalResult(hasErrors);
      return results;

    } catch (error) {
      console.error('[Upload] Preparation error:', error);
      this.ui.showMessage(
        'Failed to prepare files for upload: ' + error.message,
        'error'
      );
      throw error;
    } finally {
      this.ui.setUploadingState(false);
    }
  }

  /**
   * Upload a single file with progress tracking
   * @param {File} file - File to upload
   * @param {number} currentIndex - Current file index
   * @param {number} totalFiles - Total number of files
   * @returns {Promise} Promise that resolves with upload result
   */
  uploadSingleFile(file, currentIndex, totalFiles) {
    return new Promise((resolve, reject) => {
      const formData = new FormData();
      formData.append('file', file);
      formData.append('originalSize', file.size.toString());
      formData.append('timestamp', Date.now().toString());

      const xhr = new XMLHttpRequest();
      const uploadId = `upload_${currentIndex}_${Date.now()}`;
      
      // Store active upload for potential cancellation
      this.activeUploads.set(uploadId, xhr);

      // Set up progress tracking with throttling to avoid excessive updates
      const progressHandler = throttle((e) => {
        if (e.lengthComputable) {
          this.handleUploadProgress(e, file, currentIndex, totalFiles);
        }
      }, CONFIG.PROGRESS_UPDATE_INTERVAL);

      xhr.upload.addEventListener('progress', progressHandler);

      // Handle successful response
      xhr.addEventListener('load', async () => {
        this.activeUploads.delete(uploadId);
        
        if (xhr.status === 200) {
          try {
            const response = JSON.parse(xhr.responseText);
            if (response.status === 'success') {
              // Optional: Verify file integrity
              if (response.hash && response.size) {
                const isValid = await verifyFileIntegrity(file, response.size, response.hash);
                if (!isValid) {
                  console.warn(`[Upload] File integrity check failed for ${file.name}`);
                }
              }
              
              resolve(response);
            } else {
              reject(new Error(response.message || 'Upload failed'));
            }
          } catch (e) {
            reject(new Error('Failed to parse server response'));
          }
        } else {
          reject(new Error(`Server error: ${xhr.status} ${xhr.statusText}`));
        }
      });

      // Handle network errors with retry logic
      let retryCount = 0;
      const maxRetries = CONFIG.MAX_RETRIES;

      const errorHandler = () => {
        this.activeUploads.delete(uploadId);
        
        if (retryCount < maxRetries) {
          retryCount++;
          console.log(
            `[Upload] File ${currentIndex} network error, retrying (${retryCount}/${maxRetries})...`
          );
          
          // Exponential backoff
          setTimeout(() => {
            this.attemptUpload(xhr, formData, uploadId);
          }, CONFIG.RETRY_DELAY_BASE * Math.pow(2, retryCount - 1));
        } else {
          const backendUrl = getBackendUrl();
          reject(new Error(
            `Upload failed after ${maxRetries + 1} attempts. Make sure the backend server is running at ${backendUrl}`
          ));
        }
      };

      xhr.addEventListener('error', errorHandler);

      // Handle timeout
      xhr.timeout = CONFIG.UPLOAD_TIMEOUT;
      xhr.addEventListener('timeout', () => {
        this.activeUploads.delete(uploadId);
        reject(new Error(
          'Upload timed out. Please try again with a smaller file or check your connection.'
        ));
      });

      // Start the upload
      this.attemptUpload(xhr, formData, uploadId);
    });
  }

  /**
   * Attempt to upload with the given XHR object
   * @param {XMLHttpRequest} xhr - XMLHttpRequest object
   * @param {FormData} formData - Form data to send
   * @param {string} uploadId - Upload identifier
   */
  attemptUpload(xhr, formData, uploadId) {
    const uploadUrl = getUploadUrl();
    console.log(`[Frontend] Upload attempt to: ${uploadUrl}`);
    
    xhr.open('POST', uploadUrl);
    xhr.send(formData);
  }

  /**
   * Handle upload progress updates
   * @param {ProgressEvent} event - Progress event
   * @param {File} file - File being uploaded
   * @param {number} currentIndex - Current file index
   * @param {number} totalFiles - Total number of files
   */
  handleUploadProgress(event, file, currentIndex, totalFiles) {
    const { loaded, total } = event;
    const percentComplete = (loaded / total) * 100;
    const remaining = total - loaded;

    // Update UI progress
    this.ui.updateProgress(
      percentComplete,
      `File ${currentIndex}/${totalFiles}: ${file.name} - ${formatFileSize(loaded)} / ${formatFileSize(total)} (${percentComplete.toFixed(1)}%) - ${formatFileSize(remaining)} remaining`
    );

    // Log progress for large files
    if (total > CONFIG.LARGE_FILE_THRESHOLD) {
      const milestone = Math.floor(percentComplete / CONFIG.PROGRESS_LOG_INTERVAL) * CONFIG.PROGRESS_LOG_INTERVAL;
      if (milestone !== this.lastLoggedMilestone && milestone > 0) {
        console.log(
          `[Upload Progress] File ${currentIndex}: ${milestone}% - ${formatFileSize(loaded)}/${formatFileSize(total)}`
        );
        this.lastLoggedMilestone = milestone;
      }
    }

    // Update overall stats
    this.updateOverallProgress(loaded, total, currentIndex, totalFiles);
  }

  /**
   * Update overall upload progress across all files
   * @param {number} loaded - Bytes loaded for current file
   * @param {number} total - Total bytes for current file
   * @param {number} currentIndex - Current file index
   * @param {number} totalFiles - Total number of files
   */
  updateOverallProgress(loaded, total, currentIndex, totalFiles) {
    // Calculate bytes from completed files
    const completedFiles = currentIndex - 1;
    let completedBytes = 0;
    
    // This is a simplified calculation - in a real implementation,
    // you might want to track individual file progress more precisely
    const avgFileSize = this.uploadStats.totalBytes / totalFiles;
    completedBytes = completedFiles * avgFileSize;
    
    const currentFileProgress = loaded;
    const totalProgress = completedBytes + currentFileProgress;
    const overallPercent = (totalProgress / this.uploadStats.totalBytes) * 100;

    // Update progress text with overall statistics
    if (totalFiles > 1) {
      const progressText = document.getElementById('progressText');
      if (progressText) {
        progressText.innerHTML = `
          <div>Overall: ${this.uploadStats.uploadedFiles}/${totalFiles} files completed</div>
          <div>Current: ${formatFileSize(loaded)} / ${formatFileSize(total)} (${(loaded/total*100).toFixed(1)}%)</div>
        `;
      }
    }
  }

  /**
   * Show final upload result
   * @param {boolean} hasErrors - Whether there were any errors
   */
  showFinalResult(hasErrors) {
    const { uploadedFiles, failedFiles, totalFiles } = this.uploadStats;

    if (hasErrors) {
      this.ui.showMessage(
        `Uploaded ${uploadedFiles}/${totalFiles} files. ${failedFiles} uploads failed.`,
        'warning'
      );
    } else {
      this.ui.showMessage(
        `Successfully uploaded ${uploadedFiles} file${uploadedFiles > 1 ? 's' : ''}!`,
        'success'
      );
    }
  }

  /**
   * Cancel all active uploads
   */
  cancelAllUploads() {
    console.log(`[Upload] Cancelling ${this.activeUploads.size} active uploads`);
    
    for (const [uploadId, xhr] of this.activeUploads) {
      try {
        xhr.abort();
        console.log(`[Upload] Cancelled upload: ${uploadId}`);
      } catch (error) {
        console.error(`[Upload] Error cancelling upload ${uploadId}:`, error);
      }
    }
    
    this.activeUploads.clear();
    this.ui.setUploadingState(false);
    this.ui.showMessage('Uploads cancelled', 'warning');
  }

  /**
   * Get upload statistics
   * @returns {Object} Upload statistics
   */
  getUploadStats() {
    return { ...this.uploadStats };
  }

  /**
   * Check if uploads are currently active
   * @returns {boolean} Whether uploads are active
   */
  hasActiveUploads() {
    return this.activeUploads.size > 0;
  }
}