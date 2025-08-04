/**
 * Main application module for RapidComm upload interface
 */

import { CONFIG, logConfiguration } from './config.js';
import { UIManager } from './ui.js';
import { UploadManager } from './upload.js';
import { initializeAnimations } from './animations.js';

/**
 * Main Application class
 */
class RapidCommApp {
  constructor() {
    this.ui = null;
    this.uploadManager = null;
    this.isInitialized = false;
  }

  /**
   * Initialize the application
   */
  async init() {
    try {
      console.log('[App] Initializing RapidComm Upload Interface...');
      
      // Wait for DOM to be ready
      if (document.readyState === 'loading') {
        await new Promise(resolve => {
          document.addEventListener('DOMContentLoaded', resolve);
        });
      }

      // Initialize managers
      this.ui = new UIManager();
      this.uploadManager = new UploadManager(this.ui);

      // Set up form submission handler
      this.setupFormHandler();

      // Initialize animations and effects
      initializeAnimations();

      // Set up global error handlers
      this.setupErrorHandlers();

      // Set up keyboard shortcuts
      this.setupKeyboardShortcuts();

      // Set up cleanup on page unload
      this.setupCleanup();

      // Log configuration
      logConfiguration();

      this.isInitialized = true;
      console.log('[App] RapidComm Upload Interface initialized successfully');

    } catch (error) {
      console.error('[App] Failed to initialize application:', error);
      this.showFallbackError(error);
    }
  }

  /**
   * Set up form submission handler
   */
  setupFormHandler() {
    const form = this.ui.elements.uploadForm;
    if (!form) {
      console.warn('[App] Upload form not found');
      return;
    }

    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      await this.handleFormSubmission();
    });
  }

  /**
   * Handle form submission
   */
  async handleFormSubmission() {
    try {
      const selectedFiles = this.ui.getSelectedFiles();
      
      if (!selectedFiles || selectedFiles.length === 0) {
        this.ui.showMessage('Please select files to upload', 'error');
        return;
      }

      console.log(`[App] Starting upload of ${selectedFiles.length} files`);
      
      // Validate files before upload
      const validationResult = this.validateFiles(selectedFiles);
      if (!validationResult.valid) {
        this.ui.showMessage(validationResult.message, 'error');
        return;
      }

      // Start upload process
      const results = await this.uploadManager.uploadFiles(selectedFiles);
      
      console.log(`[App] Upload process completed:`, results);
      
      // Reset form after successful upload
      setTimeout(async () => {
        await this.ui.resetForm();
      }, 2000);

    } catch (error) {
      console.error('[App] Upload error:', error);
      this.ui.showMessage(
        `Upload failed: ${error.message}`,
        'error'
      );
      
      // Reset form state on error
      setTimeout(async () => {
        await this.ui.resetForm();
      }, 1000);
    }
  }

  /**
   * Validate selected files
   * @param {Array} files - Array of File objects
   * @returns {Object} Validation result
   */
  validateFiles(files) {
    // Check if files array is valid
    if (!Array.isArray(files) || files.length === 0) {
      return {
        valid: false,
        message: 'No files selected'
      };
    }

    // Check file count limit (optional)
    const maxFiles = 50; // Reasonable limit
    if (files.length > maxFiles) {
      return {
        valid: false,
        message: `Too many files selected. Maximum ${maxFiles} files allowed.`
      };
    }

    // Check total size limit (optional)
    const maxTotalSize = 1024 * 1024 * 1024; // 1GB total
    const totalSize = files.reduce((sum, file) => sum + file.size, 0);
    if (totalSize > maxTotalSize) {
      return {
        valid: false,
        message: 'Total file size exceeds 1GB limit'
      };
    }

    // Check individual file size limit (optional)
    const maxFileSize = 100 * 1024 * 1024; // 100MB per file
    const oversizedFile = files.find(file => file.size > maxFileSize);
    if (oversizedFile) {
      return {
        valid: false,
        message: `File "${oversizedFile.name}" exceeds 100MB limit`
      };
    }

    // Check for empty files
    const emptyFile = files.find(file => file.size === 0);
    if (emptyFile) {
      return {
        valid: false,
        message: `File "${emptyFile.name}" is empty`
      };
    }

    return { valid: true };
  }

  /**
   * Set up global error handlers
   */
  setupErrorHandlers() {
    // Handle unhandled promise rejections
    window.addEventListener('unhandledrejection', (event) => {
      console.error('[App] Unhandled promise rejection:', event.reason);
      
      // Prevent default browser handling
      event.preventDefault();
      
      // Show user-friendly error message
      this.ui.showMessage(
        'An unexpected error occurred. Please try again.',
        'error'
      );
    });

    // Handle general JavaScript errors
    window.addEventListener('error', (event) => {
      console.error('[App] JavaScript error:', event.error);
      
      // Don't show error message for every JS error as it might be too intrusive
      // Only log to console for debugging
    });
  }

  /**
   * Set up keyboard shortcuts
   */
  setupKeyboardShortcuts() {
    document.addEventListener('keydown', (e) => {
      // Ctrl/Cmd + U: Focus file input
      if ((e.ctrlKey || e.metaKey) && e.key === 'u') {
        e.preventDefault();
        this.ui.elements.fileInput?.click();
      }

      // Escape: Cancel uploads or reset form
      if (e.key === 'Escape') {
        if (this.uploadManager.hasActiveUploads()) {
          this.uploadManager.cancelAllUploads();
        } else {
          this.ui.resetForm();
        }
      }

      // Enter: Submit form (when files are selected)
      if (e.key === 'Enter' && !e.shiftKey && !e.ctrlKey && !e.altKey) {
        const activeElement = document.activeElement;
        
        // Only submit if not focused on an input that should handle Enter normally
        if (activeElement.tagName !== 'INPUT' && activeElement.tagName !== 'TEXTAREA') {
          const selectedFiles = this.ui.getSelectedFiles();
          if (selectedFiles.length > 0 && !this.uploadManager.hasActiveUploads()) {
            e.preventDefault();
            this.handleFormSubmission();
          }
        }
      }
    });
  }

  /**
   * Set up cleanup on page unload
   */
  setupCleanup() {
    window.addEventListener('beforeunload', (e) => {
      if (this.uploadManager.hasActiveUploads()) {
        // Show confirmation dialog if uploads are in progress
        e.preventDefault();
        e.returnValue = 'Uploads are in progress. Are you sure you want to leave?';
        return e.returnValue;
      }
    });

    // Clean up resources on page unload
    window.addEventListener('unload', () => {
      if (this.uploadManager) {
        this.uploadManager.cancelAllUploads();
      }
    });
  }

  /**
   * Show fallback error when app fails to initialize
   * @param {Error} error - The initialization error
   */
  showFallbackError(error) {
    const errorHtml = `
      <div style="
        position: fixed;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        background: #ff6b6b;
        color: white;
        padding: 2rem;
        border-radius: 12px;
        box-shadow: 0 10px 25px rgba(255, 107, 107, 0.3);
        text-align: center;
        z-index: 10000;
        max-width: 400px;
      ">
        <h3>Application Error</h3>
        <p>Failed to initialize the upload interface.</p>
        <p style="font-size: 0.9em; margin-top: 1rem; opacity: 0.8;">
          Please refresh the page or contact support if the issue persists.
        </p>
        <button onclick="window.location.reload()" style="
          margin-top: 1rem;
          padding: 0.5rem 1rem;
          background: rgba(255, 255, 255, 0.2);
          border: 1px solid rgba(255, 255, 255, 0.3);
          color: white;
          border-radius: 6px;
          cursor: pointer;
        ">
          Refresh Page
        </button>
      </div>
    `;
    
    document.body.insertAdjacentHTML('beforeend', errorHtml);
  }

  /**
   * Get app status and statistics
   * @returns {Object} App status
   */
  getStatus() {
    return {
      initialized: this.isInitialized,
      hasActiveUploads: this.uploadManager?.hasActiveUploads() || false,
      uploadStats: this.uploadManager?.getUploadStats() || null,
      selectedFiles: this.ui?.getSelectedFiles()?.length || 0
    };
  }
}

// Initialize and start the application
const app = new RapidCommApp();

// Start initialization when script loads
app.init().catch(error => {
  console.error('[App] Failed to start application:', error);
});

// Export for debugging and testing
window.RapidCommApp = app;

export default app;