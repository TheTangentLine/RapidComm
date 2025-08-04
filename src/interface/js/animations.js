/**
 * Animation and visual effects module for RapidComm upload interface
 */

import { CONFIG } from './config.js';

/**
 * Create floating particles animation
 */
export function createParticles() {
  const particles = document.getElementById("particles");
  if (!particles) return;

  // Clear existing particles
  particles.innerHTML = '';

  for (let i = 0; i < CONFIG.PARTICLE_COUNT; i++) {
    const particle = document.createElement("div");
    particle.className = "particle";
    particle.style.left = Math.random() * 100 + "%";
    particle.style.animationDelay = Math.random() * 15 + "s";
    particle.style.animationDuration = Math.random() * 10 + 10 + "s";
    particles.appendChild(particle);
  }
}

/**
 * Animate progress bar with smooth transitions
 * @param {HTMLElement} progressBar - Progress bar element
 * @param {number} percentage - Target percentage (0-100)
 */
export function animateProgressBar(progressBar, percentage) {
  if (!progressBar) return;
  
  const currentWidth = parseFloat(progressBar.style.width) || 0;
  const targetWidth = Math.min(Math.max(percentage, 0), 100);
  
  // Use CSS transition for smooth animation
  progressBar.style.transition = 'width 0.3s ease-out';
  progressBar.style.width = targetWidth.toFixed(2) + "%";
}

/**
 * Create a glowing pulse effect on an element
 * @param {HTMLElement} element - Element to animate
 * @param {string} color - Glow color
 * @param {number} duration - Animation duration in ms
 */
export function createGlowPulse(element, color = '#667eea', duration = 1000) {
  if (!element) return;

  element.style.animation = `none`;
  element.offsetHeight; // Trigger reflow
  element.style.animation = `glow-pulse ${duration}ms ease-in-out`;
  
  // Clean up after animation
  setTimeout(() => {
    element.style.animation = '';
  }, duration);
}

/**
 * Animate element entrance with fade and slide
 * @param {HTMLElement} element - Element to animate
 * @param {string} direction - Direction to slide from ('up', 'down', 'left', 'right')
 * @param {number} duration - Animation duration in ms
 */
export function animateEntrance(element, direction = 'up', duration = 600) {
  if (!element) return;

  const transforms = {
    up: 'translateY(30px)',
    down: 'translateY(-30px)',
    left: 'translateX(30px)',
    right: 'translateX(-30px)'
  };

  // Set initial state
  element.style.opacity = '0';
  element.style.transform = transforms[direction] || transforms.up;
  element.style.transition = `opacity ${duration}ms ease-out, transform ${duration}ms ease-out`;

  // Trigger animation
  requestAnimationFrame(() => {
    element.style.opacity = '1';
    element.style.transform = 'translate(0, 0)';
  });

  // Clean up after animation
  setTimeout(() => {
    element.style.transition = '';
  }, duration);
}

/**
 * Animate element exit with fade and slide
 * @param {HTMLElement} element - Element to animate
 * @param {string} direction - Direction to slide to ('up', 'down', 'left', 'right')
 * @param {number} duration - Animation duration in ms
 * @returns {Promise} Promise that resolves when animation completes
 */
export function animateExit(element, direction = 'up', duration = 400) {
  return new Promise((resolve) => {
    if (!element) {
      resolve();
      return;
    }

    const transforms = {
      up: 'translateY(-30px)',
      down: 'translateY(30px)',
      left: 'translateX(-30px)',
      right: 'translateX(30px)'
    };

    element.style.transition = `opacity ${duration}ms ease-out, transform ${duration}ms ease-out`;
    element.style.opacity = '0';
    element.style.transform = transforms[direction] || transforms.up;

    setTimeout(() => {
      element.style.transition = '';
      resolve();
    }, duration);
  });
}

/**
 * Create a ripple effect on click
 * @param {HTMLElement} element - Element to create ripple on
 * @param {Event} event - Click event
 */
export function createRippleEffect(element, event) {
  if (!element) return;

  const rect = element.getBoundingClientRect();
  const size = Math.max(rect.width, rect.height);
  const x = event.clientX - rect.left - size / 2;
  const y = event.clientY - rect.top - size / 2;

  const ripple = document.createElement('div');
  ripple.style.cssText = `
    position: absolute;
    width: ${size}px;
    height: ${size}px;
    left: ${x}px;
    top: ${y}px;
    background: rgba(255, 255, 255, 0.3);
    border-radius: 50%;
    transform: scale(0);
    animation: ripple 600ms ease-out;
    pointer-events: none;
    z-index: 1000;
  `;

  element.style.position = 'relative';
  element.style.overflow = 'hidden';
  element.appendChild(ripple);

  // Add ripple animation keyframes if not already added
  if (!document.querySelector('#ripple-styles')) {
    const style = document.createElement('style');
    style.id = 'ripple-styles';
    style.textContent = `
      @keyframes ripple {
        to {
          transform: scale(2);
          opacity: 0;
        }
      }
    `;
    document.head.appendChild(style);
  }

  // Remove ripple after animation
  setTimeout(() => {
    if (ripple.parentNode) {
      ripple.parentNode.removeChild(ripple);
    }
  }, 600);
}

/**
 * Shake animation for error states
 * @param {HTMLElement} element - Element to shake
 * @param {number} intensity - Shake intensity (1-10)
 * @param {number} duration - Animation duration in ms
 */
export function shakeElement(element, intensity = 5, duration = 500) {
  if (!element) return;

  const keyframes = [
    { transform: 'translateX(0)' },
    { transform: `translateX(-${intensity}px)` },
    { transform: `translateX(${intensity}px)` },
    { transform: `translateX(-${intensity * 0.8}px)` },
    { transform: `translateX(${intensity * 0.8}px)` },
    { transform: `translateX(-${intensity * 0.5}px)` },
    { transform: `translateX(${intensity * 0.5}px)` },
    { transform: 'translateX(0)' }
  ];

  const options = {
    duration: duration,
    easing: 'ease-in-out'
  };

  element.animate(keyframes, options);
}

/**
 * Bounce animation for success states
 * @param {HTMLElement} element - Element to bounce
 * @param {number} height - Bounce height in pixels
 * @param {number} duration - Animation duration in ms
 */
export function bounceElement(element, height = 10, duration = 600) {
  if (!element) return;

  const keyframes = [
    { transform: 'translateY(0)' },
    { transform: `translateY(-${height}px)` },
    { transform: 'translateY(0)' },
    { transform: `translateY(-${height * 0.6}px)` },
    { transform: 'translateY(0)' },
    { transform: `translateY(-${height * 0.3}px)` },
    { transform: 'translateY(0)' }
  ];

  const options = {
    duration: duration,
    easing: 'ease-in-out'
  };

  element.animate(keyframes, options);
}

/**
 * Initialize all animations
 */
export function initializeAnimations() {
  createParticles();
  
  // Add CSS animations if not already present
  if (!document.querySelector('#animation-styles')) {
    const style = document.createElement('style');
    style.id = 'animation-styles';
    style.textContent = `
      @keyframes glow-pulse {
        0%, 100% { filter: drop-shadow(0 0 5px currentColor); }
        50% { filter: drop-shadow(0 0 20px currentColor); }
      }
    `;
    document.head.appendChild(style);
  }
}