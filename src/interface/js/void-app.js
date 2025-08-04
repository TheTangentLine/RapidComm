/**
 * Main application module for the void page
 */

import { VoidEffectsManager } from './void-effects.js';
import { VoidInteractionsManager } from './void-interactions.js';

/**
 * Void Application class
 */
class VoidApp {
  constructor() {
    this.effects = null;
    this.interactions = null;
    this.isInitialized = false;
    this.startTime = Date.now();
    
    // Configuration
    this.config = {
      enableEffects: true,
      enableInteractions: true,
      respectReducedMotion: true
    };
  }

  /**
   * Initialize the void application
   */
  async init() {
    try {
      console.log('[VoidApp] Initializing void experience...');
      
      // Wait for DOM to be ready
      if (document.readyState === 'loading') {
        await new Promise(resolve => {
          document.addEventListener('DOMContentLoaded', resolve);
        });
      }

      // Check for reduced motion preference
      if (this.config.respectReducedMotion && this.prefersReducedMotion()) {
        this.config.enableEffects = false;
        console.log('[VoidApp] Reduced motion detected, disabling effects');
      }

      // Initialize effects manager
      if (this.config.enableEffects) {
        this.effects = new VoidEffectsManager();
        this.effects.init();
      }

      // Initialize interactions manager
      if (this.config.enableInteractions) {
        this.interactions = new VoidInteractionsManager(this.effects);
        this.interactions.init();
      }

      // Set up global error handlers
      this.setupErrorHandlers();

      // Set up cleanup on page unload
      this.setupCleanup();

      // Log startup info
      this.logStartupInfo();

      this.isInitialized = true;
      console.log('[VoidApp] Void experience initialized successfully');

      // Optional: Show debug info in development
      if (this.isDevelopment()) {
        this.setupDebugMode();
      }

    } catch (error) {
      console.error('[VoidApp] Failed to initialize void experience:', error);
      this.showFallbackMessage();
    }
  }

  /**
   * Set up global error handlers
   */
  setupErrorHandlers() {
    // Handle unhandled promise rejections
    window.addEventListener('unhandledrejection', (event) => {
      console.error('[VoidApp] Unhandled promise rejection:', event.reason);
      event.preventDefault();
    });

    // Handle general JavaScript errors
    window.addEventListener('error', (event) => {
      console.error('[VoidApp] JavaScript error:', event.error);
    });
  }

  /**
   * Set up cleanup on page unload
   */
  setupCleanup() {
    window.addEventListener('beforeunload', () => {
      this.cleanup();
    });

    window.addEventListener('unload', () => {
      this.cleanup();
    });

    // Handle visibility change (tab switching)
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        this.handlePageHidden();
      } else {
        this.handlePageVisible();
      }
    });
  }

  /**
   * Handle page becoming hidden (tab switch, minimize)
   */
  handlePageHidden() {
    // Pause effects to save resources
    if (this.effects && typeof this.effects.pause === 'function') {
      this.effects.pause();
    }
    console.log('[VoidApp] Page hidden, pausing effects');
  }

  /**
   * Handle page becoming visible again
   */
  handlePageVisible() {
    // Resume effects
    if (this.effects && typeof this.effects.resume === 'function') {
      this.effects.resume();
    }
    console.log('[VoidApp] Page visible, resuming effects');
  }

  /**
   * Log startup information
   */
  logStartupInfo() {
    const loadTime = Date.now() - this.startTime;
    console.log('========================================');
    console.log('  RapidComm Void Experience');
    console.log('========================================');
    console.log(`Load time: ${loadTime}ms`);
    console.log(`Effects enabled: ${this.config.enableEffects}`);
    console.log(`Interactions enabled: ${this.config.enableInteractions}`);
    console.log(`Reduced motion: ${this.prefersReducedMotion()}`);
    console.log(`Screen size: ${window.innerWidth}x${window.innerHeight}`);
    console.log(`User agent: ${navigator.userAgent.substring(0, 50)}...`);
    console.log('========================================');
  }

  /**
   * Set up debug mode for development
   */
  setupDebugMode() {
    // Add debug panel
    const debugPanel = document.createElement('div');
    debugPanel.id = 'debug-panel';
    debugPanel.style.cssText = `
      position: fixed;
      top: 10px;
      right: 10px;
      background: rgba(0, 0, 0, 0.8);
      color: #00f5ff;
      padding: 10px;
      border-radius: 5px;
      font-family: monospace;
      font-size: 12px;
      z-index: 10000;
      max-width: 200px;
      display: none;
    `;

    document.body.appendChild(debugPanel);

    // Toggle debug panel with Ctrl+D
    document.addEventListener('keydown', (e) => {
      if (e.ctrlKey && e.key === 'd') {
        e.preventDefault();
        debugPanel.style.display = 
          debugPanel.style.display === 'none' ? 'block' : 'none';
      }
    });

    // Update debug info periodically
    setInterval(() => {
      if (debugPanel.style.display !== 'none') {
        this.updateDebugInfo(debugPanel);
      }
    }, 1000);

    // Add to global for console access
    window.voidApp = this;
    
    console.log('[VoidApp] Debug mode enabled. Press Ctrl+D to toggle debug panel');
  }

  /**
   * Update debug information
   * @param {HTMLElement} panel - Debug panel element
   */
  updateDebugInfo(panel) {
    const state = this.interactions ? this.interactions.getState() : {};
    const uptime = Math.floor((Date.now() - this.startTime) / 1000);
    
    panel.innerHTML = `
      <div><strong>Void Debug</strong></div>
      <div>Uptime: ${uptime}s</div>
      <div>Clicks: ${state.clickCount || 0}</div>
      <div>Message: ${state.messageIndex || 0}</div>
      <div>Konami: ${state.isKonamiActive ? 'Yes' : 'No'}</div>
      <div>FPS: ${this.getCurrentFPS()}</div>
      <div>Memory: ${this.getMemoryUsage()}MB</div>
    `;
  }

  /**
   * Get current FPS (approximate)
   * @returns {number} Approximate FPS
   */
  getCurrentFPS() {
    // This is a simplified FPS counter
    // In a real implementation, you'd track frame times
    return 60; // Placeholder
  }

  /**
   * Get memory usage if available
   * @returns {string} Memory usage or 'N/A'
   */
  getMemoryUsage() {
    if (performance.memory) {
      return Math.round(performance.memory.usedJSHeapSize / 1024 / 1024);
    }
    return 'N/A';
  }

  /**
   * Show fallback message if app fails to initialize
   */
  showFallbackMessage() {
    const fallback = document.createElement('div');
    fallback.style.cssText = `
      position: fixed;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      background: #1a0d2e;
      color: #00f5ff;
      padding: 2rem;
      border-radius: 12px;
      text-align: center;
      z-index: 10000;
      font-family: 'Orbitron', monospace;
      border: 1px solid #00f5ff;
    `;
    
    fallback.innerHTML = `
      <h3>Void Experience Error</h3>
      <p>The void failed to initialize properly.</p>
      <p style="font-size: 0.9em; margin-top: 1rem; opacity: 0.8;">
        But perhaps this too is part of the void...
      </p>
      <button onclick="window.location.reload()" style="
        margin-top: 1rem;
        padding: 0.5rem 1rem;
        background: transparent;
        border: 1px solid #00f5ff;
        color: #00f5ff;
        border-radius: 6px;
        cursor: pointer;
        font-family: 'Orbitron', monospace;
      ">
        Restart Void
      </button>
    `;
    
    document.body.appendChild(fallback);
  }

  /**
   * Check if user prefers reduced motion
   * @returns {boolean} Whether reduced motion is preferred
   */
  prefersReducedMotion() {
    return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  }

  /**
   * Check if running in development mode
   * @returns {boolean} Whether in development mode
   */
  isDevelopment() {
    return window.location.hostname === 'localhost' || 
           window.location.hostname === '127.0.0.1' ||
           window.location.search.includes('debug=true');
  }

  /**
   * Get application status
   * @returns {Object} Application status
   */
  getStatus() {
    return {
      initialized: this.isInitialized,
      uptime: Date.now() - this.startTime,
      effectsEnabled: this.config.enableEffects,
      interactionsEnabled: this.config.enableInteractions,
      reducedMotion: this.prefersReducedMotion(),
      interactionState: this.interactions ? this.interactions.getState() : null
    };
  }

  /**
   * Manually trigger a specific effect
   * @param {string} effectName - Name of effect to trigger
   * @param {Object} params - Effect parameters
   */
  triggerEffect(effectName, params = {}) {
    if (!this.effects || !this.interactions) {
      console.warn('[VoidApp] Effects or interactions not initialized');
      return;
    }

    switch (effectName) {
      case 'exploreVoid':
        this.interactions.exploreVoid();
        break;
      case 'createSomething':
        this.interactions.createSomething();
        break;
      case 'embraceNothing':
        this.interactions.embraceNothing();
        break;
      case 'explosion':
        const x = params.x || window.innerWidth / 2;
        const y = params.y || window.innerHeight / 2;
        const intensity = params.intensity || 5;
        this.effects.createExplosion(x, y, intensity);
        break;
      case 'ripple':
        const rippleX = params.x || window.innerWidth / 2;
        const rippleY = params.y || window.innerHeight / 2;
        const color = params.color || '#00f5ff';
        this.effects.createRipple(rippleX, rippleY, color);
        break;
      default:
        console.warn(`[VoidApp] Unknown effect: ${effectName}`);
    }
  }

  /**
   * Cleanup all resources
   */
  cleanup() {
    if (this.effects && typeof this.effects.cleanup === 'function') {
      this.effects.cleanup();
    }
    
    if (this.interactions && typeof this.interactions.cleanup === 'function') {
      this.interactions.cleanup();
    }
    
    console.log('[VoidApp] Void experience cleaned up');
  }
}

// Initialize and start the void application
const voidApp = new VoidApp();

// Start initialization when script loads
voidApp.init().catch(error => {
  console.error('[VoidApp] Failed to start void experience:', error);
});

// Export for debugging and external access
window.VoidApp = voidApp;

export default voidApp;