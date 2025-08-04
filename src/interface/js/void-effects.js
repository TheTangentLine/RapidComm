/**
 * Visual effects module for the void page
 */

/**
 * Void Effects Manager class
 */
export class VoidEffectsManager {
  constructor() {
    this.particles = [];
    this.trails = [];
    this.cursor = null;
    this.isInitialized = false;
    this.animationFrameId = null;
    
    // Configuration
    this.config = {
      particleCount: 50,
      trailProbability: 0.3,
      trailLifetime: 500,
      maxTrails: 20
    };
  }

  /**
   * Initialize all visual effects
   */
  init() {
    if (this.isInitialized) return;

    try {
      this.createVoidParticles();
      this.initCustomCursor();
      this.setupCursorTracking();
      this.isInitialized = true;
      console.log('[VoidEffects] Visual effects initialized');
    } catch (error) {
      console.error('[VoidEffects] Failed to initialize effects:', error);
    }
  }

  /**
   * Create floating void particles
   */
  createVoidParticles() {
    const container = document.getElementById("voidParticles");
    if (!container) {
      console.warn('[VoidEffects] Void particles container not found');
      return;
    }

    // Clear existing particles
    container.innerHTML = '';
    this.particles = [];

    const colors = ["#00f5ff", "#ff0080", "#ffff00"];

    for (let i = 0; i < this.config.particleCount; i++) {
      const particle = document.createElement("div");
      particle.className = "void-particle";
      particle.style.left = Math.random() * 100 + "%";
      particle.style.animationDelay = Math.random() * 20 + "s";
      particle.style.animationDuration = Math.random() * 15 + 15 + "s";
      particle.style.background = colors[Math.floor(Math.random() * colors.length)];

      container.appendChild(particle);
      this.particles.push(particle);
    }
  }

  /**
   * Initialize custom cursor
   */
  initCustomCursor() {
    this.cursor = document.getElementById("customCursor");
    
    if (!this.cursor) {
      console.warn('[VoidEffects] Custom cursor element not found');
      return;
    }

    // Hide on mobile devices
    if (window.innerWidth <= 768) {
      this.cursor.style.display = 'none';
      return;
    }

    // Hide default cursor on desktop
    document.body.style.cursor = 'none';
  }

  /**
   * Set up cursor tracking and trail effects
   */
  setupCursorTracking() {
    if (!this.cursor || window.innerWidth <= 768) return;

    let lastTrailTime = 0;
    const trailThrottleMs = 50; // Limit trail creation frequency

    document.addEventListener("mousemove", (e) => {
      // Update cursor position
      this.cursor.style.left = e.clientX + "px";
      this.cursor.style.top = e.clientY + "px";

      // Create cursor trail with throttling
      const now = Date.now();
      if (now - lastTrailTime > trailThrottleMs && Math.random() < this.config.trailProbability) {
        this.createCursorTrail(e.clientX, e.clientY);
        lastTrailTime = now;
      }
    });

    // Hide cursor when leaving window
    document.addEventListener("mouseleave", () => {
      if (this.cursor) {
        this.cursor.style.opacity = '0';
      }
    });

    // Show cursor when entering window
    document.addEventListener("mouseenter", () => {
      if (this.cursor) {
        this.cursor.style.opacity = '0.8';
      }
    });
  }

  /**
   * Create a cursor trail effect
   * @param {number} x - X coordinate
   * @param {number} y - Y coordinate
   */
  createCursorTrail(x, y) {
    // Limit number of active trails
    if (this.trails.length >= this.config.maxTrails) {
      const oldTrail = this.trails.shift();
      if (oldTrail && oldTrail.parentNode) {
        oldTrail.parentNode.removeChild(oldTrail);
      }
    }

    const trail = document.createElement("div");
    trail.className = "cursor-trail";
    trail.style.left = x + "px";
    trail.style.top = y + "px";
    
    document.body.appendChild(trail);
    this.trails.push(trail);

    // Remove trail after animation
    setTimeout(() => {
      if (trail.parentNode) {
        trail.parentNode.removeChild(trail);
      }
      
      // Remove from trails array
      const index = this.trails.indexOf(trail);
      if (index > -1) {
        this.trails.splice(index, 1);
      }
    }, this.config.trailLifetime);
  }

  /**
   * Create explosion effect at given coordinates
   * @param {number} x - X coordinate
   * @param {number} y - Y coordinate
   * @param {number} intensity - Explosion intensity (1-10)
   */
  createExplosion(x, y, intensity = 5) {
    const colors = ["#00f5ff", "#ff0080", "#ffff00"];
    const particleCount = intensity * 3;

    for (let i = 0; i < particleCount; i++) {
      const particle = document.createElement("div");
      const size = Math.random() * 6 + 2;
      const angle = (Math.PI * 2 * i) / particleCount;
      const velocity = Math.random() * 100 + 50;
      const lifetime = Math.random() * 1000 + 500;

      particle.style.cssText = `
        position: fixed;
        width: ${size}px;
        height: ${size}px;
        background: ${colors[Math.floor(Math.random() * colors.length)]};
        border-radius: 50%;
        left: ${x}px;
        top: ${y}px;
        pointer-events: none;
        z-index: 9999;
        animation: explosion-particle ${lifetime}ms ease-out forwards;
        --angle: ${angle}rad;
        --velocity: ${velocity}px;
      `;

      document.body.appendChild(particle);

      // Remove particle after animation
      setTimeout(() => {
        if (particle.parentNode) {
          particle.parentNode.removeChild(particle);
        }
      }, lifetime);
    }

    // Add explosion animation keyframes if not already present
    this.ensureExplosionStyles();
  }

  /**
   * Ensure explosion animation styles are present
   */
  ensureExplosionStyles() {
    if (document.querySelector('#explosion-styles')) return;

    const style = document.createElement('style');
    style.id = 'explosion-styles';
    style.textContent = `
      @keyframes explosion-particle {
        0% {
          transform: translate(0, 0) scale(1);
          opacity: 1;
        }
        100% {
          transform: translate(
            calc(cos(var(--angle)) * var(--velocity)),
            calc(sin(var(--angle)) * var(--velocity))
          ) scale(0);
          opacity: 0;
        }
      }
    `;
    document.head.appendChild(style);
  }

  /**
   * Create ripple effect at given coordinates
   * @param {number} x - X coordinate  
   * @param {number} y - Y coordinate
   * @param {string} color - Ripple color
   */
  createRipple(x, y, color = '#00f5ff') {
    const ripple = document.createElement('div');
    ripple.style.cssText = `
      position: fixed;
      width: 0;
      height: 0;
      left: ${x}px;
      top: ${y}px;
      border: 2px solid ${color};
      border-radius: 50%;
      pointer-events: none;
      z-index: 9998;
      animation: ripple-expand 1s ease-out forwards;
      transform: translate(-50%, -50%);
    `;

    document.body.appendChild(ripple);

    // Ensure ripple styles exist
    this.ensureRippleStyles();

    // Remove ripple after animation
    setTimeout(() => {
      if (ripple.parentNode) {
        ripple.parentNode.removeChild(ripple);
      }
    }, 1000);
  }

  /**
   * Ensure ripple animation styles are present
   */
  ensureRippleStyles() {
    if (document.querySelector('#ripple-styles')) return;

    const style = document.createElement('style');
    style.id = 'ripple-styles';
    style.textContent = `
      @keyframes ripple-expand {
        0% {
          width: 0;
          height: 0;
          opacity: 1;
        }
        100% {
          width: 300px;
          height: 300px;
          opacity: 0;
        }
      }
    `;
    document.head.appendChild(style);
  }

  /**
   * Add new floating shapes dynamically
   * @param {number} count - Number of shapes to add
   */
  addFloatingShapes(count = 3) {
    const container = document.querySelector(".floating-elements");
    if (!container) return;

    const colors = [
      "rgba(0, 245, 255, 0.3)",
      "rgba(255, 0, 128, 0.3)",
      "rgba(255, 255, 0, 0.3)",
    ];

    for (let i = 0; i < count; i++) {
      const shape = document.createElement("div");
      shape.className = "floating-shape";
      
      const size = Math.random() * 60 + 40;
      shape.style.width = size + "px";
      shape.style.height = size + "px";
      shape.style.top = Math.random() * 80 + "%";
      shape.style.left = Math.random() * 80 + "%";
      shape.style.borderRadius = Math.random() > 0.5 ? "50%" : "0";
      shape.style.borderColor = colors[Math.floor(Math.random() * colors.length)];
      shape.style.animationDelay = Math.random() * 20 + "s";

      container.appendChild(shape);

      // Remove shape after some time
      setTimeout(() => {
        if (shape.parentNode) {
          shape.parentNode.removeChild(shape);
        }
      }, 20000);
    }
  }

  /**
   * Trigger screen flash effect
   * @param {string} color - Flash color
   * @param {number} duration - Flash duration in ms
   */
  flashScreen(color = 'rgba(0, 245, 255, 0.1)', duration = 200) {
    const flash = document.createElement('div');
    flash.style.cssText = `
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: ${color};
      pointer-events: none;
      z-index: 10000;
      animation: screen-flash ${duration}ms ease-out forwards;
    `;

    document.body.appendChild(flash);

    // Ensure flash styles exist
    this.ensureFlashStyles();

    // Remove flash after animation
    setTimeout(() => {
      if (flash.parentNode) {
        flash.parentNode.removeChild(flash);
      }
    }, duration);
  }

  /**
   * Ensure flash animation styles are present
   */
  ensureFlashStyles() {
    if (document.querySelector('#flash-styles')) return;

    const style = document.createElement('style');
    style.id = 'flash-styles';
    style.textContent = `
      @keyframes screen-flash {
        0% { opacity: 1; }
        100% { opacity: 0; }
      }
    `;
    document.head.appendChild(style);
  }

  /**
   * Cleanup all effects and event listeners
   */
  cleanup() {
    // Stop any running animations
    if (this.animationFrameId) {
      cancelAnimationFrame(this.animationFrameId);
    }

    // Clear trails
    this.trails.forEach(trail => {
      if (trail.parentNode) {
        trail.parentNode.removeChild(trail);
      }
    });
    this.trails = [];

    // Reset cursor
    if (this.cursor) {
      document.body.style.cursor = 'auto';
    }

    this.isInitialized = false;
    console.log('[VoidEffects] Effects cleaned up');
  }

  /**
   * Check if reduced motion is preferred
   * @returns {boolean} Whether reduced motion is preferred
   */
  prefersReducedMotion() {
    return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  }
}