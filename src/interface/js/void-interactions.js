/**
 * Interactive behaviors module for the void page
 */

import { VoidEffectsManager } from './void-effects.js';

/**
 * Void Interactions Manager class
 */
export class VoidInteractionsManager {
  constructor(effectsManager) {
    this.effects = effectsManager || new VoidEffectsManager();
    this.elements = {};
    this.state = {
      messageIndex: 0,
      isKonamiActive: false,
      konamiCode: [],
      clickCount: 0
    };
    
    this.messages = [
      "You found... more nothing!",
      "The void stares back at you",
      "Nothing becomes everything",
      "In nothingness, infinite possibilities exist",
      "There is nothing to explore here",
      "Welcome to the space between spaces",
      "Emptiness is fullness in disguise",
      "You have discovered the art of digital zen"
    ];

    this.konamiSequence = [
      "ArrowUp", "ArrowUp", "ArrowDown", "ArrowDown",
      "ArrowLeft", "ArrowRight", "ArrowLeft", "ArrowRight",
      "KeyB", "KeyA"
    ];
  }

  /**
   * Initialize all interactions
   */
  init() {
    try {
      this.cacheElements();
      this.setupEventListeners();
      this.setupKeyboardControls();
      this.setupEasterEggs();
      console.log('[VoidInteractions] Interactions initialized');
    } catch (error) {
      console.error('[VoidInteractions] Failed to initialize interactions:', error);
    }
  }

  /**
   * Cache DOM elements for better performance
   */
  cacheElements() {
    this.elements = {
      mainHeading: document.getElementById("mainHeading"),
      goHomeBtn: document.querySelector('[href="/"]'),
      body: document.body,
      hiddenMessage: document.querySelector('.hidden-message')
    };
  }

  /**
   * Set up all event listeners
   */
  setupEventListeners() {
    // Main heading interactions
    if (this.elements.mainHeading) {
      this.elements.mainHeading.addEventListener('click', (e) => {
        this.handleHeadingClick(e);
      });

      this.elements.mainHeading.addEventListener('mouseenter', () => {
        this.handleHeadingHover();
      });
    }

    // Button interactions
    if (this.elements.goHomeBtn) {
      this.elements.goHomeBtn.addEventListener('click', (e) => {
        this.handleButtonClick(e);
      });
    }

    // Document-wide click effects
    document.addEventListener('click', (e) => {
      this.handleDocumentClick(e);
    });

    // Touch events for mobile
    if ('ontouchstart' in window) {
      this.setupTouchEvents();
    }
  }

  /**
   * Set up keyboard controls and shortcuts
   */
  setupKeyboardControls() {
    document.addEventListener('keydown', (e) => {
      this.handleKeyPress(e);
    });

    // Handle Konami code
    document.addEventListener('keydown', (e) => {
      this.handleKonamiCode(e);
    });
  }

  /**
   * Set up easter eggs and hidden interactions
   */
  setupEasterEggs() {
    // Secret click sequence
    let secretClicks = 0;
    const secretSequence = 7;

    if (this.elements.mainHeading) {
      this.elements.mainHeading.addEventListener('click', () => {
        secretClicks++;
        if (secretClicks === secretSequence) {
          this.triggerSecretMode();
          secretClicks = 0;
        }
        
        // Reset counter after timeout
        setTimeout(() => {
          secretClicks = 0;
        }, 3000);
      });
    }

    // Time-based easter egg
    setTimeout(() => {
      this.revealHiddenTruth();
    }, 30000); // After 30 seconds
  }

  /**
   * Handle main heading click
   * @param {Event} e - Click event
   */
  handleHeadingClick(e) {
    const rect = e.target.getBoundingClientRect();
    const x = e.clientX;
    const y = e.clientY;

    // Create explosion effect at click point
    this.effects.createExplosion(x, y, 3);
    
    // Change heading text
    this.exploreVoid();
    
    // Create ripple effect
    this.effects.createRipple(x, y, '#ff0080');
    
    // Flash screen
    this.effects.flashScreen('rgba(255, 0, 128, 0.1)', 150);

    this.state.clickCount++;
    
    // Special effect after multiple clicks
    if (this.state.clickCount % 5 === 0) {
      this.triggerMultiClickEffect();
    }
  }

  /**
   * Handle heading hover
   */
  handleHeadingHover() {
    if (this.elements.mainHeading) {
      // Add subtle glitch effect
      this.elements.mainHeading.style.animation = 'glitchText 0.3s ease-in-out';
      
      setTimeout(() => {
        this.elements.mainHeading.style.animation = 
          'gradientShift 3s ease-in-out infinite, textGlow 2s ease-in-out infinite alternate';
      }, 300);
    }
  }

  /**
   * Handle button clicks with effects
   * @param {Event} e - Click event
   */
  handleButtonClick(e) {
    const x = e.clientX;
    const y = e.clientY;
    
    // Create ripple effect
    this.effects.createRipple(x, y, '#00f5ff');
    
    // Add button animation
    const button = e.target;
    button.style.transform = 'scale(0.95)';
    setTimeout(() => {
      button.style.transform = '';
    }, 150);
  }

  /**
   * Handle document-wide clicks for ambient effects
   * @param {Event} e - Click event
   */
  handleDocumentClick(e) {
    // Don't trigger for button clicks
    if (e.target.classList.contains('void-btn') || 
        e.target.id === 'mainHeading') {
      return;
    }

    // Random chance for ambient effects
    if (Math.random() < 0.3) {
      const x = e.clientX;
      const y = e.clientY;
      
      this.effects.createRipple(x, y, 'rgba(255, 255, 0, 0.5)');
    }
  }

  /**
   * Set up touch events for mobile devices
   */
  setupTouchEvents() {
    let touchStartTime = 0;
    
    document.addEventListener('touchstart', (e) => {
      touchStartTime = Date.now();
    });

    document.addEventListener('touchend', (e) => {
      const touchDuration = Date.now() - touchStartTime;
      
      // Long press (> 800ms) triggers special effect
      if (touchDuration > 800) {
        const touch = e.changedTouches[0];
        this.effects.createExplosion(touch.clientX, touch.clientY, 2);
      }
    });

    // Gesture detection
    let startTouch = null;
    
    document.addEventListener('touchstart', (e) => {
      startTouch = e.touches[0];
    });

    document.addEventListener('touchmove', (e) => {
      if (!startTouch) return;
      
      const currentTouch = e.touches[0];
      const deltaX = currentTouch.clientX - startTouch.clientX;
      const deltaY = currentTouch.clientY - startTouch.clientY;
      
      // Swipe detection
      if (Math.abs(deltaX) > 50 || Math.abs(deltaY) > 50) {
        this.handleSwipeGesture(deltaX, deltaY);
        startTouch = null;
      }
    });
  }

  /**
   * Handle swipe gestures
   * @param {number} deltaX - Horizontal swipe distance
   * @param {number} deltaY - Vertical swipe distance
   */
  handleSwipeGesture(deltaX, deltaY) {
    if (Math.abs(deltaX) > Math.abs(deltaY)) {
      // Horizontal swipe
      if (deltaX > 0) {
        this.createSomething();
      } else {
        this.embraceNothing();
      }
    } else {
      // Vertical swipe
      if (deltaY > 0) {
        this.exploreVoid();
      } else {
        this.triggerUpwardSwipe();
      }
    }
  }

  /**
   * Handle key press events
   * @param {KeyboardEvent} e - Keyboard event
   */
  handleKeyPress(e) {
    switch (e.key) {
      case ' ': // Spacebar
        e.preventDefault();
        this.exploreVoid();
        break;
      case 'Enter':
        this.createSomething();
        break;
      case 'Escape':
        this.resetToDefault();
        break;
      case 'c':
      case 'C':
        if (e.ctrlKey || e.metaKey) return; // Don't interfere with copy
        this.createSomething();
        break;
      case 'e':
      case 'E':
        this.embraceNothing();
        break;
      case 'r':
      case 'R':
        if (e.ctrlKey || e.metaKey) return; // Don't interfere with refresh
        this.randomEffect();
        break;
    }
  }

  /**
   * Handle Konami code input
   * @param {KeyboardEvent} e - Keyboard event
   */
  handleKonamiCode(e) {
    this.state.konamiCode.push(e.code);
    
    if (this.state.konamiCode.length > this.konamiSequence.length) {
      this.state.konamiCode.shift();
    }

    if (JSON.stringify(this.state.konamiCode) === JSON.stringify(this.konamiSequence)) {
      this.triggerKonamiEffect();
      this.state.konamiCode = [];
    }
  }

  /**
   * Explore void effect - changes heading text
   */
  exploreVoid() {
    if (!this.elements.mainHeading) return;

    const heading = this.elements.mainHeading;
    const currentMessage = this.messages[this.state.messageIndex];
    
    // Glitch effect during text change
    heading.style.animation = "glitchText 0.5s ease-in-out";
    
    setTimeout(() => {
      heading.textContent = currentMessage;
      heading.style.animation = 
        "gradientShift 3s ease-in-out infinite, textGlow 2s ease-in-out infinite alternate";
      
      // Cycle through messages
      this.state.messageIndex = (this.state.messageIndex + 1) % this.messages.length;
    }, 250);
  }

  /**
   * Create something effect - adds floating shapes
   */
  createSomething() {
    this.effects.addFloatingShapes(Math.floor(Math.random() * 3) + 1);
    
    // Update heading
    if (this.elements.mainHeading) {
      this.elements.mainHeading.textContent = "Something manifests from nothing...";
    }
    
    // Flash effect
    this.effects.flashScreen('rgba(0, 245, 255, 0.1)', 300);
  }

  /**
   * Embrace nothing effect - particle explosion
   */
  embraceNothing() {
    if (!this.elements.mainHeading) return;
    
    // Change heading
    this.elements.mainHeading.textContent = "Nothing embraces you back";
    
    // Body animation
    if (this.elements.body) {
      this.elements.body.style.animation = "voidShift 2s ease-in-out";
    }

    // Create multiple particle explosions
    for (let i = 0; i < 20; i++) {
      setTimeout(() => {
        const x = Math.random() * window.innerWidth;
        const y = Math.random() * window.innerHeight;
        this.effects.createExplosion(x, y, 1);
      }, i * 100);
    }
  }

  /**
   * Trigger random effect
   */
  randomEffect() {
    const effects = [
      () => this.exploreVoid(),
      () => this.createSomething(),
      () => this.embraceNothing(),
      () => this.triggerMultiClickEffect()
    ];
    
    const randomEffect = effects[Math.floor(Math.random() * effects.length)];
    randomEffect();
  }

  /**
   * Trigger upward swipe effect
   */
  triggerUpwardSwipe() {
    if (this.elements.mainHeading) {
      this.elements.mainHeading.textContent = "You reach toward the infinite...";
    }
    
    // Create upward-moving particles
    for (let i = 0; i < 10; i++) {
      setTimeout(() => {
        const x = Math.random() * window.innerWidth;
        const y = window.innerHeight;
        this.effects.createExplosion(x, y, 2);
      }, i * 50);
    }
  }

  /**
   * Trigger multi-click effect
   */
  triggerMultiClickEffect() {
    if (this.elements.mainHeading) {
      this.elements.mainHeading.textContent = "The void responds to your persistence...";
    }
    
    // Screen flash
    this.effects.flashScreen('rgba(255, 0, 128, 0.2)', 500);
    
    // Multiple explosions
    const centerX = window.innerWidth / 2;
    const centerY = window.innerHeight / 2;
    
    for (let i = 0; i < 8; i++) {
      const angle = (Math.PI * 2 * i) / 8;
      const radius = 100;
      const x = centerX + Math.cos(angle) * radius;
      const y = centerY + Math.sin(angle) * radius;
      
      setTimeout(() => {
        this.effects.createExplosion(x, y, 4);
      }, i * 100);
    }
  }

  /**
   * Trigger Konami code effect
   */
  triggerKonamiEffect() {
    if (this.elements.mainHeading) {
      this.elements.mainHeading.textContent = 
        "You found the secret! There was something here all along...";
    }
    
    // Change body background
    if (this.elements.body) {
      this.elements.body.style.background = 
        "linear-gradient(45deg, #ff0080, #00f5ff, #ffff00)";
    }
    
    // Massive explosion at center
    const centerX = window.innerWidth / 2;
    const centerY = window.innerHeight / 2;
    this.effects.createExplosion(centerX, centerY, 10);
    
    this.state.isKonamiActive = true;
  }

  /**
   * Trigger secret mode
   */
  triggerSecretMode() {
    if (this.elements.mainHeading) {
      this.elements.mainHeading.textContent = 
        "You have unlocked the secret of digital nothingness";
    }
    
    // Create rainbow effect
    const colors = ['#ff0080', '#00f5ff', '#ffff00', '#ff8000', '#8000ff'];
    
    colors.forEach((color, index) => {
      setTimeout(() => {
        this.effects.flashScreen(`${color}20`, 200);
      }, index * 300);
    });
    
    // Add many floating shapes
    this.effects.addFloatingShapes(10);
  }

  /**
   * Reveal hidden truth after time
   */
  revealHiddenTruth() {
    if (this.elements.hiddenMessage) {
      this.elements.hiddenMessage.textContent = 
        "The real treasure was the void you experienced along the way...";
      this.elements.hiddenMessage.style.animation = 
        "mysteriousAppear 1s ease-in-out forwards";
    }
  }

  /**
   * Reset to default state
   */
  resetToDefault() {
    if (this.elements.mainHeading) {
      this.elements.mainHeading.textContent = "There is nothing to explore here";
    }
    
    if (this.elements.body && this.state.isKonamiActive) {
      this.elements.body.style.background = '';
      this.state.isKonamiActive = false;
    }
    
    this.state.messageIndex = 0;
    this.state.clickCount = 0;
  }

  /**
   * Get current interaction state
   * @returns {Object} Current state
   */
  getState() {
    return { ...this.state };
  }

  /**
   * Cleanup interactions
   */
  cleanup() {
    // Remove event listeners would go here in a full implementation
    console.log('[VoidInteractions] Interactions cleaned up');
  }
}