/**
 * UI management module for RapidComm upload interface
 */

import { formatFileSize } from './utils.js';
import { CONFIG } from './config.js';
import { animateEntrance, animateExit, createRippleEffect, shakeElement, bounceElement } from './animations.js';

/**
 * UI Manager class to handle all user interface interactions
 */
export class UIManager {
  constructor() {
    this.elements = this.initializeElements();
    this.setupEventListeners();
    this.state = {
      selectedFiles: [],
      isUploading: false,
      dragCounter: 0
    };
  }

  /**
   * Initialize and cache DOM elements
   * @returns {Object} Object containing cached DOM elements
   */
  initializeElements() {
    return {
      fileInput: document.getElementById("file"),
      fileLabel: document.getElementById("fileLabel"),
      fileInfo: document.getElementById("fileInfo"),
      filesSelected: document.getElementById("filesSelected"),
      filesList: document.getElementById("filesList"),
      submitBtn: document.getElementById("submitBtn"),
      progressContainer: document.getElementById("progressContainer"),
      progressBar: document.getElementById("progressBar"),
      progressText: document.getElementById("progressText"),
      message: document.getElementById("message"),
      uploadForm: document.querySelector(".upload-form")
    };
  }

  /**
   * Set up all event listeners
   */
  setupEventListeners() {
    // File input change
    this.elements.fileInput?.addEventListener("change", (e) => {
      this.handleFileSelection(e.target.files);
    });

    // Drag and drop events
    this.setupDragAndDrop();

    // Button click effects
    this.elements.submitBtn?.addEventListener("click", (e) => {
      createRippleEffect(e.currentTarget, e);
    });

    // Form submission is handled by the upload manager
  }

  /**
   * Set up drag and drop functionality
   */
  setupDragAndDrop() {
    if (!this.elements.fileLabel) return;

    // Prevent default drag behaviors
    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
      this.elements.fileLabel.addEventListener(eventName, this.preventDefaults, false);
      document.body.addEventListener(eventName, this.preventDefaults, false);
    });

    // Highlight drop area when item is dragged over it
    ['dragenter', 'dragover'].forEach(eventName => {
      this.elements.fileLabel.addEventListener(eventName, () => {
        this.state.dragCounter++;
        this.elements.fileLabel.classList.add('dragover');
      }, false);
    });

    ['dragleave', 'dragend'].forEach(eventName => {
      this.elements.fileLabel.addEventListener(eventName, () => {
        this.state.dragCounter--;
        if (this.state.dragCounter <= 0) {
          this.state.dragCounter = 0;
          this.elements.fileLabel.classList.remove('dragover');
        }
      }, false);
    });

    // Handle dropped files
    this.elements.fileLabel.addEventListener('drop', (e) => {
      this.state.dragCounter = 0;
      this.elements.fileLabel.classList.remove('dragover');
      const files = e.dataTransfer.files;
      this.handleFileSelection(files);
    }, false);
  }

  /**
   * Prevent default drag behaviors
   * @param {Event} e - Event object
   */
  preventDefaults(e) {
    e.preventDefault();
    e.stopPropagation();
  }

  /**
   * Handle file selection (from input or drag & drop)
   * @param {FileList} files - Selected files
   */
  handleFileSelection(files) {
    if (!files || files.length === 0) {
      this.hideFileInfo();
      this.setSubmitButtonState(false);
      return;
    }

    this.state.selectedFiles = Array.from(files);
    this.displaySelectedFiles(this.state.selectedFiles);
    this.showFileInfo();
    this.setSubmitButtonState(true);

    // Update file input with selected files (for drag & drop)
    if (this.elements.fileInput) {
      this.elements.fileInput.files = files;
    }
  }

  /**
   * Display selected files in the UI
   * @param {Array} files - Array of File objects
   */
  displaySelectedFiles(files) {
    if (!this.elements.filesSelected || !this.elements.filesList) return;

    // Calculate total size
    const totalSize = files.reduce((sum, file) => sum + file.size, 0);

    // Update files selected text
    this.elements.filesSelected.textContent = 
      `${files.length} file${files.length > 1 ? 's' : ''} selected (${formatFileSize(totalSize)} total)`;

    // Clear and populate files list
    this.elements.filesList.innerHTML = '';
    
    files.forEach((file, index) => {
      const fileDiv = document.createElement('div');
      fileDiv.className = 'file-item';
      fileDiv.innerHTML = `
        <span class="file-name">${this.escapeHtml(file.name)}</span>
        <span class="file-size">${formatFileSize(file.size)}</span>
      `;
      
      // Animate file item entrance
      fileDiv.style.opacity = '0';
      fileDiv.style.transform = 'translateX(-20px)';
      this.elements.filesList.appendChild(fileDiv);
      
      setTimeout(() => {
        animateEntrance(fileDiv, 'left', 300);
      }, index * 50);
    });
  }

  /**
   * Show file info section with animation
   */
  showFileInfo() {
    if (!this.elements.fileInfo) return;
    
    if (this.elements.fileInfo.style.display === 'none' || !this.elements.fileInfo.style.display) {
      this.elements.fileInfo.style.display = 'block';
      animateEntrance(this.elements.fileInfo, 'up', 400);
    }
  }

  /**
   * Hide file info section with animation
   */
  async hideFileInfo() {
    if (!this.elements.fileInfo) return;
    
    if (this.elements.fileInfo.style.display !== 'none') {
      await animateExit(this.elements.fileInfo, 'up', 300);
      this.elements.fileInfo.style.display = 'none';
    }
  }

  /**
   * Set submit button state
   * @param {boolean} enabled - Whether button should be enabled
   * @param {string} text - Button text
   */
  setSubmitButtonState(enabled, text = null) {
    if (!this.elements.submitBtn) return;

    this.elements.submitBtn.disabled = !enabled;
    if (text) {
      this.elements.submitBtn.textContent = text;
    } else if (!text && enabled) {
      this.elements.submitBtn.textContent = 'Upload Files';
    }

    // Add visual feedback
    if (enabled && !this.state.isUploading) {
      this.elements.submitBtn.classList.remove('loading');
    }
  }

  /**
   * Show progress container with animation
   */
  showProgress() {
    if (!this.elements.progressContainer) return;
    
    this.elements.progressContainer.style.display = 'block';
    animateEntrance(this.elements.progressContainer, 'up', 400);
  }

  /**
   * Hide progress container
   */
  hideProgress() {
    if (!this.elements.progressContainer) return;
    this.elements.progressContainer.style.display = 'none';
  }

  /**
   * Update progress bar and text
   * @param {number} percentage - Progress percentage (0-100)
   * @param {string} text - Progress text
   */
  updateProgress(percentage, text = '') {
    if (this.elements.progressBar) {
      this.elements.progressBar.style.width = `${Math.min(Math.max(percentage, 0), 100).toFixed(2)}%`;
    }
    
    if (this.elements.progressText && text) {
      this.elements.progressText.textContent = text;
    }
  }

  /**
   * Show message with animation
   * @param {string} text - Message text
   * @param {string} type - Message type ('success', 'error', 'warning')
   * @param {number} autoHideDelay - Auto hide delay in ms (0 to disable)
   */
  showMessage(text, type = 'info', autoHideDelay = CONFIG.MESSAGE_AUTO_HIDE_DELAY) {
    if (!this.elements.message) return;

    this.elements.message.textContent = text;
    this.elements.message.className = `message ${type}`;
    this.elements.message.style.display = 'block';
    
    animateEntrance(this.elements.message, 'up', 400);

    // Add appropriate animation based on type
    if (type === 'success') {
      setTimeout(() => bounceElement(this.elements.message, 5, 400), 200);
    } else if (type === 'error') {
      setTimeout(() => shakeElement(this.elements.message, 3, 400), 200);
    }

    // Auto hide after delay
    if (autoHideDelay > 0) {
      setTimeout(() => this.hideMessage(), autoHideDelay);
    }
  }

  /**
   * Hide message with animation
   */
  async hideMessage() {
    if (!this.elements.message) return;
    
    await animateExit(this.elements.message, 'down', 300);
    this.elements.message.style.display = 'none';
  }

  /**
   * Reset the form to initial state
   */
  async resetForm() {
    // Reset file input
    if (this.elements.fileInput) {
      this.elements.fileInput.value = '';
    }

    // Hide file info
    await this.hideFileInfo();

    // Hide progress
    this.hideProgress();

    // Reset progress bar
    this.updateProgress(0, '');

    // Reset submit button
    this.setSubmitButtonState(false, 'Upload Files');

    // Reset state
    this.state.selectedFiles = [];
    this.state.isUploading = false;
    this.state.dragCounter = 0;

    // Remove drag classes
    this.elements.fileLabel?.classList.remove('dragover');
  }

  /**
   * Set uploading state
   * @param {boolean} uploading - Whether currently uploading
   */
  setUploadingState(uploading) {
    this.state.isUploading = uploading;
    
    if (uploading) {
      this.elements.submitBtn?.classList.add('loading');
      this.setSubmitButtonState(false, 'Preparing uploads...');
      this.showProgress();
    } else {
      this.elements.submitBtn?.classList.remove('loading');
    }
  }

  /**
   * Get currently selected files
   * @returns {Array} Array of selected File objects
   */
  getSelectedFiles() {
    return this.state.selectedFiles;
  }

  /**
   * Escape HTML to prevent XSS
   * @param {string} text - Text to escape
   * @returns {string} Escaped text
   */
  escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
  }

  /**
   * Add loading state to submit button
   */
  addLoadingState() {
    if (!this.elements.submitBtn) return;
    
    const originalText = this.elements.submitBtn.textContent;
    this.elements.submitBtn.innerHTML = `
      <span style="opacity: 0.7;">Uploading...</span>
    `;
    this.elements.submitBtn.disabled = true;
    this.elements.submitBtn.style.cursor = 'not-allowed';
  }

  /**
   * Remove loading state from submit button
   */
  removeLoadingState() {
    if (!this.elements.submitBtn) return;
    
    this.elements.submitBtn.innerHTML = 'Upload Files';
    this.elements.submitBtn.disabled = this.state.selectedFiles.length === 0;
    this.elements.submitBtn.style.cursor = this.state.selectedFiles.length === 0 ? 'not-allowed' : 'pointer';
  }
}